//-----------------------------------------------------------------------------
//! \file       HEA.c
//! \author     R. Weimar. RMB Holland bv
//! \brief      Contains routines for the general sequence
//! \details
//! \Attention
//! \Created on: May 5, 2022
//-----------------------------------------------------------------------------
#include "heaters.h"
#include "analog.h"
#include "recipe.h"

//! \Global bit values of heater percentage
uint16_t PhaseList[] = {0,512,528,584,585,682,693,731,887,1022,1023};

//! \Global counter for the bit evaluation
uint8_t PhaseCounter;
//! \Global heater structure
stcHeater HEA[3];
//! \H3 conversion factor
uint8_t H3_Factor = 25;
//! \PID values
float SetHeaterP = 100.0;
float SetHeaterI = 4.5;
float SetHeaterD = 1000.0;
float SetHeater3P = 1.0;
float SetHeater3I = 0.0;
float SetHeater3D = 0.0;
uint8_t MaxDeltaT = 2;

//-----------------------------------------------------------------------------
//! \brief      Gets the temperature of the selected heater
//! \details    Gets the temperature of the selected heater
//! \param[in]  uint8_t newHeater		Heater number
//! \param[out] uint32_t Temperature
uint32_t HEA_GetHeaterTemperature (uint8_t newHeater)
{
	return HEA[newHeater].GetTemperature;
}
//-----------------------------------------------------------------------------
//! \brief      Handles EfastCounters
//! \details	Calculates the Efast if a measurement was active
//! \params		None
void HEA_HandleEfast(void)
{
	for (uint8_t i = 0; i < 2; i++)
	{
		//HEA[i].EfastCounterOld = 0;
	}
}
//-----------------------------------------------------------------------------
//! \brief      Resets EfastCounters
//! \details
//! \params		None
void HEA_ResetEfast(void)
{
	for (uint8_t i = 0; i < 2; i++)
	{
		if (HEA[i].EfastCounter > HEA[i].EfastCounterMax)
		{
			HEA[i].EfastCounterMax = HEA[i].EfastCounter;
		}
		HEA[i].EfastCounterOld = HEA[i].EfastCounter;
		if (HEA[i].EfastActive == 1)
		{
			HEA[i].EfastActive = 0;
			HEA[i].EfastValidValue = HEA[i].EfastCounter - HEA[i].EfastOffset;
		}
		HEA[i].EfastCounter = 0;
	}
}
//-----------------------------------------------------------------------------
//! \brief        Increases the EFast counter of the selected Heater
//! \details
//! \param[in]    uint8_t newHeater H1 or H2
void HEA_IncEfast (uint8_t newHeater)
{
	if (newHeater > H2) return;

	HEA[newHeater].EfastCounter ++;
}
//-----------------------------------------------------------------------------
//! \brief        Sets the control value of the selected Heater
//! \details      Sets the power value from the PID
//! \param[in]    uint8_t newHeater (0 - 2)
//! \param[in]    float newPower (0 - 1200)
uint8_t HEA_SetControl (uint8_t newHeater, float newPower)
{
	uint8_t ReturnValue = TASK_ERROR;
	if (newHeater < H3)
		if (newPower == FTH_E_POWER)
			HEA[newHeater].Power = FTH_E_POWER;
		else
			HEA[newHeater].Power = (uint16_t) (newPower/8.33333);//Max 1200 W

	else
		HEA[newHeater].Power = (uint16_t) (newPower/H3_Factor);     //Max 400 W
	ReturnValue = TASK_READY;
	return ReturnValue;
}
//-----------------------------------------------------------------------------
//! \brief        Handles the heaters H1 and H2 and H3
//! \details      Starts the temperature measurement and the PID calculation every 100 ms
//! \params       None
void HEA_Handle (void)
{
	static uint16_t TickTimer=7;
	if (TickTimer ++ < 49) return; //50 ms interval
	TickTimer = 0;
	HEA_HandlePID();
}

//-----------------------------------------------------------------------------
//! \brief        Handles the PID of the heaters based on temperature
//! \details      Calculates the PID for the heaters based on the required and measured temperature
//! \params       None
void HEA_HandlePID (void)
{
	for (uint8_t i = 0; i < 3; i++)
	{
		if (i == 0) //FTH 1
		{
			if (!DUMMYMODE) HEA[i].GetTemperature = ANA_GetSensorValue(ADC_NTC1);
		}
		else if (i == 1) //FTH 2
		{
			if (!DUMMYMODE) HEA[i].GetTemperature = ANA_GetSensorValue(ADC_NTC2);
		}
		else if (i == 2) //Brewer heater
		{
			if ((!DUMMYMODE) && (!DUMMYMODEH3)) HEA[i].GetTemperature = ANA_GetSensorValue(ADC_NTC3);
		}
		HEA[i].DeltaTemperature = HEA[i].GetTemperature - HEA[i].GetTemperatureOld;
		if (HEA[i].SetTemperature > 0)
			HEA[i].TemperatureDeviation = HEA[i].SetTemperature - HEA[i].GetTemperature;
		else
			HEA[i].TemperatureDeviation = 0;
		if ((HEA[i].Status == sINACTIVE) || (HEA[i].SetTemperature == 0))
		{
			HEA[i].Status = sINACTIVE;
			HEA_SetControl (i,0);
			HEA[i].ErrorP_Heater = 0;
			HEA[i].ErrorP_Heater_Old = 0;
			HEA[i].ErrorD_Heater = 0;
			HEA[i].ErrorI_Heater = 0;
			HEA[i].PID_Heater = 0;
			HEA[i].HeaterControl = 0;
			HEA[i].HeaterControlOld = 0;
			if (!DUMMYMODE)
			{
				HEA[i].SetTemperature = 0;
				HEA[i].GetTemperatureOld = 0;
				HEA[i].DeltaTemperature = 0;
			}

		}
		else
		{
			HEA[i].ErrorP_Heater= (float)(HEA[i].SetTemperature) - (float)(HEA[i].GetTemperature); //get error and bring into control range
			HEA[i].ErrorI_Heater= HEA[i].ErrorI_Heater + (HEA[i].ErrorP_Heater);//bring into control range
			HEA[i].ErrorD_Heater= (HEA[i].ErrorP_Heater - HEA[i].ErrorP_Heater_Old); //bring into control range
			HEA[i].ErrorP_Heater_Old = HEA[i].ErrorP_Heater;
			if (i<H3)
			{
				HEA[i].P_Heater = SetHeaterP;
				HEA[i].I_Heater = SetHeaterI;
				HEA[i].D_Heater = SetHeaterD;
			}
			else
			{
				HEA[i].P_Heater = SetHeater3P;
				HEA[i].I_Heater = SetHeater3I;
				HEA[i].D_Heater = SetHeater3D;
			}
			HEA[i].PID_Heater = (HEA[i].ErrorP_Heater * HEA[i].P_Heater); // Calculate P;
			HEA[i].PID_Heater = HEA[i].PID_Heater + ((HEA[i].ErrorI_Heater * HEA[i].I_Heater)); //Calculate I and add to P
			HEA[i].PID_Heater = HEA[i].PID_Heater + (((HEA[i].ErrorD_Heater * HEA[i].D_Heater))); // Calculate D and add to PI
			if (HEA[i].PID_Heater < 0)
			{
				HEA[i].HeaterControl = 0;
			}
			else
			{
				HEA[i].HeaterControl = HEA[i].PID_Heater;// + (HeaterPreset * PresetScale);
			}
			if (HEA[i].HeaterControl < 10)
			{
				HEA[i].HeaterControl = 0; //To avoid selecting FTH_E_POWER which is 8.3333 W
			}
			if (HEA[i].DeltaTemperature > MaxDeltaT)
			{
				HEA[i].HeaterControl = 0;
			}
			if (HEA[i].HeaterControl > 10000)
			{
				HEA[i].HeaterControl = 10000;
			}
			if (HEA[i].TemperatureDeviation > 50)
				HEA[i].HeaterControl = 10000;

			//Cut off heater if temperature already at setpoint and still rising
			if ((HEA[i].GetTemperature >= HEA[i].SetTemperature) && (HEA[i].DeltaTemperature > 0))
				HEA[i].HeaterControl = 0;
			//Switch on heater if temperature already under setpoint and falling
			if ((HEA[i].GetTemperature <= HEA[i].SetTemperature) && (HEA[i].DeltaTemperature < 0) && (HEA[i].HeaterControl < 7500))
				HEA[i].HeaterControl = 10000;

			if (HEA[i].EfastValidValue > MAXEFAST) //&& (HEA[i].GetTemperature > 1200)) //Protect heater against overheating (In case of steam)
			{
				HEA[i].HeaterControl = FTH_E_POWER; //Special mode for Efast measuring at 120W
			}
			else if (HEA[i].EfastValidValue > CUTOFFEFAST)
			{
				HEA[i].HeaterControl = FTH_E_POWER; //Special mode for Efast measuring at 120W
			}
			if (i == H3)
			{
				if (HEA[i].GetTemperature > HEA[i].SetTemperature)
					HEA[i].HeaterControl = 0;
			}
			HEA[i].HeaterControlOld = HEA[i].HeaterControl;
			HEA[i].GetTemperatureOld = HEA[i].GetTemperature;
			HEA_SetControl (i, (uint16_t) (HEA[i].HeaterControl));
		}
	}
}
//-----------------------------------------------------------------------------
//! \brief      Changes the status of the selected heater
//! \details    Sets the selected heater status to the requested status
//! \param[in]  uint8_t newHeater		Heater number
//! \param[in]  enuStatus newStatus		INACTIVE or ACTIVE
void HEA_Set (uint8_t newHeater, enuStatus newStatus)
{
	if (DUMMYMODE) return;
	HEA[newHeater].Status = newStatus;
}

//-----------------------------------------------------------------------------
//! \brief      Sets the temperature of the selected heater
//! \details    Sets the temperature of the selected heater
//! \param[in]  uint8_t newHeater		Heater number
//! \param[in]  uint16_t newTemperature
//! \param[in]  uint16_t newSteamMode
void HEA_SetHeaterTemperature (uint8_t newHeater, uint16_t newTemperature, uint8_t newSteamMode)
{
	HEA[newHeater].SetTemperature = newTemperature;
	HEA[newHeater].SteamMode = newSteamMode;
}
//-----------------------------------------------------------------------------
//! \brief      Sets the temperature of the selected heater
//! \details    Sets the temperature of the selected heater
//! \param[in]  uint8_t newHeater		Heater number H1, H2, H3
void HEA_SetHeaterGPIO (uint8_t newHeater)
{
	uint16_t x1, x2;
	if (DUMMYMODE) return;
	if (newHeater < H3) //Heater 1 or Heater 2
	{
		//Make sure power never over 100%
		if (HEA[newHeater].Power > FTH_L_POWER + FTH_H_POWER) HEA[newHeater].Power = FTH_L_POWER + FTH_H_POWER;

		x1 = PhaseCounter;
		//Make sure heaters are switched off if power = 0
		if ((HEA[newHeater].Power == 0) || (HEA[newHeater].Status == sINACTIVE))
		{
			if (newHeater == H1)
			{
				H1L_OFF();
				H1H_OFF();
			}
			else
			{
				H2L_OFF();
				H2H_OFF();
			}
		}
		else if (HEA[newHeater].Power == FTH_E_POWER)//Use both heaters
		{
			x2 = 528; //Corresponds with 1 phase on 9 phases off
			if ((1<<(x1)) & (x2)) //Both heater tracks on
			{
				HEA[newHeater].EfastActive = 1;
				if (newHeater == H1)
				{
					H1L_ON();
					H1H_ON();
				}
				else
				{
					H2H_ON();
					H2L_ON();
				}
			}
			else
			{
				//HEA[newHeater].EfastActive = 0;
				if (newHeater == H1)
				{
					H1L_OFF();
					H1H_OFF();
				}
				else
				{
					H2L_OFF();
					H2H_OFF();
				}
			}
		}
		else if (HEA[newHeater].Power > FTH_H_POWER) //Use both HEA. High power part is 100% on
		{
			if (newHeater == H1) H1H_ON(); else H2H_ON();
			x2 = PhaseList[((HEA[newHeater].Power-FTH_H_POWER)*10/FTH_L_POWER)];
			if ((1<<(x1)) & (x2)) //This phase is on
			{
				HEA[newHeater].EfastActive = 1;
				if (newHeater == H1) H1L_ON(); else H2L_ON();
			}
			else
			{
				//HEA[newHeater].EfastActive = 0;
				if (newHeater == H1) H1L_OFF(); else H2L_OFF();
			}
		}
		else if (HEA[newHeater].Power <= FTH_L_POWER) //Only use Low power part
		{
			if (newHeater == 0) H1H_OFF(); else H2H_OFF();
			x2 = PhaseList[(HEA[newHeater].Power*10/FTH_L_POWER)];
			if ((1<<(x1)) & (x2)) //This phase is on
			{
				HEA[newHeater].EfastActive = 1;
				if (newHeater == 0) H1L_ON(); else H2L_ON();
			}
			else
			{
				//HEA[newHeater].EfastActive = 0;
				if (newHeater == 0) H1L_OFF(); else H2L_OFF();
			}
		}
		else if (HEA[newHeater].Power <= FTH_H_POWER) //Only use High power part
		{
			if (newHeater == 0) H1L_OFF(); else H2L_OFF();
			x2 = PhaseList[(HEA[newHeater].Power*10/FTH_H_POWER)];
			if ((1<<(x1)) & (x2)) //This phase is on
			{
				HEA[newHeater].EfastActive = 1;
				if (newHeater == 0) H1H_ON(); else H2H_ON();
			}
			else
			{
				//HEA[newHeater].EfastActive = 0;
				if (newHeater == 0) H1H_OFF(); else H2H_OFF();
			}
		}
	}
	else if ((newHeater == H3)&&(!DUMMYMODEH3))
	{
		x1 = PhaseCounter;
		if (HEA[newHeater].Power > BH_POWER) HEA[newHeater].Power = BH_POWER;
		x2 = PhaseList[(HEA[newHeater].Power*10/BH_POWER)];
		//Make sure heater is switched off if power = 0
		if ((HEA[newHeater].Power == 0) || (HEA[newHeater].Status == sINACTIVE))
		{
			H3_OFF();
		}
		else
		{
			if ((1<<(x1)) & (x2)) //This phase is on
			{
				H3_ON();
			}
			else
			{
				H3_OFF();
			}
		}
	}
}



