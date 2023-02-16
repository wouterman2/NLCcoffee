//-----------------------------------------------------------------------------
//! \file       ritualbutton.c
//! \author     R. Weimar. RMB Holland bv
//! \brief      Contains routines for the ritual button module
//! \details
//! \Attention
//! \Created on: Apr 27, 2022

#include "ritualbutton.h"

//! \Ritual button structure
stcRitualButton BTN[2];

void BTN_Process (uint8_t newButton, uint8_t newState, enuStatus newDirection)
{
	if (newDirection != sUNDEFINED)
	{
		BTN[newButton].Direction = newDirection;
	}
	if (newState == BTN_PUSHED)
	{
		if (BTN[newButton].TimePushed < 0xffff) BTN[newButton].TimePushed++;
		BTN[newButton].TimeReleased = 0;
		if ((BTN[newButton].TimePushed >= BTN_SHORTPRESSTIMEMIN) && (BTN[newButton].TimePushed <= BTN_SHORTPRESSTIMEMAX))
		{
			BTN[newButton].InShortPressRange = 1;
		}
		else
		{
			BTN[newButton].InShortPressRange = 0;
		}
		if (BTN[newButton].TimePushed >= BTN_LONGPRESSTIMEMIN)
		{
			//Kumkeo wanted a longpress forever, until released //if (BTN[newButton].TimePushed <= BTN_LONGPRESSTIMEMAX)
			//{
				BTN[newButton].InLongPressRange = 1;
				BTN[newButton].Status = sLONGPRESSED;
			/*}
			else
			{
				BTN[newButton].InLongPressRange = 0;
				BTN[newButton].Status = sOFF;
			}
			*/
		}
	}
	else
	{
		if (BTN[newButton].InShortPressRange == 1)
		{
			BTN[newButton].Status = sSHORTPRESSED;
		}
		if (BTN[newButton].TimeReleased < 0xffff) BTN[newButton].TimeReleased++;
		if (BTN[newButton].TimeReleased > BTN_RESETSTATUSTIME)
		{
			BTN[newButton].Status = sOFF;
			BTN[newButton].Direction = sUNDEFINED;
		}
		BTN[newButton].TimePushed = 0;
		BTN[newButton].InShortPressRange = 0;
		BTN[newButton].InLongPressRange = 0;
	}
}

//-----------------------------------------------------------------------------
//! \brief      Handles the button status
//! \details    Checks the time buttons are pushed and creates status
//! \params		None
void BTN_Handle (void)
{
	//If both light sensors are active, the ritual button is malfunctioning or not connected.
	if ((STATE_COFFEE_UP == BTN_ACTIVE) && (STATE_COFFEE_DOWN == BTN_ACTIVE))
	{
		BTN[BTN_COFFEE].TimeReleased = 0;
		BTN[BTN_COFFEE].TimePushed = 0;
		BTN[BTN_COFFEE].Status = sUNITERROR;
		BTN[BTN_COFFEE].InShortPressRange = 0;
		BTN[BTN_COFFEE].InLongPressRange = 0;
	}
	else
	{
		if (STATE_COFFEE_DOWN == BTN_ACTIVE)
		{
			BTN_Process (BTN_COFFEE, BTN_PUSHED, sDOWNWARDS);
		}
		else if (STATE_COFFEE_UP == BTN_ACTIVE)
		{
			BTN_Process (BTN_COFFEE, BTN_PUSHED, sUPWARDS);
		}
		else if (STATE_COFFEE_DOWN == BTN_INACTIVE)
		{
			BTN_Process (BTN_COFFEE, BTN_RELEASED, sUNDEFINED);
		}
		else if (STATE_COFFEE_UP == BTN_INACTIVE)
		{
			BTN_Process (BTN_COFFEE, BTN_RELEASED, sUNDEFINED);
		}
	}
	if (STATE_MILK == BTN_PUSHED)
	{
		BTN_Process (BTN_MILK, BTN_PUSHED, sUNDEFINED);
	}
	else if (STATE_MILK == BTN_RELEASED)
	{
		BTN_Process (BTN_MILK, BTN_RELEASED, sUNDEFINED);
	}
}
//-----------------------------------------------------------------------------
//! \brief      Returns the selected button status
//! \details    Returns the status of the button defined by newButton
//! \param[in]  uint8_t newButton (o = Coffee, 1 = Milk)
//! \param[out] enuStatus ButtonStatus
enuStatus BTN_GetStatus (uint8_t newButton)
{
	enuStatus newStatus = BTN[newButton].Status;
	//Kumkeo wants longpress forever
	//if (newStatus == sLONGPRESSED)
	/*{
		BTN[newButton].TimePushed = BTN_LONGPRESSTIMEMAX; //Prevent firing again after request
	}
	*/
	return newStatus;
}

//-----------------------------------------------------------------------------
//! \brief      Returns the current filter type
//! \details    Returns the current filter type
//! \param[in]  uint8_t newButton (o = Coffee, 1 = Milk)
//! \param[out] enuStatus ButtonStatus
enuStatus BTN_GetDirection (uint8_t newButton)
{
	//To be added: Evaluation of the sensors to determine the filter type
	return BTN[newButton].Direction;
}

