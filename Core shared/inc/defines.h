//-----------------------------------------------------------------------------
//! \file       defines.h
//! \author     R. Weimar
//! \brief      Contains defines for coffee machine and grinder
//! \details
//! \Attention
//-----------------------------------------------------------------------------
#ifndef INC_DEFINES_H_
#define INC_DEFINES_H_

#include <stdint.h>
#define TASK_UNDEFINED  0
#define TASK_READY		1
#define TASK_ERROR		2
#define TASK_BUSY		3
//! \brief typedefs
typedef enum
{
	sUNDEFINED = 0,
	sINITIALIZE = 1,
	sIDLE = 2,
	sACTIVE = 3,
	sINACTIVE = 4,
	sSTANDBY = 5,
	sWAIT = 6,
	sON = 7,
	sOFF = 8,
	sNOTINPOSITION = 9,
	sINPOSITION = 10,
	sNOTPLACED = 11,
	sPLACED = 12,
	sFULL = 13,
	sEMPTY = 14,
	sSHORTPRESSED = 15,
	sLONGPRESSED = 16,
	sUPWARDS = 17,
	sDOWNWARDS = 18,
	sPREPARE = 19,
	sUNITERROR = 20,
	sHARDWAREINCOMPATIBLE = 21,
	sHOME = 22,
	sLIFTEDFLUID = 23,
	sLIFTEDNOFLUID = 24,
	sPARKEDFLUID = 25,
	sPARKEDNOFLUID = 26,
}enuStatus;
typedef enum
{
	pUNDEFINED = 0,
	pINITIALIZE = 1,
	pINACTIVE = 2,
	pWAIT = 3,
	pWAITFORCONDITIONS = 4,
	pWAITFOROTHERRECIPE = 5,
	pDOSE = 6,
	pWEIGH = 7,
	pGRIND = 8,
	pADJUST = 9,
	pERROR = 10,
} enuPrepare;

typedef struct
{
	enuStatus MainStatus;
	enuStatus MainStatusOld;
	enuStatus SubStatus;
	enuStatus SubStatusOld;
	enuPrepare PrepareStatus[3];
	uint32_t CounterSequence;
	uint32_t CounterAction;
}stcWRK;

#endif /* INC_DEFINES_H_ */
