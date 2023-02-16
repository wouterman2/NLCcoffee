//-----------------------------------------------------------------------------
//! \file       eeprom.h
//! \author     R. Weimar
//! \brief      Contains routines for the eeprom handling
//! \details
//! \Attention
//-----------------------------------------------------------------------------

#ifndef INC_EEPROM_H_
#define INC_EEPROM_H_
//-----------------------------------------------------------------------------
#include <stdint.h>

#define	WRK_DUMMYMODEGRINDER 	0
#define WRK_DUMMYMODE  			0
#define	WRK_DUMMYMODEFILTER 	1
#define	WRK_DUMMYMODERFID		2
#define WRK_DUMMYMODEH3  		2
#define	WRK_DUMMYMODESCALE 		3
#define	HOP_TIMEMIN 			10
#define	HOP_TIMEMAX				11
#define	HOP_DUMPANGLE 			12
#define	HOP_DOSEANGLE			13
#define	HOP_DOSESTEPSMAX		14
#define HOP_CLOSEDPULSETIME		15
#define HOP_OPENEDPULSETIME		16
#define	HOP_EEPROMFILLED 		19
#define	SCL_STABLETIME 			20
#define	SCL_EEPROMFILLED 		29
#define	FLA_APPLICATIONVERSION  30
#define	FLA_APPLICATIONSTATUS  	31
#define FLA_ROLLBACKVERSION		32
#define FLA_ROLLBACKSTATUS		33
#define	FLA_UPGRADEVERSION  	34
#define	FLA_UPGRADELASTBLOCK  	35
#define	FLA_UPGRADENROFBLOCKS  	36
#define	FLA_UPGRADESTATUS  		37
#define FLA_BOOTLOADERVERSION	38
#define PUM_ULPERPULSEP1		39
#define PUM_ULPERPULSEP2		40
#define ADJ_P					41  //P value * 100
#define ADJ_I					42  //I value * 1000
#define ADJ_D					43  //D value

extern void EEP_WriteEEPROM (uint32_t newAddress, uint32_t newValue);
extern uint32_t EEP_ReadEEPROM (int8_t newAddress);

#endif /* INC_EEPROM_H_ */
