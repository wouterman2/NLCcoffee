//-----------------------------------------------------------------------------
//! \file       filter.c
//! \author     R. Weimar. RMB Holland bv
//! \brief      Contains routines for the filter module
//! \details
//! \Attention
//! \Created on: Apr 27, 2022

#include "steamwand.h"
#include "work.h"
//! \SteamWand structure
stcSteamWand WAN;


//-----------------------------------------------------------------------------
//! \brief      Handles the temperature measurement of the steamwand
//! \details    Handles the temperature measurement of the steamwand
//! \params	None
void WAN_Handle (void)
{
	static uint16_t TickTimer=7;
	if (TickTimer ++ < 99) return; //100 ms interval
	TickTimer = 0;

	if (!DUMMYMODE) WAN.GetTemperature = ANA_GetSensorValue(ADC_NTC4);
	if (MILK_PRESENT)
		WAN.StatusMilk = sEMPTY;
	else
		WAN.StatusMilk = sFULL;
}
//-----------------------------------------------------------------------------
//! \brief      Returns the status of the steam wand
//! \details    Returns the current status based n the switch
//! \param[out] enuStatus Position
enuStatus WAN_GetStatus (void)
{
	//To be added: Evaluation of the sensors to determine the position of the steam wand
	if (STEAMWANDINPOS)
	{
		if (WAN.StatusMilk == sEMPTY)
		{
			WAN.Status = sPARKEDNOFLUID;
		}
		else
		{
			WAN.Status = sPARKEDFLUID;
		}
	}
	else
	{
		if (WAN.StatusMilk == sEMPTY)
		{
			WAN.Status = sLIFTEDNOFLUID;
		}
		else
		{
			WAN.Status = sLIFTEDFLUID;
		}
	}
	return WAN.Status;
}

//-----------------------------------------------------------------------------
//! \brief      Gets the temperature of the mlik
//! \details    Gets the temperature of the milk
//! \param[out] uint32_t Temperature
uint32_t WAN_GetStatusMilk (void)
{
	return WAN.StatusMilk;
}

//-----------------------------------------------------------------------------
//! \brief      Gets the temperature of the mlik
//! \details    Gets the temperature of the milk
//! \param[out] uint32_t Temperature
uint32_t WAN_GetTemperature (void)
{
	return WAN.GetTemperature;
}

//-----------------------------------------------------------------------------
//! \brief      Sets the required temperature of the milk
//! \details    Sets the required temperature of the milk
//! \param[in]  uint32_t newTemperature
void WAN_SetTemperature (uint32_t newTemperature)
{
	WAN.GetTemperature = newTemperature;
}

