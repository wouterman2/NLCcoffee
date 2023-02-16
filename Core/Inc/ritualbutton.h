//-----------------------------------------------------------------------------
//! \file       ritualbutton.h
//! \author     R. Weimar
//! \brief      Contains routines for the ritual button module
//! \details
//! \Attention
//-----------------------------------------------------------------------------


#ifndef INC_RITUALBTNCOFFEE_H_
#define INC_RITUALBTNCOFFEE_H_
/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "work.h"

#define BTN_COFFEE			0
#define BTN_MILK			1
#define BTN_PUSHED			0
#define BTN_RELEASED		1
#define BTN_SHORTPRESSED	2
#define BTN_LONGPRESSED		3
#define BTN_WAITFORRELEASED	4
#define BTN_ACTIVE		1
#define BTN_INACTIVE	0
#define BTN_SHORTPRESSTIMEMIN	100  //Minimal 100 ms push is needed
#define BTN_SHORTPRESSTIMEMAX	500	 //Maximal 500 ms push is needed. Longer will not trigger short press
#define BTN_LONGPRESSTIMEMIN	1500 //Minimal 3000 ms push is needed
#define BTN_LONGPRESSTIMEMAX	6000 //Maximal 6000 ms push is needed. Longer will not trigger long press
#define BTN_RESETSTATUSTIME		500	 //Time the button status remains active after releasing the button. Mean Linux has 500 ms to get the status

#define STATE_COFFEE_UP	  	HAL_GPIO_ReadPin(SW1A_GPIO_Port, SW1A_Pin)
#define STATE_COFFEE_DOWN 	HAL_GPIO_ReadPin(SW1B_GPIO_Port, SW1B_Pin)
#define STATE_MILK	  		HAL_GPIO_ReadPin(SW2_GPIO_Port, SW2_Pin)

typedef struct
{
	enuStatus Status;
	enuStatus Direction;
	uint16_t TimePushed;
	uint16_t TimeReleased;
	uint8_t InShortPressRange;
	uint8_t InLongPressRange;

}stcRitualButton;

extern enuStatus BTN_GetStatus (uint8_t newButton);
extern enuStatus BTN_GetDirection (uint8_t newButton);
extern void BTN_Handle (void);
#endif /* INC_RITUALBTNCOFFEE_H_ */
