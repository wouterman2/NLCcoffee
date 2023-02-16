//-----------------------------------------------------------------------------
//! \file       filter.h
//! \author     R. Weimar
//! \brief      Contains routines for the filter module
//! \details
//! \Attention
//-----------------------------------------------------------------------------

#ifndef INC_FILTER_H_
#define INC_FILTER_H_
/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "work.h"
//-----------------------------------------------------------------------------
#define HAL_NEG_THRESHOLD	1450
#define HAL_POS_THRESHOLD	1650

#define FilterCount 27
typedef struct
{
	enuStatus Status;
	uint8_t Type;
	uint8_t Trigger;
}stcFilter;


extern uint8_t FIL_GetStatus (void);
extern uint8_t FIL_GetTrigger (void);
extern stcFilter FIL;

#endif /* INC_FILTER_H_ */
