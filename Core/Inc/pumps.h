//-----------------------------------------------------------------------------
//! \file       pumps.h
//! \author     R. Weimar
//! \brief      Contains routines for the pump module
//! \details
//! \Attention
//-----------------------------------------------------------------------------


//! \brief  defines
#ifndef INC_PUMP_H_
#define INC_PUMP_H_
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "work.h"
//-----------------------------------------------------------------------------
#define P1					0
#define P2					1
#define P3					2
//#define PUMP_P      		0.25
//#define PUMP_I      		0.05
//#define PUMP_D      		0
#define PUM_ULPERPULSE		404 // Datasheet is 419 What is the problem? //2382 pulses per liter -> 1000000 ul /2382 pulses = 419.815 pulses per ul
#define PUM_NOCONTROL		0
#define PUM_FLOWCONTROL		1
#define PUM_PRESSURECONTROL	2
#define PUM_BOTHCONTROL		3
#define NOFLOWCUTOFFTIME	2000

#define P1_ON()		HAL_GPIO_WritePin(P1_GPIO_Port, P1_Pin, GPIO_PIN_SET);
#define P1_OFF()	HAL_GPIO_WritePin(P1_GPIO_Port, P1_Pin, GPIO_PIN_RESET);
#define P2_ON()		HAL_GPIO_WritePin(P2_GPIO_Port, P2_Pin, GPIO_PIN_SET);
#define P2_OFF()	HAL_GPIO_WritePin(P2_GPIO_Port, P2_Pin, GPIO_PIN_RESET);
//! \brief typedefs
typedef struct
{
	enuStatus Status;
	uint8_t Percentage;
	uint32_t SetFlow; //ul/s
	uint32_t GetFlow; //ul/s
	uint32_t GetFlowTimeBased; //ul/s
	uint32_t SetPressure; //mbar
	uint32_t GetPressure; //mbar
	uint64_t GetVolume; //ul
	uint64_t GetTotalVolume; //ul
	uint64_t GetVolumeOld; //ul
	uint64_t SetVolume; //ul
	uint8_t SetControlType;
	uint8_t RegulateFlow;
	uint8_t RegulatePressure;
	uint32_t UlPerPulse;
	uint32_t Pulses;
	uint16_t PulseTime;
	uint16_t PulseTimeOld;
	uint8_t StartRegulating;
	uint8_t Outlet; //VALVENONE, VALVECOFFEE, VALVEMILK, VALVEWATER, VALVERELEASE
	uint8_t SteamMode;
	uint8_t ControlType; //(0 = Phase elimination [--~--~], 1 = Phase cutting)
	float P_Pump;
	float I_Pump;
	float D_Pump;
	float ErrorP_Pump_Old;
	float ErrorP_Pump;
	float ErrorI_Pump;
	float ErrorD_Pump;
	float PID_Pump;
	float PumpControl;
	float PumpControlOld;
}stcPump;
//-----------------------------------------------------------------------------
//! \brief private function prototypes
uint8_t PUM_SetControl (uint8_t newPump, uint8_t newPercentage);
void PUM_HandleFlowMeters(void);
void PUM_HandlePressureSensors (void);
void PUM_HandlePIDFlow (void);
void PUM_HandlePIDPressure (void);

//-----------------------------------------------------------------------------
//! \brief  Public variables
extern uint8_t FlgFM[2];
extern stcPump PUM[2];
extern uint8_t PhaseCounterPumps;
//-----------------------------------------------------------------------------
//! \brief  Public function prototypes
extern void PUM_CalculateFlow(uint8_t newPump);
extern void PUM_CalculateVolume (uint8_t newPump);
extern uint16_t PUM_GetFlow (uint8_t newPump);
extern uint16_t PUM_GetParameter (uint8_t newParameter);
extern uint16_t PUM_GetPressure (uint8_t newPump);
extern uint16_t PUM_GetVolume (uint8_t newPump);
extern void PUM_Handle (void);
extern void PUM_Init(void);
extern void PUM_Set (uint8_t newPump, uint32_t newStatus);
extern void PUM_SetControlType (uint8_t newPump, uint32_t newControlType);
extern void PUM_SetFlow (uint8_t newPump, uint32_t newFlow, uint8_t newSteamMode);
extern void PUM_SetParameter (uint8_t newParameter, uint16_t newValue);
extern void PUM_SetPressure (uint8_t newPump, uint32_t newPressure);
extern void PUM_SetPulses (uint8_t newPump);
extern void PUM_SetVolume (uint8_t newPump, uint32_t newVolume);
extern void PUM_SetPumpGPIO (uint8_t newPump, enuStatus newStatus);
#endif /* INC_PUMP_H_ */

//---------------- End of file -----------------------
