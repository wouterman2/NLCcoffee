//-----------------------------------------------------------------------------
//! \file       zerocross.h
//! \author     R. Weimar
//! \brief      Contains routines for the zero cross detection module
//! \details
//! \Attention
//-----------------------------------------------------------------------------


#ifndef INC_ZEROCROSS_H_
#define INC_ZEROCROSS_H_
/* Includes ------------------------------------------------------------------*/
#include "work.h"
//-----------------------------------------------------------------------------
//! \brief typedefs
typedef struct
{
	enuStatus Status;
	uint16_t Frequency; //in 0.1 Hz
	uint32_t TimingValue;
	uint32_t TimingValueOld;
	uint32_t Pulses;
	uint8_t WatchDog;
}stcZeroCross;

extern uint8_t FlgZCR;;
extern stcZeroCross ZCR;

//-----------------------------------------------------------------------------
//! \brief  Public function prototypes
extern void ZCR_Handle(void);
extern void ZCR_Set(enuStatus newStatus);
extern void ZCR_SetZeroCross(uint32_t newCounter);

#endif /* INC_ZEROCROSS_H_ */

//---------------- End of file -----------------------
