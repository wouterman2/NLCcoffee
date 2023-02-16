//-----------------------------------------------------------------------------
//! \file       airpump.h
//! \author     R. Weimar
//! \brief      Contains routines for the air pump module
//! \details
//! \Attention
//-----------------------------------------------------------------------------


#ifndef INC_AIRPUMP_H_
#define INC_AIRPUMP_H_
/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "work.h"
/* Defines -------------------------------------------------------------------*/
#define AIR_Control		TIM3->CCR2

typedef struct
{
	enuStatus Status;
	uint8_t Percentage;
}stcAirPump;

extern uint8_t AIR_GetPercentage (void);
extern void AIR_Init (void);
extern void AIR_Set (uint8_t newPercentage);

#endif /* INC_AIRPUMP_H_ */
