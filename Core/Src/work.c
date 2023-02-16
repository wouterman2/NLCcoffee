//-----------------------------------------------------------------------------
//! \file       work.c
//! \author     R. Weimar. RMB Holland bv
//! \brief      Contains routines for the general sequence
//! \details
//! \Attention
//! \Created on: May 5, 2022
//-----------------------------------------------------------------------------

#include "work.h"
#include "AM2302.h"
#include "valves.h"
#include "serial.h"
#include "watertank.h"
#include "led.h"
#include "zerocross.h"
#include "tim.h"
#include "pumps.h"
#include "recipe.h"
#include "heaters.h"
#include "ritualbutton.h"
#include "airpump.h"
#include "steamwand.h"


//! \Global tick time flag
uint8_t Flg1ms;
//! \Global Water tank volume temporary
float WaterTankVolume;
//! \Global temporary get humidity-temperature
uint8_t FlgGetHT = 0;
//! \Global temporary get water tank volume
uint8_t FlgGetWTV = 0;
uint8_t FlgGetP3 = 0;
//! \Global Work module structure
stcWRK WRK;
//! \Global flag to indicate all modules are initialized
uint8_t Initialized;
//! \Global action that is received from the display unit
uint8_t gAction;
uint8_t gActionAccepted;
//! \Global flag for HW0
uint8_t HW0;
//! \Global progress indicator
uint16_t Progress;
int32_t ProgressOld;
//! \Global dummymode settings
uint8_t DUMMYMODE = 0;  //Active when 1, Not active, when 0
uint8_t DUMMYMODEFILTER = 0;  //Active when 1, Not active, when 0
uint8_t DUMMYMODEH3 = 0;  //If H3 is missing set to 1



//-----------------------------------------------------------------------------
//! \brief      Routine to get an action to be executed
//! \details    Compares the required action with the active action
//! \details    Returns 1 and sets gActionAccepted if identical, Returns 0 if not
//! \param[in]  uint8_t ReqAction
uint8_t WRK_GetAction (uint8_t ReqAction)
{
	uint8_t returnValue = 0;
	if (gAction > 0)
	{
		if (ReqAction == gAction)
		{
			gAction = 0;
			gActionAccepted = 1;
			returnValue = 1;
		}
	}
	return returnValue;
}
//-----------------------------------------------------------------------------
//! \brief      Checks the end condition specified
//! \details    returns 1 or higher if any of the condition blocks are valid for the condition specified
//! \param[in]	uint8_t newValue
//! \param[in]	uint8_t newRecipe
//! \param[out]	uint8_t ReturnValue (0 = not valid, > 1 is valid)
uint8_t WRK_GetEndConditions (uint8_t newEndCondition, uint16_t newValue, uint8_t newRecipe)
{
	uint64_t ReturnValue = 0;
	switch (newEndCondition)
	{
		case 0:
		{
			ReturnValue = (ActiveBlock.EndConditions1 & newValue);
			break;
		}
		case 1:
		{
			ReturnValue = (ActiveBlock.EndConditions2 & newValue);
			break;
		}
		case 2:
		{
			ReturnValue = (ActiveBlock.EndConditions3 & newValue);
			break;
		}
		case 3:
		{
			ReturnValue = (ActiveBlock.EndConditions4 & newValue);
			break;
		}
		default: break;
	}
	if (ReturnValue > 0)
		ReturnValue = 1;
	return (uint8_t) ReturnValue;
}
//-----------------------------------------------------------------------------
//! \brief      Returns the hardware version
//! \details    Makes 1 integer of the hardware version that can be sent by UART
//! \params		None
uint32_t WRK_GetHardwareVersion (void)
{
	return (HW0 + HW1*2 + HW2*4 + HW3*8);
}
//-----------------------------------------------------------------------------
//! \brief      Returns the main status
//! \details    Returns the main status
//! \param[out]	enuStatus WRK.MainStatus
enuStatus WRK_GetMainStatus (void)
{
	return WRK.MainStatus;
}
//-----------------------------------------------------------------------------
//! \brief      Returns the software version
//! \details    Makes 1 integer of the software version that can be sent by UART
//! \params		None
uint32_t WRK_GetSoftwareVersion (void)
{
	return (SW_VER_MAJOR << 12) + (SW_VER_MINOR << 8) + SW_VER_BUGFIX;
}
//-----------------------------------------------------------------------------
//! \brief      Returns the sub status
//! \details    Returns the sub status
//! \param[out]	enuStatus WRK.MainStatus
enuStatus WRK_GetSubStatus (void)
{
	return WRK.SubStatus;
}
//-----------------------------------------------------------------------------
//! \brief      Routine to initiate return message or timeout
//! \details    Checks if the command timed out or is accepted
//! \details    Initiates the sending of the respective return message
//! \params 	None
void WRK_HandleAction (void)
{
	static uint8_t TickTime = 10;
	if (TickTime ++ < 99) return;
	TickTime = 0;

	if ((gAction > 0) || (gActionAccepted))
	{
		WRK.CounterAction += 100;
		if (WRK.CounterAction > ACTIONTIMEOUT)
		{
			if (gActionAccepted)
			{
				SER_SetStatus (0x01); //Action executed
			}
			else
			{
				SER_SetStatus (0x13); //Action timed out
			}
			gActionAccepted = 0;
			gAction = 0;
		}
		else if (gActionAccepted)
		{
			SER_SetStatus (0x01); //Action executed
			gActionAccepted = 0;
			gAction = 0;
		}
	}
}
//-----------------------------------------------------------------------------
//! \brief      Handles the IDLE status of sequence of the coffee machine
//! \details    Handles the IDLE status of sequence of the coffee machine
//! \param      None
void WRK_HandleIdle (void)
{
	//Add block number control for Linux
	if (gRecipe[RECIPE3].RecipeBlocks[0].H3SetTemp > 0)
	{
		gRecipe[RECIPE3].CurrentBlock = 0;
	}
	else
	{
		gRecipe[RECIPE3].CurrentBlock = 255;
	}
	switch (WRK.SubStatus)
	{
		case sUNDEFINED:
		{
			//Temporary: Later will be controlled from the Linux system
			LED_Set(STANDBYLED, FADEIN, 255, 255, 255, 50, 0, 0, 5, 0, 0); //On White
			LED_Set(WATERTANKLEDS, FADEIN, 0, 0, 255, 50, 0, 0, 5, 0, 0); //Breathing blue
			HAL_TIM_Base_Start_IT(&htim9);
			HAL_TIM_OC_Start_IT(&htim9, TIM_CHANNEL_1);
			//Pumps[0].Percentage = 50;
			//Pumps[1].Percentage = 50;
			//Set status to WAIT
			HEA_SetHeaterTemperature (H3, gRecipe[RECIPE3].RecipeBlocks[0].H3SetTemp * 10,0); //Heater 3 temperature set (Brew head) * 0.1 degrees
			HEA_Set(H3, sACTIVE);
			WRK_SetSubStatus (sWAIT);
			break;
		}
		case sWAIT:
		{
			if (WRK_GetAction(MTypeStandby)) //Received Switch to STANDBY command
			{
				WRK_SetMainStatus(sSTANDBY);
			}
			else if (WRK_GetAction(MTypeSwitchOn)) //Received Switch to IDLE command
			{
				WRK.MainStatus = sIDLE; //Not really needed, but put here to respond to UART quickly, since otherwise a time out would occur
			}
			else if (WRK_GetAction(MTypeStartRecipe1)) //Received Command 0x11: Start recipe 1
			{
				REC_Set(RECIPE1, sACTIVE);
				//gRecipe[RECIPE1].CurrentBlock = 0;
				//gRecipe[RECIPE1].Active = 1;
				REC_Set(RECIPE3, sINACTIVE);
				//gRecipe[RECIPE3].Active = 0;
				WRK_SetMainStatus(sPREPARE);
			}
			else if (WRK_GetAction(MTypeStartRecipe2)) //Received Command 0x12: Start recipe 2
			{
				REC_Set(RECIPE2, sACTIVE);
				//gRecipe[RECIPE2].CurrentBlock = 0;
				//gRecipe[RECIPE2].Active = 1;
				REC_Set(RECIPE3, sINACTIVE);
				//gRecipe[RECIPE3].Active = 0;
				WRK_SetMainStatus(sPREPARE);
			}
			for (uint8_t i = 0; i<2; i++)
			{
				HEA[i].EfastOffset = HEA[i].EfastCounterOld;
			}
			break;
		}
		default:
			break;
	}
}
//-----------------------------------------------------------------------------
//! \brief      Handles the PREPARE status of sequence of the coffee machine
//! \details    Handles the PREPARE status of sequence of the coffee machine
//! \param      None
void WRK_HandlePrepare (void)
{
	switch (WRK.SubStatus)
	{
		case sUNDEFINED:
		{
			//Check if all recipes are finished.
			if ((gRecipe[RECIPE1].Active == 0) && (gRecipe[RECIPE2].Active == 0))
			{
				WRK_ResetAllModules(RECIPE1);
				WRK_ResetAllModules(RECIPE2);
				WRK_SetMainStatus(sIDLE);
			}
			else //Not all are finished. Good to go.
			{
				WRK_SetSubStatus (sACTIVE);
			}
			break;
		}
		case sACTIVE:
		{
			for (uint8_t i= 0; i < RECIPE3; i++)
			{
				if (gRecipe[i].Active == 1) //Recipe x started
				{
					WRK_HandlePrepareRecipe(i);
				}
			}
			//Check if all recipes are finished
			if ((gRecipe[RECIPE1].Active == 0) && (gRecipe[RECIPE2].Active == 0))
			{
				WRK_ResetAllModules(RECIPE1);
				WRK_ResetAllModules(RECIPE2);

				if (DUMMYMODE)
				{
					HEA[H1].GetTemperature = 0;
					HEA[H2].GetTemperature = 0;
					HEA[H3].GetTemperature = 0;
					WAN.GetTemperature = 0;
					PUM[P1].GetFlowTimeBased = 0;
					PUM[P1].GetPressure = 0;
					PUM[P2].GetFlowTimeBased = 0;
					PUM[P2].GetPressure = 0;
				}
				else if (DUMMYMODEH3)
				{
					HEA[H3].GetTemperature = 0;
				}
				WRK_SetMainStatus(sIDLE);
			}
			//Check if other recipes are started
			if (WRK_GetAction(MTypeStartRecipe1)) //Received Command 0x11: Start recipe 1
			{
				/*if (gRecipe[RECIPE1].Active == 1) //Recipe already running
				{
					//Add error message
				}
				else
				{

					gRecipe[RECIPE1].CurrentBlock = 0;
					gRecipe[RECIPE1].Active = 1;
				}
				*/
				REC_Set(RECIPE1, sACTIVE);
			}
			else if (WRK_GetAction(MTypeStartRecipe2)) //Received Command 0x12: Start recipe 2
			{
				/*if (gRecipe[RECIPE2].Active == 1) //Recipe already running
				{
					//Add error message
				}
				else
				{
					REC_Set(RECIPE2, sACTIVE);
					//gRecipe[RECIPE2].CurrentBlock = 0;
					//gRecipe[RECIPE2].Active = 1;
				}
				*/
				REC_Set(RECIPE2, sACTIVE);
			}
			break;
		}
		default:
			break;
	}
}
//-----------------------------------------------------------------------------
//! \brief      Handles the actual preparing of coffee
//! \details    Handles the action needed to make a coffee
//! \param[in]	uint8_t newRecipe (RECIPE1, RECIPE2, RECIPE3)
void WRK_HandlePrepareRecipe (uint8_t newRecipe)
{
	uint8_t Circuit1InUse = 0, Circuit2InUse = 0, P3InUse = 0, S3InUse = 0; //Used to check circuit use other recipe (0 = not in use, 1 = in use)
	uint8_t EndConditionsMet[4];
	uint8_t UnitInError = 0;
	static uint32_t PrepareCounter = 0;

	if (PrepareCounter < 65535) PrepareCounter++; //1 tick = 100 ms

	if (WRK_GetAction(MTypeStopRecipe1 + newRecipe)) //Received Command 0x14, 0x15 or 0x16: Stop preparation
	{
		//Switch off everything in a safe way
		WRK_ResetAllModules(newRecipe);
		REC_Set(newRecipe, sINACTIVE);
		WRK.PrepareStatus[newRecipe] = pINACTIVE;
	}
	else
	{
		switch (WRK.PrepareStatus[newRecipe])
		{
			case pERROR:
			{
				//Switch off everything in a safe way
				WRK_ResetAllModules(newRecipe);
				REC_Set(newRecipe, sINACTIVE);
				break;
			}
			case pUNDEFINED:
			case pINACTIVE:
			{
				WRK.PrepareStatus[newRecipe] = pINITIALIZE;
				break;
			}
			case pINITIALIZE:
			case pWAITFOROTHERRECIPE:
			{
				//Check which circuits are in use by the other recipe
				Circuit1InUse = gRecipe[1-newRecipe].UsingCircuit1; //Circuit 1 available if not used in other recipe
				Circuit2InUse = gRecipe[1-newRecipe].UsingCircuit2; //Circuit 2 available if not used in other recipe
				P3InUse = gRecipe[1-newRecipe].UsingP3; //P3 available if not used in other recipe
				S3InUse = gRecipe[1-newRecipe].UsingS3; //S3 available if not used in other recipe
				//Check which circuits are needed for this recipe block
				gRecipe[newRecipe].UsingCircuit1 = 0;
				gRecipe[newRecipe].UsingCircuit2 = 0;
				if  ((ActiveBlock.Valves > CIRCUIT_1_OFF)&&(ActiveBlock.Valves <= CIRCUIT_1_WATER)) //Circuit 1 only
					gRecipe[newRecipe].UsingCircuit1 = 1;
				else if  ((ActiveBlock.Valves > CIRCUIT_2_OFF)&&(ActiveBlock.Valves <= CIRCUIT_2_WATER)) //Circuit 1 only
					gRecipe[newRecipe].UsingCircuit2 = 1;
				else if (ActiveBlock.Valves > CIRCUIT_1_2_OFF) //Circuit 1 and 2
					gRecipe[newRecipe].UsingCircuit1 = 1, gRecipe[newRecipe].UsingCircuit2 = 1;
				//Check if the circuit can be executed
				if ((gRecipe[newRecipe].UsingCircuit1 && Circuit1InUse) || (gRecipe[newRecipe].UsingCircuit2 && Circuit2InUse)) //Circuits intended cannot be used. Block cannot be executed at the moment
				{
					WRK.PrepareStatus[newRecipe] = pWAITFOROTHERRECIPE;
				}
				else if ((S3InUse) && (((ActiveBlock.Valves == CIRCUIT_1_WATER)&&(gRecipe[newRecipe].UsingCircuit1)) || ((ActiveBlock.Valves == CIRCUIT_2_WATER)&&(gRecipe[newRecipe].UsingCircuit2))))
				{
					//S3 in use by other recipe, so need to wait unitl the other reciep doesn't use S3 anymore
					WRK.PrepareStatus[newRecipe] = pWAITFOROTHERRECIPE;
				}
				if ((P3InUse) && (ActiveBlock.P3Flow > 0) && (((ActiveBlock.Valves == CIRCUIT_1_MILK)&&(gRecipe[newRecipe].UsingCircuit1)) || ((ActiveBlock.Valves == CIRCUIT_2_MILK)&&(gRecipe[newRecipe].UsingCircuit2))))
				{
					//P3 in use by other recipe, so need to wait until the other recipe doesn't use P3 anymore
					WRK.PrepareStatus[newRecipe] = pWAITFOROTHERRECIPE;
				}
				else
				{
					//Set the valves according to the recipe
					VAL_SetCircuit(ActiveBlock.Valves,0); //Set valves by copying whole buffer
					//Set UsingS3 if water circuit is used
					if (newRecipe==RECIPE1)
					{
						if (
								(ActiveBlock.Valves == CIRCUIT_1_WATER) ||
								(ActiveBlock.Valves == CIRCUIT_1_2_WATER)
							)
						{
							gRecipe[newRecipe].UsingS3 = 1;
						}
						else
						{
							gRecipe[newRecipe].UsingS3 = 0;
						}
					}
					else if (newRecipe==RECIPE2)
					{
						if (
								(ActiveBlock.Valves == CIRCUIT_2_WATER) ||
								(ActiveBlock.Valves == CIRCUIT_1_2_WATER)
							)
						{
							gRecipe[newRecipe].UsingS3 = 1;
						}
						else
						{
							gRecipe[newRecipe].UsingS3 = 0;
						}
					}
					AIR_Set (ActiveBlock.P3Flow); //Pump 3 Percentage set
					if (ActiveBlock.P3Flow > 0)
					{
						AIR_Set(ActiveBlock.P3Flow); //Switch airpump on at x %
						gRecipe[newRecipe].UsingP3 = 1;
					}
					else
					{
						AIR_Set(0); //Switch airpump off
						gRecipe[newRecipe].UsingP3 = 0;
					}
					if ((ActiveBlock.Valves == CIRCUIT_1_MILK)||(ActiveBlock.Valves == CIRCUIT_2_MILK)||(ActiveBlock.Valves == CIRCUIT_1_2_MILK))
					{
						WAN.SetTemperature = ActiveBlock.MFSetTemp*10;
						HEA_SetHeaterTemperature (H4, ActiveBlock.MFSetTemp,0); //Heater 4 temperature set (Milk foam)
					}

					HEA_SetHeaterTemperature (H3, ActiveBlock.H3SetTemp * 10,0); //Heater 3 temperature set (Brew head) * 0.1 degrees
					if (ActiveBlock.H3SetTemp > 0) HEA_Set(H3, sACTIVE), gRecipe[newRecipe].UsingH3 = 1; else HEA_Set(H3, sINACTIVE), gRecipe[newRecipe].UsingH3 = 0;
					if (gRecipe[newRecipe].UsingCircuit1) //Set and start the pumps and heater of the active block
					{
						if ((ActiveBlock.Valves == CIRCUIT_1_MILK)||(ActiveBlock.Valves == CIRCUIT_1_2_MILK))
						{
							HEA_SetHeaterTemperature (H1, ActiveBlock.H1SetTemp * 10, 1); //Heater 1 temperature set (FTH1) * 0.1 degrees and steammode set
							PUM_SetFlow (P1, ActiveBlock.P1Flow,1); //Pump 1 flow set
						}
						else
						{
							HEA_SetHeaterTemperature (H1, ActiveBlock.H1SetTemp * 10, 0); //Heater 1 temperature set (FTH1) * 0.1 degrees and steammode set
							PUM_SetFlow (P1, ActiveBlock.P1Flow,0); //Pump 1 flow set
						}
						PUM_SetPressure (P1, ActiveBlock.P1Pressure*100); //Pump 1 pressure set
						PUM_SetVolume (P1, ActiveBlock.P1Volume); //Pump 1 volume set
						PUM_SetControlType (P1, ActiveBlock.ControlType1); //Pump 1 control type set
						if (ActiveBlock.H1SetTemp > 0) HEA_Set(H1, sACTIVE), gRecipe[newRecipe].UsingH1 = 1; else HEA_Set(H1, sINACTIVE), gRecipe[newRecipe].UsingH1 = 0;
						if ((ActiveBlock.P1Flow > 0) || (ActiveBlock.P1Pressure > 0) || (ActiveBlock.P1Volume > 0)) PUM_Set(P1, sACTIVE), gRecipe[newRecipe].UsingP1 = 1; else PUM_Set(P1, sINACTIVE), gRecipe[newRecipe].UsingP1 = 0;
					}
					if (gRecipe[newRecipe].UsingCircuit2) //Set and start the pumps and heater of the active block
					{
						if ((ActiveBlock.Valves == CIRCUIT_2_MILK)||(ActiveBlock.Valves == CIRCUIT_1_2_MILK))
						{
							HEA_SetHeaterTemperature (H2, ActiveBlock.H2SetTemp * 10, 1); //Heater 2 temperature set (FTH2) * 0.1 degrees
							PUM_SetFlow (P2, ActiveBlock.P2Flow,1); //Pump 2 flow set
						}
						else
						{
							HEA_SetHeaterTemperature (H2, ActiveBlock.H2SetTemp * 10, 0); //Heater 2 temperature set (FTH2) * 0.1 degrees
							PUM_SetFlow (P2, ActiveBlock.P2Flow,0); //Pump 2 flow set
						}
						PUM_SetPressure (P2, ActiveBlock.P2Pressure*100); //Pump 2 pressure set
						PUM_SetVolume (P2, ActiveBlock.P2Volume); //Pump 2 volume set
						PUM_SetControlType (P2, ActiveBlock.ControlType2); //Pump 2 control type set
						if (ActiveBlock.H2SetTemp > 0) HEA_Set(H2, sACTIVE), gRecipe[newRecipe].UsingH2 = 1; else HEA_Set(H2, sINACTIVE), gRecipe[newRecipe].UsingH2 = 0;
						if ((ActiveBlock.P2Flow > 0) || (ActiveBlock.P2Pressure > 0) || (ActiveBlock.P2Volume > 0)) PUM_Set(P2, sACTIVE), gRecipe[newRecipe].UsingP2 = 1; else  PUM_Set(P2, sINACTIVE), gRecipe[newRecipe].UsingP2 = 0;
					}
					PrepareCounter = 0; //Reset the counter used for timing
					WRK.PrepareStatus[newRecipe] = pWAITFORCONDITIONS;
				}
				break;
			}
			case pWAITFORCONDITIONS:
			{
				if (DUMMYMODE)
				{
					//Simulate temperature H1
					if (HEA[H1].GetTemperature < HEA[H1].SetTemperature) //Temperature H1 not reached yet
					{
						if (HEA[H1].SetTemperature < 10) HEA[H1].SetTemperature  = 10; //Otherwise no temperature increase
						HEA[H1].GetTemperature  += HEA[H1].SetTemperature / 10; //Add temperature in 1 second
						if (HEA[H1].GetTemperature > HEA[H1].SetTemperature) HEA[H1].GetTemperature  = HEA[H1].SetTemperature ;
					}
					else if (HEA[H1].GetTemperature > HEA[H1].SetTemperature) //Temperature H1 too high
					{
						if (HEA[H1].GetTemperature >  HEA[H1].SetTemperature - 5)
						{
							HEA[H1].GetTemperature -= 5;
						}
						else
						{
							HEA[H1].GetTemperature = HEA[H1].SetTemperature;
						}
					}
					//Simulate temperature H2
					if (HEA[H2].GetTemperature < HEA[H2].SetTemperature) //Temperature H2 not reached yet
					{
						if (HEA[H2].SetTemperature < 10) HEA[H2].SetTemperature  = 10; //Otherwise no temperature increase
						HEA[H2].GetTemperature  += HEA[H2].SetTemperature / 10; //Add temperature in 1 second
						if (HEA[H2].GetTemperature > HEA[H2].SetTemperature) HEA[H2].GetTemperature  = HEA[H2].SetTemperature ;
					}
					else if (HEA[H2].GetTemperature > HEA[H2].SetTemperature) //Temperature H2 too high
					{
						if (HEA[H2].GetTemperature >  HEA[H2].SetTemperature - 5)
						{
							HEA[H2].GetTemperature -= 5;
						}
						else
						{
							HEA[H2].GetTemperature = HEA[H2].SetTemperature;
						}
					}
					//Simulate temperature H3
					if (HEA[H3].GetTemperature < HEA[H3].SetTemperature) //Temperature H3 not reached yet
					{
						if (HEA[H3].SetTemperature < 10) HEA[H3].SetTemperature  = 10; //Otherwise no temperature increase
						HEA[H3].GetTemperature  += HEA[H3].SetTemperature / 10; //Add temperature in 1 second
						if (HEA[H3].GetTemperature > HEA[H3].SetTemperature) HEA[H3].GetTemperature  = HEA[H3].SetTemperature ;
					}
					else if (HEA[H3].GetTemperature > HEA[H3].SetTemperature) //Temperature H3 too high
					{
						if (HEA[H3].GetTemperature >  HEA[H3].SetTemperature)
						{
							HEA[H3].GetTemperature --;
						}
					}
					//Simulate milk temperature
					if (WAN.GetTemperature < WAN.SetTemperature) //Temperature milk not reached yet
					{
						if (WAN.SetTemperature < 10) WAN.SetTemperature  = 10; //Otherwise no temperature increase
						WAN.GetTemperature  += WAN.SetTemperature/400;  //Add temperature in 40 second
						if (WAN.GetTemperature > WAN.SetTemperature) WAN.GetTemperature  = WAN.SetTemperature ;
					}
					//Simulate volume P1
					if ((PUM[P1].GetVolume < PUM[P1].SetVolume) && (PUM[P1].SetFlow) && (PUM[P1].SetControlType)) //Volume P1 not reached yet
					{
						PUM[P1].GetFlowTimeBased = PUM[P1].SetFlow;
						PUM[P1].GetPressure = PUM[P1].SetPressure;
						if (PUM[P1].SetFlow < 10) PUM[P1].SetFlow = 10; //Otherwise no volume counting
						PUM[P1].GetVolume += PUM[P1].SetFlow / 10; //Add volume per 100 ms.
						if (PUM[P1].GetVolume > PUM[P1].SetVolume) PUM[P1].GetVolume = PUM[P1].SetVolume;
					}
					//Simulate volume P2
					if ((PUM[P2].GetVolume < PUM[P2].SetVolume) && (PUM[P2].SetFlow) && (PUM[P2].SetControlType)) //Volume P2 not reached yet
					{
						PUM[P2].GetFlowTimeBased = PUM[P2].SetFlow;
						PUM[P2].GetPressure = PUM[P2].SetPressure;
						if (PUM[P2].SetFlow < 10) PUM[P2].SetFlow = 10; //Otherwise no volume counting
						PUM[P2].GetVolume += PUM[P2].SetFlow / 10; //Add volume per 100 ms.
						if (PUM[P2].GetVolume > PUM[P2].SetVolume) PUM[P2].GetVolume = PUM[P2].SetVolume;
					}
				}
				else if (DUMMYMODEH3) //Only H3 is missing
				{
					//Simulate temperature H3
					if (HEA[H3].GetTemperature < HEA[H3].SetTemperature) //Temperature H3 not reached yet
					{
						if (HEA[H3].SetTemperature < 10) HEA[H3].SetTemperature  = 10; //Otherwise no temperature increase
						HEA[H3].GetTemperature  += HEA[H3].SetTemperature / 10; //Add temperature in 1 second
						if (HEA[H3].GetTemperature > HEA[H3].SetTemperature) HEA[H3].GetTemperature  = HEA[H3].SetTemperature ;
					}
					else if (HEA[H3].GetTemperature > HEA[H3].SetTemperature) //Temperature H3 too high
					{
						if (HEA[H3].GetTemperature >  HEA[H3].SetTemperature)
						{
							HEA[H3].GetTemperature --;
						}
					}
				}
				//Evaluate all end conditions of the active block. If one end condition is not met, the EndConditions are not accepted
				for (uint8_t i = 0; i< 4; i++)
				{
					EndConditionsMet[i] = 1; //Set to 1 (Accepted, as the checks only can set to 0
					if ((WRK_GetEndConditions(i,MStatPressure1Reached, newRecipe)) && (PUM[P1].GetPressure < PUM[P1].SetPressure)) //Pressure P1 not reached yet
						EndConditionsMet[i] = 0;
					if ((WRK_GetEndConditions(i,MStatPressure2Reached, newRecipe)) && (PUM[P2].GetPressure < PUM[P2].SetPressure)) //Pressure P1 not reached yet
						EndConditionsMet[i] = 0;
					if ((WRK_GetEndConditions(i,MStatFlow1Reached, newRecipe)) && (PUM[P1].GetFlow < PUM[P1].SetFlow)) //Flow P1 not reached yet
						EndConditionsMet[i] = 0;
					if ((WRK_GetEndConditions(i,MStatFlow2Reached, newRecipe)) && (PUM[P2].GetFlow < PUM[P2].SetFlow)) //Flow P2 not reached yet
						EndConditionsMet[i] = 0;
					if ((WRK_GetEndConditions(i,MStatVolume1Reached, newRecipe)) && (PUM[P1].GetVolume < PUM[P1].SetVolume)) //Volume P1 not reached yet
						EndConditionsMet[i] = 0;
					if ((WRK_GetEndConditions(i,MStatVolume2Reached, newRecipe)) && (PUM[P2].GetVolume < PUM[P2].SetVolume)) //Volume P2 not reached yet
						EndConditionsMet[i] = 0;
					if ((WRK_GetEndConditions(i,MStatTemperature1Reached, newRecipe)) && (HEA[H1].GetTemperature < HEA[H1].SetTemperature)) //Temperature H1 not reached yet
						EndConditionsMet[i] = 0;
					if ((WRK_GetEndConditions(i,MStatTemperature2Reached, newRecipe)) && (HEA[H2].GetTemperature < HEA[H2].SetTemperature)) //Temperature H2 not reached yet
						EndConditionsMet[i] = 0;
					if ((WRK_GetEndConditions(i,MStatTemperature3Reached, newRecipe)) && (HEA[H3].GetTemperature < HEA[H3].SetTemperature)) //Temperature H3 not reached yet
						EndConditionsMet[i] = 0;
					if ((WRK_GetEndConditions(i,MStatTemperature4Reached, newRecipe)) && (WAN.GetTemperature < WAN.SetTemperature)) //Temperature H4 not reached yet
						EndConditionsMet[i] = 0;
					if ((WRK_GetEndConditions(i,MStatTimePassed, newRecipe)) && (PrepareCounter < ActiveBlock.Time)) //Time is not passed yet
						EndConditionsMet[i] = 0;
					if ((WRK_GetEndConditions(i,MStatTotalVolumeReached, newRecipe)) && ((gRecipe[newRecipe].SetTotalVolume) < (gRecipe[newRecipe].GetTotalVolume))) //Volume P1 + P2 not reached yet
						EndConditionsMet[i] = 0;
				}
				//Check for unit errors
				UnitInError = 0;
				if ((PUM[P1].Status == sUNITERROR) || (PUM[P2].Status == sUNITERROR))// || (PUM[P3].Status == sUNITERROR))
					UnitInError = 1;

				if (UnitInError == 1)
				{
					WRK.PrepareStatus[newRecipe] = pERROR;
				}
				else if (
							((EndConditionsMet[0] == 1) && (ActiveBlock.EndConditions1>0))||
							((EndConditionsMet[1] == 1) && (ActiveBlock.EndConditions2>0))||
							((EndConditionsMet[2] == 1) && (ActiveBlock.EndConditions3>0))||
							((EndConditionsMet[3] == 1) && (ActiveBlock.EndConditions4>0))
					    )//All active end conditions are met, so the next block can be started (Or the recipe ends if it is the last block)
				{
					if (DUMMYMODE)
					{
						if (PUM[P1].GetVolume >= PUM[P1].SetVolume)
						{
							PUM[P1].GetFlowTimeBased = 0;
							PUM[P1].GetPressure = 0;
						}
						if (PUM[P2].GetVolume >= PUM[P2].SetVolume)
						{
							PUM[P2].GetFlowTimeBased = 0;
							PUM[P2].GetPressure = 0;
						}
						if (HEA[H1].SetTemperature == 0) HEA[H1].GetTemperature = 0; //Switch off in dummy
						if (HEA[H2].SetTemperature == 0) HEA[H2].GetTemperature = 0; //Switch off in dummy
						if (HEA[H3].SetTemperature == 0) HEA[H3].GetTemperature = 0; //Switch off in dummy
						if (WAN.SetTemperature == 0) WAN.GetTemperature = 0; //Switch off in dummy
					}
					else if (DUMMYMODEH3)
					{
						if (HEA[H3].SetTemperature == 0) HEA[H3].GetTemperature = 0; //Switch off in dummy
					}
					gRecipe[newRecipe].CurrentBlock ++;
					if (gRecipe[newRecipe].CurrentBlock >= gRecipe[newRecipe].Blocks) //This was the last recipe block. Recipe ended
					{
						WRK_ResetAllModules(newRecipe); //Switch off the pumps and heaters used by this block
						WRK.PrepareStatus[newRecipe] = pINACTIVE;
					}
					else
					{
						WRK.PrepareStatus[newRecipe] = pINITIALIZE; //Check and start next block. Leave all heaters and pumps ACTIVE. The next block will stop them if needed
					}
				}
				break;
			}
			default:
				break;
		}
	}
}
//-----------------------------------------------------------------------------
//! \brief      Handles the sequence of the coffee machine
//! \details    Handles the different modules
//! \param      None
void WRK_HandleSequence (void)
{
	static uint32_t H3Counter = 0;
	static uint8_t TickTimer=30;
	if (TickTimer ++ < 99) return; //100 ms interval
	TickTimer = 0;

	//Brewer head heater handling
	//Should be on if the Recipe3 is Active.
	//Recipe3 can only be Active if Recipe 1 and 2 are not Active. If a recipe is activated Recipe3 is set to inactive
	//If brewer head heating is needed during recipe 1 or 2, H3 temperature should also be in Recipe1 and Recipe2.
	//Brewer heater head has a timeout of maximum 6553.5 seconds
	if (H3Counter < 65535)
		H3Counter ++;
	if ((WRK.MainStatus != sUNDEFINED) && (WRK.MainStatus != sSTANDBY))
	{
		if (WRK_GetAction(MTypeStartRecipe3)) //Received Command 0x13: Start recipe 3
		{
			REC_Set(RECIPE3, sACTIVE);
			//gRecipe[RECIPE3].Active = 1;
			H3Counter = 0;

		}
		if (WRK_GetAction(MTypeStopRecipe3)) //Received Command 0x16: Stop recipe 3
		{
			//gRecipe[RECIPE3].Active = 0;
			REC_Set(RECIPE3, sINACTIVE);
		}
		if (gRecipe[RECIPE3].RecipeBlocks[0].Time > 0)
		{
			if (H3Counter >= gRecipe[RECIPE3].RecipeBlocks[0].Time) //Time is x100 ms, H3Counter also
			{
				H3Counter = 0;
				//gRecipe[RECIPE3].Active = 0;
				REC_Set(RECIPE3, sINACTIVE);
			}
		}
	}
	else
	{
		H3Counter = 0;
		REC_Set(RECIPE3, sINACTIVE);
		//gRecipe[RECIPE3].CurrentBlock = 0;
		//gRecipe[RECIPE3].Active = 0;
	}

	switch (WRK.MainStatus)
	{
		case sUNDEFINED:
		{
			WRK_HandleUndefined();
			break;
		}
		case sSTANDBY:
		{
			WRK_HandleStandby();
			break;
		}
		case sIDLE: //Machine is switched from standby but is not performing any tasks
		{
			WRK_HandleIdle ();
			break;
		}
		case sPREPARE: //Machine is preparing a recipe
		{
			WRK_HandlePrepare ();
			break;
		}
		case sHARDWAREINCOMPATIBLE:
		{
			break;
		}
		default:
			break;
	}
}
//-----------------------------------------------------------------------------
//! \brief      Handles the STANDBY status of sequence of the coffee machine
//! \details    Handles the STANDBY status of sequence of the coffee machine
//! \param      None
void WRK_HandleStandby (void)
{
	switch (WRK.SubStatus)
	{
		case sUNDEFINED:
		{
			//Temporary: LEDs off
			if (LED[WATERTANKLEDS].Effect == SWITCHEDON)
				LED_Set(WATERTANKLEDS, FADEOUT, 0, 0, 0, LED[WATERTANKLEDS].Intensity, 0, 0, 5, 0, 0); //Fade out)
			else if (LED[WATERTANKLEDS].Effect != FADEOUT)
				LED_Set(WATERTANKLEDS, SWITCHEDOFF, 0, 0, 0, 0, 0, 0, 0, 0, 0); //Switched off)
			if (LED[STANDBYLED].Effect == SWITCHEDON)
				LED_Set(STANDBYLED, FADEOUT, 255, 255, 255, LED[STANDBYLED].Intensity, 0, 0, 5, 0, 0); //Fade out
			else if (LED[STANDBYLED].Effect != FADEOUT)
				LED_Set(STANDBYLED, BREATHING, 255, 0, 0, 50, 0, 0, 5, 0, 0); //Breathing Red
			//Temporary: All valves off
			VAL_SetCircuit (CIRCUIT_1_2_OFF,1);
			//Temporary
			//Switch off preheat
			HEA_Set(H3, sINACTIVE);
			//Switch zero cross detection off
			ZCR_Set (sINACTIVE);
			//Set status to WAIT
			WRK_SetSubStatus (sWAIT);
			break;
		}
		case sWAIT:
		{
			if (LED[STANDBYLED].Effect == SWITCHEDOFF)
			{
				LED_Set(STANDBYLED, BREATHING, 255, 0, 0, 50, 0, 0, 5, 0, 0); //Breathing Red
			}

			if (WRK_GetAction(MTypeSwitchOn)) //Received Switch to IDLE command
			{
				ZCR_Set (sACTIVE);
				WRK_SetMainStatus(sIDLE);
			}
			else if (WRK_GetAction(MTypeStandby)) //Received Switch to STANDBY command
			{
				WRK.MainStatus = sSTANDBY; //Not really needed, but put here to respond to UART quickly, since otherwise a time out would occur
			}
			break;
		}
		default:
			break;
	}
}
//-----------------------------------------------------------------------------
//! \brief      Handles the tick time counter
//! \details    Handles 1 ms time interrupt
//! \param      None
void WRK_HandleTickTime (void)
{
	static uint16_t TickTime;
	TickTime ++;

	if (!Initialized) return;

	if (TickTime >= 999)
	{
		AM_UpdateValues();
		//WaterTankVolume = WAT_GetVolume();
		//ANA_GetSensorValue(ADC_PS3);
		TickTime = 0;
	}
	AM_HandleSensor();
	VAL_Handle();
	SER_Handle();
	LED_Handle();
	WRK_HandleSequence();
	WRK_HandleAction();
	ZCR_Handle();
	PUM_Handle();
	BTN_Handle();
	HEA_Handle();
	HEA_HandleEfast();
	WAN_Handle();
}
//-----------------------------------------------------------------------------
//! \brief      Handles the UNDEFINED status of sequence of the coffee machine
//! \details    Handles the UNDEFINED status of sequence of the coffee machine
//! \param      None
void WRK_HandleUndefined (void)
{
	//All LED off
	LED_Set(OUTLETLED, SWITCHEDOFF, 0, 0, 0, 0, 0, 0, 0, 0, 0); //Switched off)
	LED_Set(WATERTANKLEDS, SWITCHEDOFF, 0, 0, 0, 0, 0, 0, 0, 0, 0); //Switched off)
	LED_Set(STANDBYLED, SWITCHEDOFF, 0, 0, 0, 0, 0, 0, 0, 0, 0); //Switched off)
	//All valves off
	VAL_SetCircuit (CIRCUIT_1_2_OFF,1);
	//Set status to STANDBY
	WRK_SetMainStatus(sSTANDBY);
}

//-----------------------------------------------------------------------------
//! \brief      Handles the actions needed at zerocross
//! \details    Set TIM9, Efast and zerocross signals
//! \params		None
void WRK_HandleZeroCross(void)
{
	ZCR_SetZeroCross(TIM7->CNT);
	HAL_TIM_OC_Stop_IT(&htim9, TIM_CHANNEL_1);
	HAL_TIM_OC_Stop_IT(&htim9, TIM_CHANNEL_2);
	//Reset counter
	TIM9->CNT = 0;
	if (PUM[P1].ControlType == 0)
	{
		PUM_SetPumpGPIO (P1, sON);
		TIM9->CCR1 = 0;
	}
	else
	{
		TIM9->CCR1 = 100 * (100 - PUM[P1].Percentage);
		HAL_TIM_OC_Start_IT(&htim9, TIM_CHANNEL_1);
	}
	if (PUM[P2].ControlType == 0)
	{
		PUM_SetPumpGPIO (P2, sON);
		TIM9->CCR2 = 0;
	}
	else
	{
		TIM9->CCR2 = 100 * (100 - PUM[P2].Percentage);
		HAL_TIM_OC_Start_IT(&htim9, TIM_CHANNEL_2);
	}
	HEA_ResetEfast();
}

//-----------------------------------------------------------------------------
//! \brief      Initalizes the work module
//! \details    Initalizes the work module
//! \params		None
void WRK_Init (void)
{
	if ((WRK_GetHardwareVersion() < HARDWAREVERSIONMINIMUM) && (!DUMMYMODE))
	{
		LED_Set(STANDBYLED, BLINKING, 255, 0, 0, 255, 100, 100, 0, 0, 0);
		LED_Set(OUTLETLED, BLINKING, 255, 0, 0, 255, 100, 100, 0, 0, 0);
		LED_Set(WATERTANKLEDS, BLINKING, 255, 0, 0, 255, 100, 100, 0, 0, 0);
		WRK_SetMainStatus(sHARDWAREINCOMPATIBLE);
	}
	DUMMYMODE = EEP_ReadEEPROM (WRK_DUMMYMODE);
	DUMMYMODEFILTER = EEP_ReadEEPROM (WRK_DUMMYMODEFILTER);
	DUMMYMODEH3 = EEP_ReadEEPROM (WRK_DUMMYMODEH3);
}
//-----------------------------------------------------------------------------
//! \brief      Routine to reset a pending action
//! \details    Resets the action and counter
//! \params 	None
void WRK_ResetAction (void)
{
	gAction = 0;
	gActionAccepted = 0;
	WRK.CounterAction = 0;
}
//-----------------------------------------------------------------------------
//! \brief      Handles the reset of all modules after recipe is finished
//! \details    Makes sure that no module is still active unexpectedly
//! \param[in]  uint8_t newRecipe
void WRK_ResetAllModules(uint8_t newRecipe)
{
	if (gRecipe[newRecipe].UsingH1 == 1) HEA_Set(H1, sINACTIVE);
	gRecipe[newRecipe].UsingH1 = 0;
	if (gRecipe[newRecipe].UsingH2 == 1) HEA_Set(H2, sINACTIVE);
	gRecipe[newRecipe].UsingH2 = 0;
	if (gRecipe[newRecipe].UsingH3 == 1) HEA_Set(H3, sINACTIVE);
	gRecipe[newRecipe].UsingH3 = 0;
	if (gRecipe[newRecipe].UsingP1 == 1) PUM_Set(P1, sINACTIVE);
	gRecipe[newRecipe].UsingP1 = 0;
	if (gRecipe[newRecipe].UsingP2 == 1) PUM_Set(P2, sINACTIVE);
	gRecipe[newRecipe].UsingP2 = 0;
	if (gRecipe[newRecipe].UsingP3 == 1) PUM_Set(P3, sINACTIVE);
	gRecipe[newRecipe].UsingP3 = 0;
	gRecipe[newRecipe].UsingS3 = 0;
	if (gRecipe[newRecipe].UsingCircuit1 == 1) VAL_SetCircuit(CIRCUIT_1_OFF, 1 - gRecipe[1 - newRecipe].UsingS3);
	gRecipe[newRecipe].UsingCircuit1 = 0;
	if (gRecipe[newRecipe].UsingCircuit2 == 1) VAL_SetCircuit(CIRCUIT_2_OFF, 1 - gRecipe[1 - newRecipe].UsingS3);
	gRecipe[newRecipe].UsingCircuit2 = 0;
	if (gRecipe[1 - newRecipe].UsingP3 == 0) AIR_Set(0);
	REC_Set(newRecipe, sINACTIVE);
	//gRecipe[newRecipe].Active = 0;
	//gRecipe[newRecipe].CurrentBlock = 0;
}


//-----------------------------------------------------------------------------
//! \brief      Routine to set an action to be executed by the work module
//! \details    Sets the action and resets the counter. Checks if a command can be executed
//! \params 	None
void WRK_SetAction (uint8_t newAction)
{
	//check commands validity before try executing
	if ((newAction == MTypeStartRecipe1) && (gRecipe[RECIPE1].Valid == 0)) //Start recipe, but no recipe loaded
	{
		SER_SetStatus (MStatNoValidRecipe); //No valid recipe loaded
	}
	else if ((newAction == MTypeStartRecipe2) &&  (gRecipe[RECIPE2].Valid == 0)) //Start recipe, but no recipe loaded
	{
		SER_SetStatus (MStatNoValidRecipe); //No valid recipe loaded
	}
	else if ((newAction == MTypeStartRecipe3) &&  (gRecipe[RECIPE3].Valid == 0)) //Start recipe, but no recipe loaded
	{
		SER_SetStatus (MStatNoValidRecipe); //No valid recipe loaded
	}
	else if ((newAction == MTypeStopRecipe1) && (gRecipe[RECIPE1].Active == 0)) //Stop recipe, but recipe is not active
	{
		SER_SetStatus (MStatRecipeNotActive); //No valid recipe loaded
	}
	else if ((newAction == MTypeStopRecipe2) && (gRecipe[RECIPE2].Active == 0)) //Stop recipe, but recipe is not active
	{
		SER_SetStatus (MStatRecipeNotActive); //No valid recipe loaded
	}
	else if ((newAction == MTypeStopRecipe3) && (gRecipe[RECIPE3].Active == 0)) //Stop recipe, but recipe is not active
	{
		SER_SetStatus (MStatRecipeNotActive); //No valid recipe loaded
	}
	else if (((newAction == MTypeStartRecipe1) || (newAction == MTypeStartRecipe2) || (newAction == MTypeStartRecipe3))  && ((ZCR.Status == sUNITERROR) || (ZCR.Status == sINACTIVE)) && (!DUMMYMODE)) //Start recipe, but no zerocross detected
	{
		SER_SetStatus (MStatNoZeroCross); //No zero cross detected. Cannot start heaters or pumps
	}
	else //Commands can be executed. Try executing
	{
		gAction = newAction;
		gActionAccepted = 0;
		WRK.CounterAction = 0;
	}

}

//-----------------------------------------------------------------------------
//! \brief      Set selected dummy mode
//! \details    Used to set the dummymode and store variables in EEprom
//! \param[in]	uint8_t newAddress
//! \Param[in]	uint8_t newValue
void WRK_SetDummyMode (uint8_t newAddress, uint8_t newValue)
{
	switch (newAddress)
	{
		case WRK_DUMMYMODE:
		{
			DUMMYMODE = newValue;
			break;
		}
		case WRK_DUMMYMODEFILTER:
		{
			DUMMYMODEFILTER = newValue;
			break;
		}
		case WRK_DUMMYMODEH3:
		{
			DUMMYMODEH3 = newValue;
			break;
		}
		default: break;
	}
	EEP_WriteEEPROM (newAddress, newValue);
}
//-----------------------------------------------------------------------------
//! \brief      Handles the main status setting
//! \details    Set the status and saves to old status. Sets the sub status to UNDEFINED
//! \param[in]  enuStatus newStatus
void WRK_SetMainStatus (enuStatus newStatus)
{
	WRK.MainStatusOld = WRK.MainStatus;
	WRK.MainStatus = newStatus;
	WRK.SubStatusOld = WRK.SubStatus;
	WRK.SubStatus = sUNDEFINED;
}

//-----------------------------------------------------------------------------
//! \brief      Handles the sub status setting
//! \details    Set the status and saves to old status.
//! \param      Non
void WRK_SetSubStatus (enuStatus newStatus)
{
	WRK.SubStatusOld = WRK.SubStatus;
	WRK.SubStatus = newStatus;
}

//---------------- End of file -----------------------
