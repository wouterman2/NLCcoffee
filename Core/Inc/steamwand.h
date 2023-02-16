//-----------------------------------------------------------------------------
//! \file       steamwand.h
//! \author     R. Weimar
//! \brief      Contains routines for the steam wand module
//! \details
//! \Attention
//-----------------------------------------------------------------------------


#ifndef INC_STEAMWAND_H_
#define INC_STEAMWAND_H_
/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "work.h"

typedef struct
{
	enuStatus Status;
	enuStatus StatusMilk;
	uint32_t GetTemperature;
	uint32_t SetTemperature;
}stcSteamWand;

#define MILK_PRESENT	HAL_GPIO_ReadPin(LS4_GPIO_Port, LS4_Pin)
#define STEAMWANDINPOS  HAL_GPIO_ReadPin(SW6_GPIO_Port, SW6_Pin)==GPIO_PIN_RESET

extern stcSteamWand WAN;
extern enuStatus WAN_GetStatus (void);
extern uint32_t WAN_GetStatusMilk (void);
extern uint32_t WAN_GetTemperature (void);
extern void WAN_Handle (void);
extern void WAN_SetTemperature (uint32_t newTemperature);

#endif /* INC_STEAMWAND_H_ */
