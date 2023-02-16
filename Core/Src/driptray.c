//-----------------------------------------------------------------------------
//! \file       driptray.c
//! \author     R. Weimar. RMB Holland bv
//! \brief      Contains routines for the drip tray module
//! \details
//! \Attention
//! \Created on: Apr 27, 2022

#include "driptray.h"

//! \Drip tray structure
stcDripTray DRP;
//-----------------------------------------------------------------------------
//! \brief      Returns the drip tray status
//! \details    Returns the current drip tray status
//! \param[out] enuStatus DripTrayStatus
uint8_t DRP_GetStatus (void)
{
	if (DRP_Present())
	{
		DRP.Status = sPLACED;
	}
	else
	{
		DRP.Status = sNOTPLACED;
	}
	if (DRP_Full()) //Also fire if driptray placed is not detected.
	{
		DRP.Status = sFULL;
	}
	return DRP.Status;
}


