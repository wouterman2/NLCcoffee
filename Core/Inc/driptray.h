//-----------------------------------------------------------------------------
//! \file       driptray.h
//! \author     R. Weimar
//! \brief      Contains routines for the drip tray module
//! \details
//! \Attention
//-----------------------------------------------------------------------------


#ifndef INC_DRIPTRAY_H_
#define INC_DRIPTRAY_H_
/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "work.h"

#define DRP_Full()	  		HAL_GPIO_ReadPin(LS2_GPIO_Port, LS2_Pin)==0
#define DRP_Present()	  	HAL_GPIO_ReadPin(LS3_GPIO_Port, LS3_Pin)==0
#define DRP_DetectionOn()	HAL_GPIO_WritePin(ENHF_GPIO_Port, ENHF_Pin, GPIO_PIN_RESET)
#define DRP_DetectionOff()	HAL_GPIO_WritePin(ENHF_GPIO_Port, ENHF_Pin, GPIO_PIN_SET)

typedef struct
{
	enuStatus Status;
}stcDripTray;

extern uint8_t DRP_GetStatus (void);
#endif /* INC_DRIPTRAY_H_ */
