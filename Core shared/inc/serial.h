//-----------------------------------------------------------------------------
//! \file       serial.h
//! \author     R. Weimar
//! \brief      Contains routines for the serial module
//! \details
//! \Attention
//-----------------------------------------------------------------------------


#ifndef INC_SERIAL_H_
#define INC_SERIAL_H_
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <stdint.h>
#ifndef BOOTLOADER
#include "usart.h"
#endif
//-----------------------------------------------------------------------------
#define UARTVERSION			1  //UART protocol version
#define MLengthMax 			255
//Message types recipe setting
#define MTypeRecipe1		0x01
#define MTypeRecipe2		0x02
#define MTypeRecipe3		0x03
#define MTypeLastRecipe		0x03
#define MTypeMaxRecipe		0x0F
//Message types recipe actions
#define MTypeStartRecipe1		0x11
#define MTypeStartRecipe2		0x12
#define MTypeStartRecipe3		0x13
#define MTypeStopRecipe1		0x14
#define MTypeStopRecipe2		0x15
#define MTypeStopRecipe3		0x16
#define MTypeLastRecipeAction	0x16
#define MTypeMaxRecipeAction	0x1F
//Message types partition actions
#define MTypeSendPartitionBlock		0x21
#define MTypeUpgradeApplication		0x22
#define MTypeGetPartitionInfo   	0x23
#define MTypeGetUpdateProgress		0x24
#define MTypeGetBootloaderStatus 	0x25
#define MTypeSetPartitionStatus 	0x26
#define MTypeGetBackupProgress 		0x27
#define MTypeGetRestoreProgress		0x28
#define MTypeRestorePartition		0x29
#define MTypeLastPartition			0x29
#define MTypeMaxPartition			0x2F
//Message types data requisition
#define MTypeGetAllData				0x31
#define MTypeGetProcessData			0x32
#define MTypeGetPhysicalData		0x33
#define MTypeGetDeviceData			0x34
#define MTypeGetDummyMode			0x35
#define MTypeGetDosingData			0x36
#define MTypeGetSettings 			0x37
#define MTypeGetMachineParameters 	0x38
#define MTypeSetMachineParameters 	0x39
#define MTypeLastGetData			0x39
#define MTypeMaxGetData				0x3F
//Message types settings
#define MTypeSetLED				0x41
#define MTypeSetDummyMode		0x42
#define MTypeSetDosingData		0x43
#define MTypeLastSetting		0x43
#define MTypeMaxSetting			0x4F
//Message types machine actions
#define MTypeReboot				0x51
#define MTypeSwitchOn			0x52
#define MTypeStandby			0x53
#define MTypeStartEmbedded		0x54
#define MTypeLastMachineAction	0x54
#define MTypeMaxMachineAction	0x5F
//Message types consumable
#define MTypeGetConsumableData  0x61
#define MTypeSetConsumableData  0x62


#define MStatNone				0x00
#define MStatSuccess			0x01
#define MStatCRCError			0x11
#define MStatUnknownType		0x12
#define MStatExecuteTimeOut		0x13
#define MStatBlockNrError		0x14
#define MStatUARTVersionError	0x15
#define MStatNoValidRecipe		0x16
#define MStatRecipeNotActive	0x17
#define MStatNoZeroCross		0x18
#define MStatWrongVersion		0x19
#define MStatImageNotValid		0x1A
#define MStatVerifyError		0x1B
#define MStatErasePageError		0x1C
#define MStatCreateRollBackError	0x1D
#define MStatImageTooBig		0x1E
#define MStatImageCorrupt		0x1F

#define MStatFlow1Reached			0x8000
#define MStatFlow2Reached			0x4000
#define MStatPressure1Reached   	0x2000
#define MStatPressure2Reached   	0x1000
#define MStatTemperature1Reached   	0x800
#define MStatTemperature2Reached   	0x400
#define MStatTemperature3Reached   	0x200
#define MStatVolume1Reached   		0x100
#define MStatVolume2Reached		   	0x80
#define MStatTemperature4Reached   	0x40
#define MStatTimePassed				   	0x20
#define MStatTotalVolumeReached			0x10
//-----------------------------------------------------------------------------
//! \brief UART message Storage structure
typedef struct
{
	uint8_t RxBuffer;
	uint8_t UARTVersion;
	uint8_t Length;
	uint8_t MType;
	uint8_t CRCValue;
	int16_t Counter;
	uint8_t Complete;
	uint8_t RxData[MLengthMax];
} stcUARTMessage;
//-----------------------------------------------------------------------------
//! \brief  Private variables
extern stcUARTMessage Message;
extern uint8_t UartReturn;
uint16_t SER_GetValue (uint8_t newStartByte, uint8_t newLength);
//-----------------------------------------------------------------------------
//! \brief  Private functions
#ifndef BOOTLOADER
void SER_HandleRecipe (void);
#endif
void SER_ProcessMessage (void);
void SER_SendReturnMessage (uint8_t newLength, uint8_t newStatus);
void SER_SetStatus (uint8_t newStatus);

//-----------------------------------------------------------------------------
//! \brief  Public functions
extern void SER_FillData (uint8_t newPosition, uint8_t newByteCount, uint64_t newValue);
extern uint8_t SER_FillBuffer (uint8_t newPosition, uint16_t MType);
extern void SER_Handle (void);
extern void SER_HandleInterrupt(UART_HandleTypeDef *huart);
extern void SER_Init (void);
//-----------------------------------------------------------------------------
#endif /* INC_SERIAL_H_ */
