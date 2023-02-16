//-----------------------------------------------------------------------------
//! \file       AM2302.h
//! \author     R. Weimar
//! \brief      Contains routines for the AM2302 module
//! \details
//! \Attention
//-----------------------------------------------------------------------------

//! \brief  defines
#ifndef INC_AM2302_H_
#define INC_AM2302_H_
/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
//-----------------------------------------------------------------------------
#define DataHigh()		HAL_GPIO_WritePin(HT1_GPIO_Port, HT1_Pin,GPIO_PIN_SET)
#define DataLow()		HAL_GPIO_WritePin(HT1_GPIO_Port, HT1_Pin,GPIO_PIN_RESET)
#define DataIsHigh()	HAL_GPIO_ReadPin(HT1_GPIO_Port, HT1_Pin) == GPIO_PIN_SET
#define DataIsLow()		HAL_GPIO_ReadPin(HT1_GPIO_Port, HT1_Pin) == GPIO_PIN_RESET

//-----------------------------------------------------------------------------
//! \brief Sensor Storage structure
typedef struct
{
	uint8_t Counter;
	float Humidity;
	float Temperature;
	uint8_t CheckSum;
	uint8_t RawValues [5];
} StcSensor;
//-----------------------------------------------------------------------------
//! \brief  Public variables

//-----------------------------------------------------------------------------
//! \brief  Private functions
void AM_SetSensorGPIO (uint32_t newType);
//-----------------------------------------------------------------------------
//! \brief  Public functions
extern float AM_GetHumidity (void);
extern uint8_t AM_GetSensorCounter (void);
extern float AM_GetTemperature (void);
extern void AM_HandleSensor (void);
extern void AM_IncreaseSensorCounter (void);
extern void AM_SetTimerValue(uint8_t newCounter, uint16_t newTime);
extern void AM_UpdateValues (void);

#endif /* INC_AM2302_H_ */
