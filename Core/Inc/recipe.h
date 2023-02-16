//-----------------------------------------------------------------------------
//! \file       recipe.h
//! \author     R. Weimar
//! \brief      Contains routines for the recipe module
//! \details
//! \Attention
//-----------------------------------------------------------------------------
#ifndef INC_RECIPE_H_
#define INC_RECIPE_H_
/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "serial.h"
//-----------------------------------------------------------------------------
#define RBlocksCount	8
#define RECIPE1			0
#define RECIPE2			1
#define RECIPE3			2
#define RCount			3
#define ActiveBlock		gRecipe[newRecipe].RecipeBlocks[gRecipe[newRecipe].CurrentBlock]

//-----------------------------------------------------------------------------
//! \brief Recipe Storage structure
typedef struct
{
	uint8_t H1SetTemp;
	uint8_t H2SetTemp;
	uint8_t H3SetTemp;
	uint8_t MFSetTemp;
	uint8_t P1Flow;
	uint8_t P2Flow;
	uint8_t P3Flow;
	uint8_t P1Pressure;
	uint8_t P2Pressure;
	uint16_t P1Volume;
	uint16_t P2Volume;
	uint8_t Valves;
	uint8_t Piston;
	uint16_t Time;
	uint8_t ControlType1;
	uint8_t ControlType2;
	uint16_t EndConditions1;
	uint16_t EndConditions2;
	uint16_t EndConditions3;
	uint16_t EndConditions4;
} stcRecipeBlock;
typedef struct
{
	uint8_t Valid;
	uint8_t Active;
	uint8_t Blocks;
	uint8_t CurrentBlock;
	uint8_t UsingCircuit1;
	uint8_t UsingCircuit2;
	uint8_t UsingH1;
	uint8_t UsingH2;
	uint8_t UsingH3;
	uint8_t UsingP1;
	uint8_t UsingP2;
	uint8_t UsingP3;
	uint8_t UsingS3;
	uint64_t SetTotalVolume;
	uint64_t GetTotalVolume;
	stcRecipeBlock RecipeBlocks[RBlocksCount];
}stcRecipe;
//-----------------------------------------------------------------------------
//! \brief  Public variables
extern stcRecipe gRecipe[MTypeLastRecipe];
//! \brief  Public functions
extern uint8_t REC_GetNrOfBlocks(uint8_t RecipeType);
extern uint8_t REC_GetCurrentBlock(uint8_t RecipeType);
extern void REC_Init(void);
extern void REC_Set (uint8_t newRecipe, enuStatus newStatus);
#endif /* INC_RECIPE_H_ */
