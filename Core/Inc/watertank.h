//-----------------------------------------------------------------------------
//! \file       analog.h
//! \author     R. Weimar
//! \brief      Contains routines for analog measurement
//! \details
//! \Attention
//-----------------------------------------------------------------------------

#ifndef INC_WATERTANK_H_
#define INC_WATERTANK_H_
/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "analog.h"
//-----------------------------------------------------------------------------

typedef struct
{
	uint16_t Volume; //Volume in ml
	uint16_t Area;
} stcWaterTank;
extern stcWaterTank WAT;
extern uint16_t WAT_GetVolume (void);
extern void WAT_Init (void);

#endif /* INC_WATERTANK_H_ */
