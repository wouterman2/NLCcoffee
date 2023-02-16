//-----------------------------------------------------------------------------
//! \file       recipe.c
//! \author     R. Weimar. RMB Holland bv
//! \brief      Contains routines for the recipe module
//! \details
//! \Attention
//! \Created on: June 9, 2022
//-----------------------------------------------------------------------------

#include "recipe.h"
#include "serial.h"
#include "work.h"
//! \Recipe structure of respectively coffee, milk and water
stcRecipe gRecipe[MTypeLastRecipe];


void REC_Set (uint8_t newRecipe, enuStatus newStatus)
{
	if (newStatus == sINACTIVE)
	{
		gRecipe[newRecipe].Active = 0;
		gRecipe[newRecipe].CurrentBlock = 255;
	}
	else if (newStatus == sACTIVE)
	{
		if (gRecipe[newRecipe].Active == 0)
		{
			gRecipe[newRecipe].GetTotalVolume = 0;
			gRecipe[newRecipe].Active = 1;
			gRecipe[newRecipe].CurrentBlock = 0;
		}
	}

}
//-----------------------------------------------------------------------------
//! \brief      Initializes the recipe module
//! \details    Initializes the recipe module
//! \params 	None
void REC_Init(void)
{
	for (uint8_t i=0; i < RCount; i++) REC_Set(i,sINACTIVE);
	gRecipe[RECIPE3].RecipeBlocks[0].H3SetTemp = 60;
	if (DUMMYMODE)
	{

	}
}

//-----------------------------------------------------------------------------
//! \brief      Returns the number of blocks in the recipe
//! \details    Returns the number of blocks in the recipe RecipeType
//! \param[out]	uint8_t Number of Blocks
uint8_t REC_GetNrOfBlocks(uint8_t RecipeType)
{
	return gRecipe[RecipeType].Blocks;
}

//-----------------------------------------------------------------------------
//! \brief      Returns the current block number the recipe
//! \details    Returns the current block number the recipe defined by RecipeType
//! \param[out]	uint8_t Active Block Number
uint8_t REC_GetCurrentBlock(uint8_t RecipeType)
{
	return gRecipe[RecipeType].CurrentBlock;
}
