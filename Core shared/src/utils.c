//-----------------------------------------------------------------------------
//! \file       utils.c
//! \author     R. Weimar. RMB Holland bv
//! \brief      Contains utilities
//! \details
//! \Attention
//! \Created on: Apr 30, 2022
//-----------------------------------------------------------------------------
#include "utils.h"
//-----------------------------------------------------------------------------
//! \brief      Returns the integer value of a hex string
//! \details    converts the hex string to an integer
//! \param[in]  uint8_t newValue		Hex string [1 byte]
//! \param[out]	uint8_t ReturnValue
uint8_t UTL_Asc2Dec (uint8_t newValue)
{
	if (newValue >= '0' && newValue <= '9') return newValue - '0';
	if (newValue >= 'A' && newValue <= 'F') return newValue - ('A' - 10);
	if (newValue >= 'a' && newValue <= 'f') return newValue - ('a' - 10);
	return 0xFF;
}
//-----------------------------------------------------------------------------
//! \brief      Returns the ascii code of an integer
//! \details    converts an integer to a ascii character
//! \param[in]  uint8_t newValue
//! \param[out] uint8_t returnValue
uint8_t UTL_Dec2Asc (uint8_t newValue)
{
	uint8_t returnValue = newValue + 48;
	if (returnValue > 57) //More than 9, so A,B,C,D,E or F
		returnValue += 7;
	return returnValue;
}
//End of file
