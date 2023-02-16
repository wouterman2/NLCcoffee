//-----------------------------------------------------------------------------
//! \file       flash.h
//! \author     R. Weimar
//! \brief      Contains routines for the flash module
//! \details
//! \Attention
//-----------------------------------------------------------------------------
#ifndef INC_FLASH_H_
#define INC_FLASH_H_

#include <stdint.h>
#include "main.h"

#define FLA_MAINADDRESS		0x08005800
#define FLA_UPGRADEADDRESS	0x08019000
#define FLA_ROLLBACKADDRESS	0x0802C800
#define FLA_LENGTH			0x13800 //78 kB Length of all partitions except bootloader
#define FLA_BLOCKSIZE		64
#define FLA_BLOCKSPERPAGE	4
#define FLA_PAGESIZE		256


typedef enum
{
	fUNDEFINED = 0,
	fVALID = 1,
	fUPDATING = 2,
	fUPDATED = 3,
	fROLLBACK = 4,
	fCORRUPT = 5,
}enuFLA;
typedef enum
{
	pBootloader = 0,
	pApplication = 1,
	pUpdate = 2,
	pRollBack = 3
}enuPartition;
typedef struct
{
	uint32_t Version;
	enuFLA Status;
	uint16_t LastBlock;
	uint16_t NrOfBlocks;
}stcFLA;

extern uint8_t ImageData[FLA_BLOCKSIZE];
extern uint8_t ImageDataVerify[FLA_BLOCKSIZE];;
extern stcFLA FLA[4];
extern HAL_StatusTypeDef FLA_ErasePage (uint32_t NewAddress);
extern void FLA_HandleSendPartitionBlock(void);
extern void FLA_HandleUpgradeApplication(void);
extern void FLA_RestorePartition(void);
extern void FLA_Init (void);
extern void FLA_ReadBlock (uint32_t NewAddress);
extern void FLA_SetStatus (enuPartition newPartition, enuFLA newStatus);
extern void FLA_SetVersion (enuPartition newPartition, uint32_t newVersion);
extern uint8_t FLA_VerifyBlock (uint32_t NewAddress);
extern HAL_StatusTypeDef FLA_WriteBlock (uint32_t NewAddress);

#endif /* INC_FLASH_H_ */
