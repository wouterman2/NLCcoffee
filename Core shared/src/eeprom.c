//-----------------------------------------------------------------------------
//! \file       filter.c
//! \author     R. Weimar. RMB Holland bv
//! \brief      Contains routines for the filter module
//! \details
//! \Attention  IO for filter detection defined in coffee and grinder separately, because GPIO is different.
//!             See HAL0, HAL1 and HAL2 in main.h. Later will change to analog.
//!				Magnet direction is important now n=because of analog sensor on digital input.
//! \Created on: Apr 27, 2022
#include "eeprom.h"
#include "stm32l1xx_hal.h"

/****************************************************************
  Function:     	WriteEEPROM
  Description:  	Used to store variables in EEprom
  Parameters:   	enuVariable newVariable, uint32_t newValue
  Return value: 	None
***************************************************************/
void EEP_WriteEEPROM (uint32_t newAddress, uint32_t newValue)
{
    uint32_t Address = 0x8080000 + (4 * newAddress);
    HAL_FLASHEx_DATAEEPROM_Unlock();
    HAL_FLASHEx_DATAEEPROM_Erase(FLASH_TYPEERASEDATA_WORD,Address);
    HAL_FLASHEx_DATAEEPROM_Program (FLASH_TYPEPROGRAMDATA_WORD, Address, newValue);
    HAL_FLASHEx_DATAEEPROM_Lock();
}
/****************************************************************
  Function:     	ReadEEPROM
  Description:  	Used to read variables from EEprom
  Parameters:   	enuVariable newVariable
  Return value: 	None
***************************************************************/
uint32_t EEP_ReadEEPROM (int8_t newAddress)
{
    return (*(__IO uint32_t*) (0x08080000 + (4 * (uint32_t) newAddress)));
}

