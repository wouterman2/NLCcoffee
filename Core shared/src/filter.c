//-----------------------------------------------------------------------------
//! \file       filter.c
//! \author     R. Weimar. RMB Holland bv
//! \brief      Contains routines for the filter module
//! \details
//! \Attention  IO for filter detection defined in coffee and grinder separately, because GPIO is different.
//!             See HAL0, HAL1 and HAL2 in main.h. Later will change to analog.
//!				Magnet direction is important now n=because of analog sensor on digital input.
//! \Created on: Apr 27, 2022

#include "filter.h"
#include "main.h"
#include "work.h"

//! \Filter structure
stcFilter FIL;
//! \HAL_Table definition
int8_t HAL_Table [FilterCount][3]=
{
	{0,0,0},//Filter type 0 (No Filter)
	{-1,0,0},//Filter type 1
	{1,0,0},//Filter type 2
	{0,0,-1},//Filter type 3
	{-1,0,-1},//Filter type 4
	{1,0,-1},//Filter type 5
	{0,0,1},//Filter type 6
	{-1,0,1},//Filter type 7
	{1,0,1},//Filter type 8
	{0,-1,0},//Filter type 9
	{-1,-1,0},//Filter type 10
	{1,-1,0},//Filter type 11
	{0,-1,-1},//Filter type 12
	{-1,-1,-1},//Filter type 13
	{1,-1,-1},//Filter type 14
	{0,-1,1},//Filter type 15
	{-1,-1,1},//Filter type 16
	{1,-1,1},//Filter type 17
	{0,1,0},//Filter type 18
	{-1,1,0},//Filter type 19
	{1,1,0},//Filter type 20
	{0,1,-1},//Filter type 21
	{-1,1,-1},//Filter type 22
	{1,1,-1},//Filter type 23
	{0,1,1},//Filter type 24
	{-1,1,1},//Filter type 25
	{1,1,1},//Filter type 26

};
int8_t HAL_Status[3];
//-----------------------------------------------------------------------------
//! \brief      Checks if a grinder should start
//! \details    Returns 1 if any filter type is detected
//! \param[out] uint8_t FilterType 0 = none, 1 - 14 = Type
uint8_t FIL_GetTrigger (void)
{
	static uint8_t FilterCounter = 0;
	uint8_t ReturnValue = 0; //0 means no filter
	if (!DUMMYMODEFILTER)
	{
		//if ((HAL0 <= HAL_NEG_THRESHOLD)||(HAL0 >= HAL_POS_THRESHOLD)) //HAL0 is triggered
		//	ReturnValue = 1;
		if ((HAL1 <= HAL_NEG_THRESHOLD)||(HAL1 >= HAL_POS_THRESHOLD))//HAL0 is triggered
		{
			//Only when we measure 5 times a value outside of the threshold, we trigger
			if (FilterCounter > 5)
			{
				if (FIL.Trigger == 0)
				{
					FIL.Trigger = 1;
					ReturnValue = 1;
				}
			}
			else
			{
				FilterCounter ++;
			}
		}
		else
		{
			FilterCounter = 0;
			FIL.Trigger = 0; //After releasing the HAL sensor, the trigger can be used again.
		}
		//if ((HAL2 <= HAL_NEG_THRESHOLD)||(HAL2 >= HAL_POS_THRESHOLD)) //HAL0 is triggered
		//	ReturnValue = 1;
	}
	return ReturnValue;
}


//-----------------------------------------------------------------------------
//! \brief      Returns the current filter type
//! \details    Returns the current filter type
//! \param[out] uint8_t FilterType 0 = none, 1 - 14 = Type
uint8_t FIL_GetStatus (void)
{
	//uint8_t ReturnValue = 0; //0 means no filter

	//uint8_t Match = 0;
	if (DUMMYMODEFILTER)
	{
		FIL.Type = 1;
	}
	else
	{
		//Temporary middle sensor is use only as trigger for the grinder
		//Evaluate HAL0
		if (HAL0 <= HAL_NEG_THRESHOLD) //HAL0 is negative
			HAL_Status[0] = -1;
		else if (HAL0 >= HAL_POS_THRESHOLD) //HAL0 is positive
			HAL_Status[0] = 1;
		else
			HAL_Status[0] = 0;
		//Evaluate HAL1 is canceled.
		if (HAL1 <= HAL_NEG_THRESHOLD) //HAL1 is negative
		{
			HAL_Status[1] = -1;
			FIL.Type = 1;
		}
		else if (HAL1 >= HAL_POS_THRESHOLD) //HAL1 is positive
		{
			HAL_Status[1] = 1;
			FIL.Type = 2;
		}
		else
		{
			HAL_Status[1] = 0;
			FIL.Type = 0;
		}
		//Evaluate HAL2 (middle sensor?)
		if (HAL2 <= HAL_NEG_THRESHOLD) //HAL2 is negative
		{
			HAL_Status[2] = -1;
		}
		else if (HAL2 >= HAL_POS_THRESHOLD) //HAL2 is positive
		{
			HAL_Status[2] = 1;
		}
		else
		{
			HAL_Status[2] = 0;
		}
		/*
		for (uint8_t i=0; i< FilterCount; i++)
		{
			Match = 0;
			for (uint8_t j=0; j<3;j+=2)
			{
				if (HAL_Table[i][j] == HAL_Status[j])
				{
					Match ++;
				}
			}
			if (Match == 2)
			{
				ReturnValue = i;
				i = FilterCount; //Filter found. Ignore rest
			}
		}
		*/
	}
	//FIL.Type = ReturnValue;
	return FIL.Type;
}




