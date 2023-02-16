//-----------------------------------------------------------------------------
//! \file       flash.c
//! \author     R. Weimar. RMB Holland bv
//! \brief      Contains routines for the flash
//! \details
//! \Attention
//! \Created on: Nov 18, 2022

#include "flash.h"
#include "main.h"
#include "work.h"
#include "eeprom.h"
#include "serial.h"
#include "led.h"
#include "iwdg.h"
#include <string.h>


//! \Image data buffer
uint8_t ImageData[FLA_BLOCKSIZE];
uint8_t ImageDataVerify[FLA_BLOCKSIZE];
//! \Flash data container for all partitions
stcFLA FLA[4];

//-----------------------------------------------------------------------------
//! \brief      Restores a partition
//! \details    Copies the RollBack partition over the Application partition
//! \params		None
void FLA_RestorePartition(void)
{
	uint8_t DataLength;
	//Check if roll back partition is valid, set the upgrade flag, reboot the system
	if (FLA[pRollBack].Status == fVALID)
	{
		FLA_SetStatus(pApplication,fROLLBACK);
		DataLength = SER_FillBuffer(10, Message.MType);
		SER_SendReturnMessage (DataLength + 10,MStatSuccess);
		HAL_NVIC_SystemReset();
	}
	else
	{
		SER_SendReturnMessage (10,MStatImageNotValid);
	}
}
//-----------------------------------------------------------------------------
//! \brief      Updates the application
//! \details    Copies the update partition over the Application partition
//! \params		None
void FLA_HandleUpgradeApplication(void)
{
	uint8_t DataLength;
	//Check if partition is valid, set the upgrade flag, reboot the system
	if (FLA[pUpdate].Status == fVALID)
	{
		FLA_SetStatus(pApplication,fUPDATING);
		DataLength = SER_FillBuffer(10, Message.MType);
		SER_SendReturnMessage (DataLength + 10,MStatSuccess);
		HAL_NVIC_SystemReset();
	}
	else
	{
		SER_SendReturnMessage (10,MStatImageNotValid);
	}
}
//-----------------------------------------------------------------------------
//! \brief      Stores the received update application
//! \details    Saves the received update blocks in the update partition
//! \params		None
void FLA_HandleSendPartitionBlock(void)
{
	uint32_t CurrentAddress;
	static uint16_t CurrentBlockOld;
	uint8_t DataLength;
	uint16_t NrOfBlocks;
	uint16_t CurrentBlock;
	HAL_IWDG_Refresh(&hiwdg);
	//Check whether the bin file size is not too big.
	if (SER_GetValue (12,4)> FLA_LENGTH / FLA_BLOCKSIZE)
	{
		SER_SendReturnMessage (10,MStatImageTooBig);
		return;
	}
	//Check whether the bin file is valid. If not, send error message
	if (SER_GetValue (16,4) == 0)
	{
		if ((SER_GetValue (20 ,2) != 0x00) || (SER_GetValue (22 ,2) != 0x80) || (SER_GetValue (24 ,2) != 0x00) || (SER_GetValue (26 ,2) != 0x20))
		{
			SER_SendReturnMessage (10,MStatImageCorrupt);
			return;
		}
	}
	//Get the number of blocks sent and store in EEPROM
	FLA[pUpdate].NrOfBlocks = SER_GetValue (12,4);
	EEP_WriteEEPROM(FLA_UPGRADENROFBLOCKS, FLA[pUpdate].NrOfBlocks);
	NrOfBlocks = FLA[pUpdate].NrOfBlocks;
	//Get the last block nr sent and store in EEPROM
	CurrentBlockOld = FLA[pUpdate].LastBlock;
	FLA[pUpdate].LastBlock = SER_GetValue (16,4);
	EEP_WriteEEPROM(FLA_UPGRADELASTBLOCK, FLA[pUpdate].LastBlock);
	CurrentBlock = FLA[pUpdate].LastBlock;
	if (CurrentBlock == 0) //First data received
	{
		//Get the version sent
		FLA_SetVersion(pUpdate, SER_GetValue(8,4));
		FLA_SetStatus(pUpdate, fUPDATING);
		//Clear the ImageData of this block
		memset(ImageData, 0, FLA_BLOCKSIZE); //Clear the array to zero before filling the block
		CurrentBlockOld = 0;
#ifdef BOOTLOADER
		LED_Set(STANDBYLED, BLINKING, 0, 0, 255, 50, 1000, 1000, 5, 0, 0); //Blinking BLUE
#endif
	}
	if ((CurrentBlock < NrOfBlocks) && (CurrentBlock - CurrentBlockOld <= 1) && (CurrentBlockOld <= CurrentBlock)) //Valid block numbers
	{
		CurrentAddress = FLA_UPGRADEADDRESS + (CurrentBlock * FLA_BLOCKSIZE);
		CurrentBlockOld = CurrentBlock;
		//Check if the Version number sent is the same as the number stored
		if (FLA[pUpdate].Version == SER_GetValue(8,4))
		{
			if (CurrentBlock % FLA_BLOCKSPERPAGE == 0)//new page
			{
				//Erase the new page in order to be able to write to it
				if (FLA_ErasePage (CurrentAddress) != HAL_OK)
				{
					SER_SendReturnMessage (10,MStatErasePageError);
					return;
				}

				//Clear the array to zero before filling the next block
				memset(ImageData, 0, FLA_BLOCKSIZE);
			}
			//Fill the buffer with the 64 bytes received
			for (uint8_t i = 0; i<FLA_BLOCKSIZE; i++)
			{
				ImageData[i] = SER_GetValue (20 + (i * 2),2);
			}
			//Write the received data to flash. This will cause the system to freeze until done. No UART possible
			FLA_WriteBlock (CurrentAddress);
			//Verify the written page
			if (FLA_VerifyBlock (CurrentAddress) == 0)
			{
				SER_SendReturnMessage (10,MStatVerifyError);
				return;
			}
			else
			{
				DataLength = SER_FillBuffer(10, Message.MType);
				SER_SendReturnMessage (DataLength + 10,MStatSuccess);
				if (CurrentBlock == NrOfBlocks - 1) //Last block
				{
					FLA_SetStatus(pUpdate, fVALID);
				}
				return;
			}
		}
		else //Version mismatch. Abort
		{
			SER_SendReturnMessage (10,MStatWrongVersion);
		}
	}
	else
	{
		SER_SendReturnMessage (10,MStatBlockNrError);
	}
}
//-----------------------------------------------------------------------------
//! \brief      Sets the status of the selected partition
//! \details    Sets the status and stores in EEprom
//! \param[in]	enuPartition newPartition
//! \param[in]	enuFLA newStatus
void FLA_SetStatus (enuPartition newPartition, enuFLA newStatus)
{
	FLA[(uint8_t) newPartition].Status = newStatus;
	if (newPartition == pApplication)
		EEP_WriteEEPROM(FLA_APPLICATIONSTATUS, FLA[pApplication].Status);
	else if (newPartition == pUpdate)
		EEP_WriteEEPROM(FLA_UPGRADESTATUS, FLA[pUpdate].Status);
	else if (newPartition == pRollBack)
		EEP_WriteEEPROM(FLA_ROLLBACKSTATUS, FLA[pRollBack].Status);
	for(uint32_t Counter = 0; Counter<10000; Counter++)
	{

	}
}
//-----------------------------------------------------------------------------
//! \brief      Sets the version of the selected partition
//! \details    Sets the version and stores in EEprom
//! \param[in]	enuPartition newPartition
//! \param[in]	uint32_t newVersion
void FLA_SetVersion (enuPartition newPartition, uint32_t newVersion)
{
	FLA[(uint8_t) newPartition].Version = newVersion;
	if (newPartition == pApplication)
		EEP_WriteEEPROM(FLA_APPLICATIONVERSION, FLA[pApplication].Version);
	else if (newPartition == pUpdate)
		EEP_WriteEEPROM(FLA_UPGRADEVERSION, FLA[pUpdate].Version);
	else if (newPartition == pRollBack)
		EEP_WriteEEPROM(FLA_ROLLBACKVERSION, FLA[pRollBack].Version);
	else if (newPartition == pBootloader)
		EEP_WriteEEPROM(FLA_BOOTLOADERVERSION, FLA[pBootloader].Version);
	for(uint32_t Counter = 0; Counter<10000; Counter++)
	{

	}
}
//-----------------------------------------------------------------------------
//! \brief      Erases a flash page
//! \details    Erases a flash page
//! \param[in] 	uint8_t NewAdress (FLA_FACTORYADDRESS or FLA_UPGRADEADDRESS)
HAL_StatusTypeDef FLA_ErasePage (uint32_t NewAddress)
{
	HAL_StatusTypeDef  flashstatus;
	FLASH_EraseInitTypeDef s_eraseinit;
	uint32_t page_error = 0;

	s_eraseinit.TypeErase   = FLASH_TYPEERASE_PAGES;
	s_eraseinit.PageAddress = NewAddress;
	s_eraseinit.NbPages     = 1;
	//Unlock flash
	HAL_FLASH_Unlock();
	flashstatus = HAL_FLASHEx_Erase(&s_eraseinit, &page_error);
	//Unlock flash
	HAL_FLASH_Lock();
	return flashstatus;
}
//-----------------------------------------------------------------------------
//! \brief      Writes a certain amount of data to a flash page
//! \details    Writes a certain amount of data to a flash page with the contents of Imagedata
//! \param[in] 	uint32_t NewAdress StartAddress for the first byte
HAL_StatusTypeDef FLA_WriteBlock (uint32_t NewAddress)
{
	HAL_StatusTypeDef  flashstatus;
	uint8_t Counter;
	for (uint16_t i=0; i<FLA_BLOCKSIZE; i+=4)
	{
		uint32_t Value = ImageData[i] + (ImageData[i+1]<<8) + (ImageData[i+2]<<16) + (ImageData[i+3]<<24);
		HAL_FLASH_Unlock();
		flashstatus = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, NewAddress + i, Value);
		Counter = 0;
		while ((flashstatus != HAL_OK) && (Counter < 100))
		{
			Counter ++;
			flashstatus = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, NewAddress + i, Value);
		}
		if (Counter == 100)
		{
			HAL_FLASH_Lock();
			return flashstatus; //Tried 100 times to write. Return error and break off
		}
	}
	HAL_FLASH_Lock();
	return flashstatus;
}
//-----------------------------------------------------------------------------
//! \brief      Verifies a flash page and compares to the data buffer
//! \details    Verifies a flash page and compares to the data buffer
//! \param[in] 	uint8_t NewAdress (FLA_FACTORYADDRESS or FLA_UPGRADEADDRESS)
//! \param[out]	uint8_t ReturnValue (0 = Failed, 1 = success)
uint8_t FLA_VerifyBlock (uint32_t NewAddress)
{
	uint32_t FlashContent;
	uint8_t ReturnValue = 1;

	for (uint16_t i = 0; i<FLA_BLOCKSIZE; i+=4)
	{
		FlashContent = (*(__IO uint32_t*) (NewAddress + i));
		ImageDataVerify[i] = (uint8_t) FlashContent;
		ImageDataVerify[i+1] = (uint8_t) (FlashContent >> 8);
		ImageDataVerify[i+2] = (uint8_t) (FlashContent >> 16);
		ImageDataVerify[i+3] = (uint8_t) (FlashContent >> 24);
		for (uint8_t j=0; j<4; j++)
		{
			if (ImageData[i+j] != ImageDataVerify[i+j])
			{
				ReturnValue = 0;
				return ReturnValue;
			}
		}
	}
	return ReturnValue;
}
//-----------------------------------------------------------------------------
//! \brief      Reads a flash page in the data buffer
//! \details    Reads a flash page into Imagedata
//! \param[in] 	uint8_t NewAdress (FLA_FACTORYADDRESS or FLA_UPGRADEADDRESS)
void FLA_ReadBlock (uint32_t NewAddress)
{
	uint32_t FlashContent;
	for (uint16_t i = 0; i<FLA_BLOCKSIZE; i+=4)
	{
		FlashContent = (*(__IO uint32_t*) (NewAddress + i));
		ImageData[i] = (uint8_t) FlashContent;
		ImageData[i+1] = (uint8_t) (FlashContent >> 8);
		ImageData[i+2] = (uint8_t) (FlashContent >> 16);
		ImageData[i+3] = (uint8_t) (FlashContent >> 24);
	}
}
//-----------------------------------------------------------------------------
//! \brief      Initalizes the flash module
//! \details    Initalizes the flash module
//! \params		None
void FLA_Init (void)
{
	//Read flash information from EEPROM
#ifdef BOOTLOADER
	FLA_SetVersion(pBootloader,WRK_GetSoftwareVersion());
	FLA[pApplication].Version = EEP_ReadEEPROM(FLA_APPLICATIONVERSION);
	FLA[pApplication].Status = EEP_ReadEEPROM(FLA_APPLICATIONSTATUS);
#else
	FLA[pBootloader].Version = EEP_ReadEEPROM(FLA_BOOTLOADERVERSION);
	FLA[pApplication].Version = WRK_GetSoftwareVersion();
	FLA[pApplication].Status = fVALID;
	//Write current application version
	EEP_WriteEEPROM(FLA_APPLICATIONVERSION, FLA[pApplication].Version);
	EEP_WriteEEPROM(FLA_APPLICATIONSTATUS, FLA[pApplication].Status);
#endif
	FLA[pRollBack].Version = EEP_ReadEEPROM(FLA_ROLLBACKVERSION);
	FLA[pRollBack].Status = EEP_ReadEEPROM(FLA_ROLLBACKSTATUS);
	FLA[pUpdate].Version = EEP_ReadEEPROM(FLA_UPGRADEVERSION);
	FLA[pUpdate].LastBlock = EEP_ReadEEPROM(FLA_UPGRADELASTBLOCK);
	FLA[pUpdate].NrOfBlocks = EEP_ReadEEPROM(FLA_UPGRADENROFBLOCKS);
	FLA[pUpdate].Status = EEP_ReadEEPROM(FLA_UPGRADESTATUS);
	FLA[pRollBack].NrOfBlocks = FLA_LENGTH / FLA_BLOCKSIZE;
}
