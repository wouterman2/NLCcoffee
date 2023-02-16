//-----------------------------------------------------------------------------
//! \file       heaters.h
//! \author     R. Weimar
//! \brief      Contains routines for the heater module
//! \details
//! \Attention
//-----------------------------------------------------------------------------


#ifndef INC_HEATERS_H_
#define INC_HEATERS_H_
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "work.h"

#define H1			0
#define H2			1
#define H3			2   //Brewer heater
#define H4			3	//Used for storing milk temperature only
#define H1L_ON()	HAL_GPIO_WritePin(H1L_GPIO_Port, H1L_Pin, GPIO_PIN_SET)
#define H1L_OFF()	HAL_GPIO_WritePin(H1L_GPIO_Port, H1L_Pin, GPIO_PIN_RESET)
#define H1H_ON()	HAL_GPIO_WritePin(H1H_GPIO_Port, H1H_Pin, GPIO_PIN_SET)
#define H1H_OFF()	HAL_GPIO_WritePin(H1H_GPIO_Port, H1H_Pin, GPIO_PIN_RESET)
#define H2L_ON()	HAL_GPIO_WritePin(H2L_GPIO_Port, H2L_Pin, GPIO_PIN_SET)
#define H2L_OFF()	HAL_GPIO_WritePin(H2L_GPIO_Port, H2L_Pin, GPIO_PIN_RESET)
#define H2H_ON()	HAL_GPIO_WritePin(H2H_GPIO_Port, H2H_Pin, GPIO_PIN_SET)
#define H2H_OFF()	HAL_GPIO_WritePin(H2H_GPIO_Port, H2H_Pin, GPIO_PIN_RESET)
#define H3_ON()		HAL_GPIO_WritePin(H3_GPIO_Port, H3_Pin, GPIO_PIN_SET)
#define H3_OFF()	HAL_GPIO_WritePin(H3_GPIO_Port, H3_Pin, GPIO_PIN_RESET)
#define FTH_P      	0.25
#define FTH_I      	0.05
#define FTH_D      	0
#define BH_P       	0.25
#define BH_I       	0.05
#define BH_D       	0
#define BH_POWER	500
#define FTH_L_POWER	300
#define FTH_H_POWER	900
#define FTH_E_POWER	1	//Power setting for switching on both heaters at 10% for Efast measuring
#define MAXEFAST	20
#define CUTOFFEFAST	25



//! \brief typedefs
typedef struct
{
	enuStatus Status;
	uint16_t Power;
	uint32_t SetTemperature;
	uint32_t GetTemperature;
	int32_t TemperatureDeviation;
	uint32_t GetTemperatureOld;
	int32_t DeltaTemperature;
	uint32_t EfastCounter;
	uint32_t EfastCounterOld;
	uint32_t EfastCounterMax;
	uint32_t EfastValidValue;
	uint8_t EfastOffset;
	uint8_t EfastActive;
	uint32_t SetTime; //For H3
	uint8_t SteamMode; //0 = Normal mode, 1 = Steam Mode
	uint32_t Counter;
	float P_Heater;
	float I_Heater;
	float D_Heater;
	float ErrorP_Heater_Old;
	float ErrorP_Heater;
	float ErrorI_Heater;
	float ErrorD_Heater;
	float PID_Heater;
	float HeaterControl;
	float HeaterControlOld;
}stcHeater;

void HEA_HandlePID (void);
uint8_t HEA_SetControl (uint8_t newHeater, float newPower);

extern uint8_t PhaseCounter;
extern uint16_t PhaseList[];
extern stcHeater HEA[3];
extern uint32_t HEA_GetHeaterTemperature (uint8_t newHeater);
extern void HEA_Handle (void);
extern void HEA_HandleEfast(void);
extern void HEA_Set (uint8_t newHeater, enuStatus newStatus);
extern void HEA_IncEfast (uint8_t newHeater);
extern void HEA_ResetEfast(void);
extern void HEA_StartEfastMeasurement (void);
extern void HEA_SetHeaterGPIO (uint8_t newHeater);
extern void HEA_SetHeaterTemperature (uint8_t newHeater, uint16_t newTemperature, uint8_t newSteamMode);

#endif /* INC_HEATERS_H_ */
