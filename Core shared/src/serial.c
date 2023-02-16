//-----------------------------------------------------------------------------
//! \file       serial.c
//! \author     R. Weimar. RMB Holland bv
//! \brief      Contains routines for the serial module
//! \details
//! \Attention
//! \Created on: Apr 27, 2022
//!  The software is equipped to handle UART:
//!  - UART2 (115200 baud)
//!
//!  The software is a start of the UART communication between coffee machine and Linux
//!  It will simply return a Success return message unless the CRC is wrong. Then it will send a CRC Error message
//!  The protocol for the coffee machine is written below:
//!
//!  The protocol for the coffee machine is written in 80482201_UART protocol V1.x.xlsm
//!
//-----------------------------------------------------------------------------

#include "serial.h"
#include "usart.h"
#include "crc.h"
#include "utils.h"
#include "work.h"
#include "led.h"
#include "filter.h"
#include "defines.h"
#ifndef BOOTLOADER
#include "recipe.h"
#include "analog.h"
#endif
#include "flash.h"
#ifdef COFFEEMAKER
#include "steamwand.h"
#include "watertank.h"
#include "driptray.h"
#include "ritualbutton.h"
#include "AM2302.h"
#include "pumps.h"
#include "airpump.h"
#include "valves.h"
#include "heaters.h"
#include "steamwand.h"
#endif
#ifdef GRINDER
#include "filterslide.h"
#include "hopper.h"
#include "grinder.h"
#include "scale.h"
#include "consumable.h"
#include "adjust.h"
#endif
#include <stdint.h>
#include <string.h>

//! \CRC value calculated
uint8_t CalculatedCRC;
//! \UART message structure
stcUARTMessage Message;
//! \UART handle
uint8_t UartReturn;
//! \UART busy flag
uint8_t SER_Busy;

//-----------------------------------------------------------------------------
//! \brief      Initiates the serial unit
//! \details    Sets up Message buffer. Starts UART
//! \param      None
void SER_Init (void)
{
	Message.RxBuffer ='\000';
	HAL_UART_Receive_DMA(&huart3, &Message.RxBuffer, 1);
	//__HAL_UART_ENABLE_IT(&huart3,UART_IT_IDLE);
	HAL_UART_Receive_DMA(&huart2, &Message.RxBuffer, 1);
}

//-----------------------------------------------------------------------------
//! \brief      Sets the status of the return message of actions
//! \details    the work module confirms accepting or timing out.
//! \param[in]  uint8_t newStatus	Status of the return message
void SER_SetStatus (uint8_t newStatus)
{
	SER_SendReturnMessage (10, newStatus);
}

//! \brief      Sends a return message to the corresponding UART
//! \details    Calculates CRC, fills the buffer and sends the buffer
//! \param[in]  uint8_t newLength	Length of the message
//! \param[in]  uint8_t newStatus	Status of the return message
void SER_SendReturnMessage (uint8_t newLength, uint8_t newStatus)
{
	uint8_t newCRC;
	//Fill length in data buffer
	Message.RxData[2]=UTL_Dec2Asc(newLength / 16);
	Message.RxData[3]=UTL_Dec2Asc(newLength % 16);
	//Fill status in data buffer
	if (newLength > 8)
	{
		Message.RxData[8]=UTL_Dec2Asc(newStatus / 16);
		Message.RxData[9]=UTL_Dec2Asc(newStatus % 16);
	}
	//Calculate CRC and store it in data buffer
	newCRC = CalculateCRC8(Message.RxData, newLength);
	Message.RxData[6]= UTL_Dec2Asc(newCRC / 16);
	Message.RxData[7]= UTL_Dec2Asc(newCRC % 16);
	//Send data

	if (UartReturn==2)
		HAL_UART_Transmit(&huart2, Message.RxData,newLength,100);
	else if (UartReturn==3)
		HAL_UART_Transmit(&huart3, Message.RxData,newLength,100);
	//Empty data buffer
	Message.Length = 0;
	Message.MType = 0;
	Message.Counter = 0;
	Message.CRCValue = 0;
	Message.RxBuffer = 0;
	memset(Message.RxData, 0, sizeof(Message.RxData));
	SER_Busy = 0;
}

//-----------------------------------------------------------------------------
//! \brief      Converts the byte from the recipe to an integer
//! \details    Converts the hex bytes to an integer, returns the integer
//! \param[in]  uint8_t newUart			Uart number
//! \param[in]  uint8_t newStartByte	Start byte of the hex string
//! \param[in]  uint8_t newLength		Length of the hex string [2 or 4 bytes]
//! \param[out]	uint16_t ReturnValue
uint16_t SER_GetValue (uint8_t newStartByte, uint8_t newLength)
{
	uint16_t returnValue = 0;
	if (newLength == 1)
		returnValue = UTL_Asc2Dec(Message.RxData[newStartByte]);
	else if (newLength == 2)
		returnValue = UTL_Asc2Dec(Message.RxData[newStartByte]) * 16 + UTL_Asc2Dec(Message.RxData[newStartByte + 1]);
	else if (newLength == 4)//4 bytes
		returnValue = UTL_Asc2Dec(Message.RxData[newStartByte]) * 4096 + UTL_Asc2Dec(Message.RxData[newStartByte + 1]) * 256 + UTL_Asc2Dec(Message.RxData[newStartByte + 2]) * 16 + UTL_Asc2Dec(Message.RxData[newStartByte + 3]);
	return returnValue;
}

//-----------------------------------------------------------------------------
//! \brief      Fills the buffer according to the byte count
//! \details    Fills the buffer with converted characters
//! \param[in]  uint8_t newPosition		Position in the array
//! \Param[in]	uint8_t newByteCount	1 byte = 2 hex positions
//! \Param[in]	uint64_t newValue
void SER_FillData (uint8_t newPosition, uint8_t newByteCount, uint64_t newValue)
{
	uint64_t CompareValue;
	if (newByteCount == 1) CompareValue = 0xF0;
	if (newByteCount == 2) CompareValue = 0xF000;
	if (newByteCount == 4) CompareValue = 0xF0000000;
	if (newByteCount == 8) CompareValue = 0xF000000000000000;

	for (uint8_t i = 0; i < (newByteCount * 2); i++)
	{
		Message.RxData[newPosition + i] = UTL_Dec2Asc(((newValue & (CompareValue >> i*4)) >> (4 * ((newByteCount * 2) - 1 - i))));
	}
}

uint8_t SER_FillBuffer (uint8_t newPosition, uint16_t MType)
{
	volatile uint8_t Counter = 0;
	uint8_t ReturnValue = 0;
	if (MType == MTypeSendPartitionBlock)
	{
		SER_FillData(newPosition, 2, (uint32_t) FLA[pUpdate].Version);
		SER_FillData(newPosition+(Counter+=4), 2, (uint32_t) FLA[pUpdate].LastBlock);
		SER_FillData(newPosition+(Counter+=4), 2, (uint32_t) FLA[pUpdate].NrOfBlocks);
		SER_FillData(newPosition+(Counter+=4), 2, (uint32_t) FLA[pUpdate].Status);
	}
	if ((MType == MTypeGetPartitionInfo) || (MType == MTypeUpgradeApplication) || (MType == MTypeSetPartitionStatus))
	{
		SER_FillData(newPosition, 2, (uint32_t) FLA[pApplication].Version);
		SER_FillData(newPosition+(Counter+=4), 2, (uint32_t) FLA[pApplication].Status);
		SER_FillData(newPosition+(Counter+=4), 2, (uint32_t) FLA[pRollBack].Version);
		SER_FillData(newPosition+(Counter+=4), 2, (uint32_t) FLA[pRollBack].Status);
		SER_FillData(newPosition+(Counter+=4), 2, (uint32_t) FLA[pUpdate].Version);
		SER_FillData(newPosition+(Counter+=4), 2, (uint32_t) FLA[pUpdate].LastBlock);
		SER_FillData(newPosition+(Counter+=4), 2, (uint32_t) FLA[pUpdate].NrOfBlocks);
		SER_FillData(newPosition+(Counter+=4), 2, (uint32_t) FLA[pUpdate].Status);
	}
#ifdef BOOTLOADER
	if ((MType == MTypeGetBackupProgress)||(MType == MTypeGetRestoreProgress))
	{
		Message.RxData[0] = '/';
		Message.RxData[1] = '1';
		SER_FillData(4, 1, (uint32_t) MType);
		SER_FillData(newPosition, 2, (uint32_t) Progress);
	}
	if (MType == MTypeGetUpdateProgress)
	{
		Message.RxData[0] = '/';
		Message.RxData[1] = '1';
		SER_FillData(4, 1, (uint32_t) MType);
		SER_FillData(newPosition, 2, (uint32_t) Progress);
	}
	if (MType == MTypeGetBootloaderStatus)
	{
		Message.RxData[0] = '/';
		Message.RxData[1] = '1';
		SER_FillData(4, 1, (uint32_t) MType);
		SER_FillData(newPosition, 2, (uint32_t) BootloaderStatus);
		SER_FillData(newPosition+(Counter+=4), 2, (uint32_t) FLA[pApplication].Status);
	}
#endif
	if (MType == MTypeGetDummyMode)
	{
#ifdef COFFEEMAKER
		SER_FillData(newPosition, 2, (uint32_t) DUMMYMODE);
		SER_FillData(newPosition+(Counter+=4), 2, (uint32_t) DUMMYMODEFILTER);
		SER_FillData(newPosition+(Counter+=4), 2, (uint32_t) DUMMYMODEH3);
#endif
#ifdef GRINDER
		SER_FillData(newPosition, 2, (uint32_t) DUMMYMODEGRINDER);
		SER_FillData(newPosition+(Counter+=4), 2, (uint32_t) DUMMYMODEFILTER);
		SER_FillData(newPosition+(Counter+=4), 2, (uint32_t) DUMMYMODERFID);
		SER_FillData(newPosition+(Counter+=4), 2, (uint32_t) DUMMYMODESCALE);
#endif
	}
	if (MType == MTypeGetMachineParameters)
	{
#ifdef COFFEEMAKER
		SER_FillData(newPosition, 2, (uint32_t) PUM_GetParameter(PUM_ULPERPULSEP1));
		SER_FillData(newPosition+(Counter+=4), 2, (uint32_t) PUM_GetParameter(PUM_ULPERPULSEP2));
#endif
#ifdef GRINDER
		SER_FillData(newPosition, 2, (uint32_t) ADJ_GetParameter(ADJ_P));
		SER_FillData(newPosition+(Counter+=4), 2, (uint32_t) ADJ_GetParameter(ADJ_I));
		SER_FillData(newPosition+(Counter+=4), 2, (uint32_t) ADJ_GetParameter(ADJ_D));
#endif
	}
	if ((MType == MTypeGetProcessData)||(MType == MTypeGetAllData))
	{
#ifdef COFFEEMAKER
		SER_FillData(newPosition, 2, (uint32_t) REC_GetNrOfBlocks(RECIPE1));
		SER_FillData(newPosition+(Counter+=4), 2, (uint32_t) REC_GetNrOfBlocks(RECIPE2));
		SER_FillData(newPosition+(Counter+=4), 2, 0);//No more recipe 3. Was Water
		SER_FillData(newPosition+(Counter+=4), 2, (uint32_t) WAT_GetVolume());
		SER_FillData(newPosition+(Counter+=4), 2, (uint32_t) FIL_GetStatus());
		SER_FillData(newPosition+(Counter+=4), 2, (uint32_t) WAN_GetStatus());
		SER_FillData(newPosition+(Counter+=4), 2, (uint32_t) DRP_GetStatus());
		SER_FillData(newPosition+(Counter+=4), 2, (uint32_t) WRK_GetMainStatus());
		SER_FillData(newPosition+(Counter+=4), 2, (uint32_t) WRK_GetSubStatus());
		SER_FillData(newPosition+(Counter+=4), 2, (uint32_t) AM_GetHumidity());
		SER_FillData(newPosition+(Counter+=4), 2, (uint32_t) AM_GetTemperature());
#endif
#ifdef GRINDER
		SER_FillData(newPosition, 2, (uint64_t) REC_GetRequiredWeight());
		SER_FillData(newPosition+(Counter+=4), 2, (uint64_t) REC_GetGrindSize());
		SER_FillData(newPosition+(Counter+=4), 2, (uint64_t) WRK_GetMainStatus());
		SER_FillData(newPosition+(Counter+=4), 2, (uint64_t) WRK_GetSubStatus());
		SER_FillData(newPosition+(Counter+=4), 2, (uint64_t) FIL_GetStatus());
		SER_FillData(newPosition+(Counter+=4), 2, (uint64_t) SLD_GetStatus());
		SER_FillData(newPosition+(Counter+=4), 2, (uint64_t) WRK_GetPrepareStatus());
		SER_FillData(newPosition+(Counter+=4), 2, (uint64_t) HOP_GetStatus());
		SER_FillData(newPosition+(Counter+=4), 2, (uint64_t) GND_GetStatus());
		SER_FillData(newPosition+(Counter+=4), 2, (uint64_t) SCL_GetStatus());
		SER_FillData(newPosition+(Counter+=4), 2, 0); //Sensors not clear why needed
#endif
	}
	if ((MType == MTypeGetPhysicalData)||(MType == MTypeGetAllData))
	{
#ifdef COFFEEMAKER
		if (Counter == 0)
			SER_FillData(newPosition, 2, (uint32_t) REC_GetCurrentBlock(RECIPE1));
		else
			SER_FillData(newPosition+(Counter+=4), 2, (uint32_t) REC_GetCurrentBlock(RECIPE1));
		SER_FillData(newPosition+(Counter+=4), 2, (uint32_t) REC_GetCurrentBlock(RECIPE2));
		SER_FillData(newPosition+(Counter+=4), 2, (uint32_t) REC_GetCurrentBlock(RECIPE3));//Preheat recipe
		SER_FillData(newPosition+(Counter+=4), 2, (uint32_t) HEA_GetHeaterTemperature(H1)); //FTH1 temperature
		SER_FillData(newPosition+(Counter+=4), 2, (uint32_t) HEA_GetHeaterTemperature(H2)); //FTH2 temperature
		SER_FillData(newPosition+(Counter+=4), 2, (uint32_t) HEA_GetHeaterTemperature(H3)); //BrewerHead temperature
		SER_FillData(newPosition+(Counter+=4), 2, (uint32_t) WAN_GetTemperature()); //Milk temperature
		SER_FillData(newPosition+(Counter+=4), 2, (uint32_t) PUM_GetFlow(0)); //Flow pump 1
		SER_FillData(newPosition+(Counter+=4), 2, (uint32_t) PUM_GetFlow(1)); //Flow pump 2
		SER_FillData(newPosition+(Counter+=4), 2, (uint32_t) AIR_GetPercentage()); //Flow pump 3 (In fact percentage. Has no flow meter for the air)
		SER_FillData(newPosition+(Counter+=4), 2, (uint32_t) PUM_GetPressure(0)); //PS1 pressure
		SER_FillData(newPosition+(Counter+=4), 2, (uint32_t) PUM_GetPressure(1)); //PS2 pressure
		SER_FillData(newPosition+(Counter+=4), 2, (uint32_t) PUM_GetVolume(0)); //Volume pump 1
		SER_FillData(newPosition+(Counter+=4), 2, (uint32_t) PUM_GetVolume(1)); //Volume pump 2
		SER_FillData(newPosition+(Counter+=4), 2, (uint32_t) VAL_GetStatus()); //Valves bit array
		SER_FillData(newPosition+(Counter+=4), 2, (uint32_t) BTN_GetStatus(BTN_COFFEE));
		SER_FillData(newPosition+(Counter+=4), 2, (uint32_t) BTN_GetDirection(BTN_COFFEE));
		SER_FillData(newPosition+(Counter+=4), 2, (uint32_t) BTN_GetStatus(BTN_MILK));
#endif
#ifdef GRINDER
		if (Counter == 0)
			SER_FillData(newPosition, 2, (uint64_t) ANA_GetSensorValue(ADC_BURRS));
		else
			SER_FillData(newPosition, (Counter+=4), (uint64_t) ANA_GetSensorValue(ADC_BURRS));
		SER_FillData(newPosition+(Counter+=4), 2, (uint64_t) ANA_GetSensorValue(ADC_MOTOR));
		SER_FillData(newPosition+(Counter+=4), 2, (uint64_t) SCL_GetWeight());
#endif
	}
#ifndef BOOTLOADER
	if ((MType == MTypeGetDeviceData)||(MType == MTypeGetAllData))
	{
		if (Counter == 0)
			SER_FillData(newPosition, 2, (uint32_t) SERIALNUMBER >> 16);
		else
			SER_FillData(newPosition+(Counter+=4), 2, (uint32_t) SERIALNUMBER >> 16);
		SER_FillData(newPosition+(Counter+=4), 2, (uint32_t) (SERIALNUMBER & 0xffff));
		SER_FillData(newPosition+(Counter+=4), 2, (uint32_t) WRK_GetHardwareVersion());
		SER_FillData(newPosition+(Counter+=4), 2, (uint32_t) WRK_GetSoftwareVersion());
		SER_FillData(newPosition+(Counter+=4), 2, (uint32_t) FLA[pBootloader].Version);
	}
#endif
#ifdef GRINDER
	else if (MType == MTypeGetDosingData)
	{
		SER_FillData(newPosition, 2, (uint64_t) HOP.TimeMin);
		SER_FillData(newPosition+(Counter+=4), 2, (uint64_t) HOP.TimeMax);
		SER_FillData(newPosition+(Counter+=4), 2, (uint64_t) HOP.DumpAngle);
		SER_FillData(newPosition+(Counter+=4), 2, (uint64_t) HOP.DoseAngle);
		SER_FillData(newPosition+(Counter+=4), 2, (uint64_t) HOP.DoseStepsMax);
		SER_FillData(newPosition+(Counter+=4), 2, (uint64_t) SCL.StableTime);
		SER_FillData(newPosition+(Counter+=4), 2, (uint64_t) HOP.ClosedPulseTime);
		SER_FillData(newPosition+(Counter+=4), 2, (uint64_t) HOP.OpenedPulseTime);
	}
	else if (MType == MTypeGetConsumableData)
	{

		for (uint8_t i=0; i < TAGDATACOUNT; i++)
		{
			SER_FillData(newPosition + Counter, 2, (uint64_t) CON.CardMemory[i]);
			if (TAGDATACOUNT - i > 1) Counter += 4;
		}
	}
#endif
	ReturnValue = (Counter+=4);
	return ReturnValue;
}

//-----------------------------------------------------------------------------
//! \brief      Handles the serial actions
//! \details    Process the message
//! \params     None
void SER_Handle (void)
{
	SER_ProcessMessage();
	if(HAL_UART_GetError(&huart2))
	{
		HAL_UART_DMAStop(&huart2);
		MX_USART2_UART_Init();
		HAL_UART_Receive_DMA(&huart2, &Message.RxBuffer, 1);
	}
	if(HAL_UART_GetError(&huart3))
	{
		HAL_UART_DMAStop(&huart3);
		MX_USART2_UART_Init();
		HAL_UART_Receive_DMA(&huart3, &Message.RxBuffer, 1);
	}
}

//-----------------------------------------------------------------------------
//! \brief      Processes the message
//! \details    Checks if the complete message is received and if so processes it
//! \param      None
void SER_ProcessMessage (void)
{
	uint8_t DataLength;

	if	((Message.Counter > 0) &&(Message.Counter == Message.Length))  //Message received completely
	{
		SER_Busy = 1;
		//Check CRC
#ifndef BOOTLOADER
		WRK_ResetAction(); //Cancel the current action in case this is pending as the return message is scrambled.
#endif
		CalculatedCRC = CalculateCRC8(Message.RxData, Message.Length);
		if (CalculatedCRC != Message.CRCValue) //Invalid CRC.
		{
			SER_SendReturnMessage (10,MStatCRCError);
		}
		else if (Message.UARTVersion != UARTVERSION)
		{
			SER_SendReturnMessage (10,MStatUARTVersionError);
		}
		//Commands only accepted by grinder and coffee machine
#ifndef BOOTLOADER
		else if ((Message.MType & 0xF0) == 0x00) //Message is a recipe
		{
			SER_HandleRecipe();
		}
		else if ((Message.MType & 0xF0) == 0x10) //Message is a recipe action
		{
				Message.Counter = 0;
				WRK_SetAction (Message.MType);
			//Work module initiates the return message by calling SER_SetStatus
			//The Linux system should wait for the return message before sending a new message
			//If another message is received before SER_SetStatus is called, the current action is canceled.
		}
		else if (Message.MType == MTypeGetProcessData)
		{
			DataLength = SER_FillBuffer(10, Message.MType);
			SER_SendReturnMessage (DataLength + 10,MStatSuccess);
		}
		else if (Message.MType == MTypeGetPhysicalData)
		{
			DataLength = SER_FillBuffer(10, Message.MType);
			SER_SendReturnMessage (DataLength + 10,MStatSuccess);
		}
		else if (Message.MType == MTypeGetDeviceData)
		{
			DataLength = SER_FillBuffer(10, Message.MType);
			SER_SendReturnMessage (DataLength + 10,MStatSuccess);
		}
		else if (Message.MType == MTypeGetAllData)
		{
			DataLength = SER_FillBuffer(10, Message.MType);
			SER_SendReturnMessage (DataLength + 10,MStatSuccess);
		}
		else if (Message.MType == MTypeGetDummyMode)
		{
			DataLength = SER_FillBuffer(10, Message.MType);
			SER_SendReturnMessage (DataLength + 10,MStatSuccess);
		}
		else if (Message.MType == MTypeGetMachineParameters)
		{
			DataLength = SER_FillBuffer(10, Message.MType);
			SER_SendReturnMessage (DataLength + 10,MStatSuccess);
		}
		else if (Message.MType == MTypeReboot)
		{
			SER_SendReturnMessage (10,MStatSuccess);
			HAL_NVIC_SystemReset();
		}
		else if ((Message.MType & 0xF0) == 0x50)
		{
			Message.Counter = 0;
			WRK_SetAction (Message.MType);
		}
#endif
		//Commands accepted by all
		else if (Message.MType == MTypeSendPartitionBlock) //Application upgrade is being sent
		{
			FLA_HandleSendPartitionBlock();
		}
		else if (Message.MType == MTypeUpgradeApplication)
		{
			FLA_HandleUpgradeApplication();
		}
		else if (Message.MType == MTypeRestorePartition)
		{
			FLA_RestorePartition();
		}
		else if (Message.MType == MTypeGetPartitionInfo) //Get image info in flash
		{
			DataLength = SER_FillBuffer(10, Message.MType);
			SER_SendReturnMessage (DataLength + 10,MStatSuccess);
#ifdef BOOTLOADER
		LED_Set(STANDBYLED, BLINKING, 255, 0, 0, 50, 100, 100, 5, 0, 0); //Blinking fast RED
#endif

		}
		else if (Message.MType == MTypeSetPartitionStatus) //Application upgrade is being sent
		{
			FLA_SetStatus(pUpdate,(enuFLA) SER_GetValue (10,2));
			DataLength = SER_FillBuffer(10, Message.MType);
			SER_SendReturnMessage (DataLength + 10,MStatSuccess);
		}
		else if (Message.MType == MTypeSetLED)
		{
			LED_Set(SER_GetValue (12,1), SER_GetValue (13,1), SER_GetValue (14,2), SER_GetValue (16,2), SER_GetValue (18,2), SER_GetValue (20,2), SER_GetValue (22,4), SER_GetValue (26,4), SER_GetValue (30,2), SER_GetValue (32,4), SER_GetValue (36,2));
			SER_SendReturnMessage (10,MStatSuccess);
		}
		//Commands only accepted by grinder
#ifdef GRINDER
		else if (Message.MType == MTypeGetDosingData)
		{
			DataLength = SER_FillBuffer(10, Message.MType);
			SER_SendReturnMessage (DataLength + 10,MStatSuccess);
		}
		else if (Message.MType == MTypeSetDosingData)
		{
			HOP_SetParameter(HOP_TIMEMIN, SER_GetValue (12,4));
			HOP_SetParameter(HOP_TIMEMAX, SER_GetValue (16,4));
			HOP_SetParameter(HOP_DUMPANGLE, SER_GetValue (20,4));
			HOP_SetParameter(HOP_DOSEANGLE, SER_GetValue (24,4));
			HOP_SetParameter(HOP_DOSESTEPSMAX, SER_GetValue (28,4));
			SCL_SetParameter(SCL_STABLETIME, SER_GetValue (32,4));
			HOP_SetParameter(HOP_CLOSEDPULSETIME, SER_GetValue (36,4));
			HOP_SetParameter(HOP_OPENEDPULSETIME, SER_GetValue (40,4));
			SER_SendReturnMessage (10,MStatSuccess);
		}
		else if (Message.MType == MTypeGetConsumableData)
		{
			DataLength = SER_FillBuffer(10, Message.MType);
			SER_SendReturnMessage (DataLength + 10,MStatSuccess);
		}
		else if (Message.MType == MTypeSetConsumableData)
		{
			for (uint8_t i = 0; i < 58; i++)
			{
				CON.CardMemory[i] = SER_GetValue (8 + (i * 2),2);
			}
			SER_SendReturnMessage (10,MStatSuccess);
		}
#endif
		else if (Message.MType == MTypeSetDummyMode)
		{
#ifdef COFFEEMAKER
			WRK_SetDummyMode(WRK_DUMMYMODE,SER_GetValue (12,2));
			WRK_SetDummyMode(WRK_DUMMYMODEFILTER,SER_GetValue (14,2));
			WRK_SetDummyMode(WRK_DUMMYMODEH3,SER_GetValue (16,2));
#endif
#ifdef GRINDER
			WRK_SetDummyMode(WRK_DUMMYMODEGRINDER,SER_GetValue (12,2));
			WRK_SetDummyMode(WRK_DUMMYMODEFILTER,SER_GetValue (14,2));
			WRK_SetDummyMode(WRK_DUMMYMODERFID,SER_GetValue (16,2));
			WRK_SetDummyMode(WRK_DUMMYMODESCALE,SER_GetValue (18,2));
#endif
			SER_SendReturnMessage (10,MStatSuccess);
			NVIC_SystemReset();
		}
		else if (Message.MType == MTypeSetMachineParameters)
		{
#ifdef COFFEEMAKER
			PUM_SetParameter(PUM_ULPERPULSEP1,SER_GetValue (12,4));
			PUM_SetParameter(PUM_ULPERPULSEP2,SER_GetValue (16,4));
#endif
#ifdef GRINDER
			ADJ_SetParameter(ADJ_P,SER_GetValue (12,4));
			ADJ_SetParameter(ADJ_I,SER_GetValue (16,4));
			ADJ_SetParameter(ADJ_D,SER_GetValue (20,4));
#endif
			SER_SendReturnMessage (10,MStatSuccess);

		}
		//Work module initiates the return message by calling SER_SetStatus
		//The Linux system should wait for the return message before sending a new message
		//If another message is received before SER_SetStatus is called, the current action is canceled.
		else //Message type is unknown
		{
			SER_SendReturnMessage (10,MStatUnknownType);
		}
	}
}

//-----------------------------------------------------------------------------
//! \brief      Handles the UART interrupt from DMA
//! \details    Resets the buffer counter if 0x1B is received
//! \details    Checks the length of the message and limits to MDataMax
//! \details    Fills the rest of the data buffer until the length of the message is received
//! \param      None
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (SER_Busy) return;
	uint8_t CurrentValue;
	if(huart->Instance == USART2)  //Received from debugging interface
	{
		UartReturn = 2;
	}
	else if(huart->Instance == USART3)  //Received from linux system
	{
		UartReturn = 3;
	}

	CurrentValue = UTL_Asc2Dec(Message.RxBuffer);
	Message.RxData[Message.Counter] = Message.RxBuffer;
	if (Message.RxBuffer == 0x2f) //Start byte. Reset Rx counter
	{
		Message.Counter = 1;
	}
	else if (Message.Counter == 1) //UART protocol
	{
		Message.UARTVersion = CurrentValue;
		Message.Counter = 2;
	}
	else if (Message.Counter == 2) //Length MSB received
	{
		Message.Length = CurrentValue * 16;
		Message.Counter = 3;
	}
	else if (Message.Counter == 3) //Length LSB received
	{
		Message.Length += CurrentValue;
		if (Message.Length > MLengthMax)
			Message.Length = MLengthMax; //Prevent buffer overflow
		Message.Counter = 4;
	}
	else if	(Message.Counter == 4) //Message type MSB received
	{
		Message.MType = CurrentValue * 16;
		Message.Counter = 5;
	}
	else if	(Message.Counter == 5) //Message type LSB received
	{
		Message.MType += CurrentValue;
		Message.Counter = 6;
	}
	else if	(Message.Counter == 6) //CRC MSB received
	{
		Message.CRCValue = CurrentValue * 16;
		Message.Counter = 7;
	}
	else if	(Message.Counter == 7) //CRC LSB received
	{
		Message.CRCValue += CurrentValue;
		Message.Counter = 8;
	}
	else if	(Message.Counter < Message.Length) //Any byte received
	{
		Message.Counter ++;
	}
}
#ifndef BOOTLOADER
void SER_HandleRecipe (void)
{
	uint16_t NrOfBlocks;
	uint16_t CurrentBlock;

	NrOfBlocks = SER_GetValue (8,2);
#ifdef COFFEEMAKER
	gRecipe[Message.MType-1].Blocks = NrOfBlocks;
#endif
#ifdef GRINDER
	gRecipe.Blocks = NrOfBlocks;
#endif
	CurrentBlock = SER_GetValue (10,2);

	if (CurrentBlock < NrOfBlocks)
	{
#ifdef GRINDER
		if (CurrentBlock == 0) memset(gRecipe.RecipeBlocks, 0, sizeof(gRecipe.RecipeBlocks)); //Clear the current coffee recipe
		gRecipe.RecipeBlocks[CurrentBlock].GrindWeight = SER_GetValue (12,2);
		gRecipe.RecipeBlocks[CurrentBlock].GrindSize = SER_GetValue (14,4);
		gRecipe.RecipeBlocks[CurrentBlock].FilterType = SER_GetValue (18,2);
		gRecipe.RecipeBlocks[CurrentBlock].ShutterTime = SER_GetValue (20,4);
		if (CurrentBlock == NrOfBlocks - 1)
			gRecipe.Valid = 1;
		else
			gRecipe.Valid = 0;

#endif
#ifdef COFFEEMAKER
		if (CurrentBlock == 0)
		{
			memset(gRecipe[Message.MType-1].RecipeBlocks, 0, sizeof(gRecipe[Message.MType-1].RecipeBlocks)); //Clear the current coffee recipe
			gRecipe[Message.MType-1].SetTotalVolume = 0;
		}
		gRecipe[Message.MType-1].RecipeBlocks[CurrentBlock].H1SetTemp = SER_GetValue (12,2);
		gRecipe[Message.MType-1].RecipeBlocks[CurrentBlock].H2SetTemp = SER_GetValue (14,2);
		gRecipe[Message.MType-1].RecipeBlocks[CurrentBlock].H3SetTemp = SER_GetValue (16,2);
		//Added direct update of temperature in case of recipe 3, misused as preheat recipe
		if (Message.MType - 1 == RECIPE3)
		{
			if (HEA[H3].Status == sACTIVE)
			{
				HEA[H3].SetTemperature = SER_GetValue (16,2) * 10;
			}
		}
		gRecipe[Message.MType-1].RecipeBlocks[CurrentBlock].MFSetTemp = SER_GetValue (18,2);
		gRecipe[Message.MType-1].RecipeBlocks[CurrentBlock].P1Flow = SER_GetValue (20,2);
		gRecipe[Message.MType-1].RecipeBlocks[CurrentBlock].P2Flow = SER_GetValue (22,2);
		gRecipe[Message.MType-1].RecipeBlocks[CurrentBlock].P3Flow = SER_GetValue (24,2);
		gRecipe[Message.MType-1].RecipeBlocks[CurrentBlock].P1Pressure = SER_GetValue (26,2);
		gRecipe[Message.MType-1].RecipeBlocks[CurrentBlock].P2Pressure = SER_GetValue (28,2);
		gRecipe[Message.MType-1].RecipeBlocks[CurrentBlock].P1Volume = SER_GetValue (30,4);
		gRecipe[Message.MType-1].SetTotalVolume += SER_GetValue (30,4);
		gRecipe[Message.MType-1].RecipeBlocks[CurrentBlock].P2Volume = SER_GetValue (34,4);
		gRecipe[Message.MType-1].SetTotalVolume += SER_GetValue (34,4);
		gRecipe[Message.MType-1].RecipeBlocks[CurrentBlock].Valves  = SER_GetValue (38,2);
		gRecipe[Message.MType-1].RecipeBlocks[CurrentBlock].Piston = SER_GetValue (40,2);
		gRecipe[Message.MType-1].RecipeBlocks[CurrentBlock].Time = SER_GetValue (42,4);
		gRecipe[Message.MType-1].RecipeBlocks[CurrentBlock].ControlType1 = SER_GetValue (46,2);
		gRecipe[Message.MType-1].RecipeBlocks[CurrentBlock].ControlType2 = SER_GetValue (48,2);
		gRecipe[Message.MType-1].RecipeBlocks[CurrentBlock].EndConditions1 = SER_GetValue (50,4);
		gRecipe[Message.MType-1].RecipeBlocks[CurrentBlock].EndConditions2 = SER_GetValue (54,4);
		gRecipe[Message.MType-1].RecipeBlocks[CurrentBlock].EndConditions3 = SER_GetValue (58,4);
		gRecipe[Message.MType-1].RecipeBlocks[CurrentBlock].EndConditions4 = SER_GetValue (62,4);
		if (CurrentBlock == NrOfBlocks - 1)
			gRecipe[Message.MType-1].Valid = 1;
		else
			gRecipe[Message.MType-1].Valid = 0;
#endif

		SER_SendReturnMessage (10,MStatSuccess);
	}
	else
	{
		SER_SendReturnMessage (10,MStatBlockNrError);
	}
}
#endif

//---------------- End of file -----------------------
