//-----------------------------------------------------------------------------
//! \file       interrupt.c
//! \author     R. Weimar. RMB Holland bv
//! \brief      Contains interrupt handlers
//! \details
//! \Attention
//! \Created on: May 5, 2022
//-----------------------------------------------------------------------------
#include "interrupt.h"
#include "pumps.h"
#include "heaters.h"
#include "tim.h"
//-----------------------------------------------------------------------------
//! \brief      Handles the timer elapsed event
//! \details    Handles the elapsed event from all timers
//! \param[in]	TIM_HandleTypeDef* htim
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim)
{
	if(htim->Instance==TIM9)
	{
		PhaseCounter ++;
		if (PhaseCounter > 9) PhaseCounter = 0;
		PhaseCounterPumps ++;
		if (PhaseCounterPumps > 19) PhaseCounterPumps = 0;
		PUM_SetPumpGPIO (P1, sOFF);
		PUM_SetPumpGPIO (P2, sOFF);
		HEA_SetHeaterGPIO(H1); //FTH1 heater
		HEA_SetHeaterGPIO(H2); //FTH2 heater
		HEA_SetHeaterGPIO(H3); //Brewer heater

	}
}
//-----------------------------------------------------------------------------
//! \brief      Handles the timer output compare event
//! \details    Handles the output compare event from all timers
//! \param[in]	TIM_HandleTypeDef* htim
void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef* htim)
{

	if(htim->Instance==TIM9)
	{
		if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
		{
			if (PUM[P1].ControlType == 1) PUM_SetPumpGPIO (0, sON);
		}

		if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
		{
			if (PUM[P2].ControlType == 1) PUM_SetPumpGPIO (1, sON);
		}
	}
}

//---------------- End of file -----------------------
