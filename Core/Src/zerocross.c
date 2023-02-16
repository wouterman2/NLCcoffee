//-----------------------------------------------------------------------------
//! \file       zerocross.c
//! \author     R. Weimar. RMB Holland bv
//! \brief      Contains routines for the zero cross detection module
//! \details
//! \Attention
//! \Created on: June 10, 2022
//-----------------------------------------------------------------------------
#include "zerocross.h"
#include "work.h"
//-----------------------------------------------------------------------------
//! \Global zero cross flag
uint8_t FlgZCR;
//! \Global ZCR structure
stcZeroCross ZCR;
//-----------------------------------------------------------------------------
//! \brief      Routine to handle timing of zero cross
//! \details    Used to calculate frequency and control the timers for phase cutting
//! \params 	None
void ZCR_Handle(void)
{
	static uint8_t TickTimer=40;

	if (DUMMYMODE)
	{
		ZCR.Status = sACTIVE;
		return;
	}
	if (TickTimer ++ < 99) return; //100 ms interval
	TickTimer = 0;
	if (ZCR.Status == sACTIVE)
	{
		if (ZCR.WatchDog < 2) ZCR.WatchDog ++;
	}
	else
	{
		ZCR.WatchDog = 0;
	}
	if (ZCR.WatchDog >= 2) //200 ms no zero cross detected.
	{
		ZCR.Status = sUNITERROR;
		ZCR.Frequency  = 0;
		ZCR.TimingValue = 0;
		ZCR.TimingValueOld = 0;
		ZCR.Pulses = 0;
	}
	else if (ZCR.TimingValue > ZCR.TimingValueOld)
	{
		if (ZCR.Status == sUNITERROR) ZCR.Status = sACTIVE; //Again pulses. ZCR is active again
		ZCR.Pulses = ZCR.TimingValue - ZCR.TimingValueOld;
		if (ZCR.Pulses != 0)
		{
			ZCR.Frequency = ((5000000/ZCR.Pulses)+5)/10 ;
		}
	}
}
//-----------------------------------------------------------------------------
//! \brief      Routine to switch zerocross on or off
//! \details    Zerocross is needed to control heaters and pumps.
//! \params 	enuStatus newStatus
void ZCR_Set(enuStatus newStatus)
{
	ZCR.Status = newStatus;
	if (newStatus == sACTIVE)
	{
		ENZC_ON();
	}
	else
	{
		ENZC_OFF();
		ZCR.Frequency = 0;
		ZCR.TimingValue = 0;
		ZCR.TimingValueOld = 0;
		ZCR.Pulses = 0;
	}
}
//-----------------------------------------------------------------------------
//! \brief      Routine to set timing of zero cross
//! \details    Timing is used to calculate frequency and control the timers for phase cutting
//! \params 	None
void ZCR_SetZeroCross(uint32_t newCounter)
{
	if ((ZCR.Status == sACTIVE) || (ZCR.Status == sUNITERROR))
	{
		ZCR.TimingValueOld = ZCR.TimingValue;
		ZCR.TimingValue = newCounter;
	}
	ZCR.WatchDog = 0;

}


//---------------- End of file -----------------------
