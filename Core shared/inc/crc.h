//-----------------------------------------------------------------------------
//! \file       crc.h
//! \author     R. Weimar
//! \brief      Contains routines for CRC calculation
//! \details
//! \Attention
//-----------------------------------------------------------------------------
#ifndef INC_CRC_H_
#define INC_CRC_H_
/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
//-----------------------------------------------------------------------------
//! \brief Public functions
extern uint8_t CalculateCRC8(uint8_t *pdata, uint8_t size);
//-----------------------------------------------------------------------------
#endif /* INC_CRC_H_ */
