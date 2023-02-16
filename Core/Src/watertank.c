//-----------------------------------------------------------------------------
//! \file       watertank.c
//! \author     R. Weimar. RMB Holland bv
//! \brief      Contains routines for the analog measurement
//! \details
//! \Attention
//! \Created on: Apr 27, 2022
//-----------------------------------------------------------------------------
#include "watertank.h"
#include "analog.h"
#include "adc.h"

stcWaterTank WAT;
//uint16_t WAT_Pressure;
uint16_t Buffer[100];
//-----------------------------------------------------------------------------
//! \brief      Returns the water tank volume
//! \details    Returns the water tank volume
//! \param[out]	uint16_t Volume
uint16_t WAT_GetVolume (void)
{
	WAT.Volume = (uint16_t) ANA_GetSensorValue(ADC_LS1);
	return WAT.Volume;
}
//-----------------------------------------------------------------------------
//! \brief      Initalizes the water tank module
//! \details    Initalizes the water tank module
//! \params		None
void WAT_Init (void)
{
	WAT.Area = 11925.0f; //For test tank. Final tank is 17686.0f; //Area of the water tank
}




