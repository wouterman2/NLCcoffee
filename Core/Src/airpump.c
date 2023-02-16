//-----------------------------------------------------------------------------
//! \file       airpump.c
//! \author     R. Weimar. RMB Holland bv
//! \brief      Contains routines for the air pump module
//! \details
//! \Attention
//! \Created on: June 12, 2022
//-----------------------------------------------------------------------------
#include "airpump.h"
#include "work.h"
#include "tim.h"
//! \Air Pump structure
stcAirPump AIR;

//-----------------------------------------------------------------------------
//! \brief        Initializes the airpump module
//! \details      Starts the PWM control
//! \params       None
void AIR_Init (void)
{
	HAL_TIM_Base_Start(&htim3);
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
}
//-----------------------------------------------------------------------------
//! \brief        Sets the percentage of the air pump
//! \details      Sets the percentage and status of the air pump
//! \param[in]	  uint8_t newPercentage (0 - 100%)
void AIR_Set (uint8_t newPercentage)
{
	if (newPercentage > 100) newPercentage = 100;
	AIR.Percentage = newPercentage;

	if (DUMMYMODE) return; //In dummymode don't switch on the pump
	AIR_Control = (uint8_t) ((float) newPercentage);// * 0.66f); //pump is 3V CCR2=66 -> 3V
	if (newPercentage > 0)
	{
		AIR.Status = sACTIVE;
	}
	else
	{
		AIR.Status = sINACTIVE;
	}
}
//-----------------------------------------------------------------------------
//! \brief        Gets the percentage of the air pump
//! \details      Gets the percentage of the air pump
//! \param[out]	  uint8_t Percentage
uint8_t AIR_GetPercentage (void)
{
	if (DUMMYMODE)
	{

	}
	return AIR.Percentage;
}
