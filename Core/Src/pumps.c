//-----------------------------------------------------------------------------
//! \file       pumps.c
//! \author     R. Weimar. RMB Holland bv
//! \brief      Contains routines for the pump module
//! \details
//! \Attention
//! \Created on: June 12, 2022
//-----------------------------------------------------------------------------
#include "pumps.h"
#include "work.h"
#include "analog.h"
#include "stdlib.h"
#include "heaters.h"
#include "recipe.h"

//-----------------------------------------------------------------------------
//! \Global Flow meter pulse flags
uint8_t FlgFM[2];
//! \Global Pump structure
stcPump PUM[2];
//! \Global counter for the bit evaluation
uint8_t PhaseCounterPumps;
//! \Global bit values of heater percentage
uint16_t PhaseListPumps[] = {0,512,768,896,960,992,1008,1016,1020,1022,1023};
uint16_t PUM_Offset = 4000;
//! \PID values
float SetP = 0.25;
float SetI = 0.1;
float SetD = 0.0;
float SetPressureP = 0.25;
float SetPressureI = 0.25;
float SetPressureD = 0.0;

//-----------------------------------------------------------------------------
//! \brief      Gets a machine parameter related to the pump module
//! \details    Gets a parameter
//! \param[in]	uint8_t newParameter   (PUM_ULPERPULSEP1 or PUM_ULPERPULSEP2)
//! \param[out]	uint16_t newValue
uint16_t PUM_GetParameter (uint8_t newParameter)
{
	uint16_t ReturnValue = 0;
	switch (newParameter)
	{
		case PUM_ULPERPULSEP1:
		{
			ReturnValue = PUM[P1].UlPerPulse;
			break;
		}
		case PUM_ULPERPULSEP2:
		{
			ReturnValue = PUM[P2].UlPerPulse;
			break;
		}
		default:
			break;
	}
	return ReturnValue;
}
//-----------------------------------------------------------------------------
//! \brief      Sets a machine parameter related to the pump module
//! \details    Sets a parameter and stores it in EEprom
//! \param[in]	uint8_t newParameter   (PUM_ULPERPULSEP1 or PUM_ULPERPULSEP2)
//! \param[in]	uint16_t newValue
void PUM_SetParameter (uint8_t newParameter, uint16_t newValue)
{
	switch (newParameter)
	{
		case PUM_ULPERPULSEP1:
		{
			PUM[P1].UlPerPulse = newValue;
			EEP_WriteEEPROM(newParameter, newValue);
			break;
		}
		case PUM_ULPERPULSEP2:
		{
			PUM[P2].UlPerPulse = newValue;
			EEP_WriteEEPROM(newParameter, newValue);
			break;
		}
		default:
			break;
	}
}
//-----------------------------------------------------------------------------
//! \brief      Calculates the volume dispensed
//! \details    Calculates the volume and sets the pump inactive when the volume is reached
//! \param[in]	uint8_t newPump (P1, P2)
void PUM_CalculateVolume (uint8_t newPump)
{
	if (newPump < P3)
	{
		if (PUM[newPump].Status == sACTIVE)
		{
			PUM[newPump].GetVolume += PUM[newPump].UlPerPulse;
			//Check if the pump is used in the active recipe and add the volume
			for (uint8_t i = 0; i<2;i++)
			{
				if (gRecipe[i].Active)
				{
					if ((gRecipe[i].UsingCircuit1 == 1) && (newPump == P1))
						gRecipe[i].GetTotalVolume += PUM[newPump].UlPerPulse;
					if ((gRecipe[i].UsingCircuit2 == 1) && (newPump == P2))
						gRecipe[i].GetTotalVolume += PUM[newPump].UlPerPulse;
				}
			}
		}
		if (PUM[newPump].GetVolume >= PUM[newPump].SetVolume)
		{
			PUM[newPump].Status = sINACTIVE;
		}
	}
}
//-----------------------------------------------------------------------------
//! \brief        Handles the pumps P1 and P2
//! \details      Starts the flow measurement and the PID calculation every 100 ms
//! \params       None
void PUM_Handle (void)
{
	static uint16_t TickTimer=50;

	PUM_HandlePressureSensors();

	if (TickTimer ++ < 99) return; //100ms interval
	TickTimer = 0;

	PUM_HandleFlowMeters();
	PUM_HandlePIDFlow();
	PUM_HandlePIDPressure();
}
uint16_t Pulses;
uint16_t PulsesOld;
uint16_t PulsesTime;
uint16_t PulsesTimeOld;
//-----------------------------------------------------------------------------
//! \brief      Calculates the time based flow
//! \details    Calculates the flow based on pulse count and pulse time
//! \param[in]	uint8_t newPump (P1, P2)
void PUM_CalculateFlow(uint8_t newPump)
{
	//uint16_t Pulses;

	Pulses = PUM[newPump].PulseTime - PUM[newPump].PulseTimeOld;
	PulsesOld = Pulses;
	PUM[newPump].PulseTimeOld = PUM[newPump].PulseTime;
	if (Pulses > 0)
	{
		//PUM[newPump].Status = sON;
		PUM[newPump].GetFlowTimeBased = PUM[newPump].UlPerPulse * 10000 / Pulses ; //ul/s //Timer is 10kHz, so pulses of 100us
	}
	else
	{
		//PUM[newPump].Status = sOFF;
		PUM[newPump].GetFlowTimeBased = 0;
	}
}
uint16_t PS1Values[100];
uint16_t PS2Values[100];
//-----------------------------------------------------------------------------
//! \brief      Gets the pressure
//! \details    Retrieves the pressure
//! \params		None
void PUM_HandlePressureSensors (void)
{
	static uint8_t Counter = 0;
	//static uint16_t PS1Values[10];
	//static uint16_t PS2Values[10];
	uint64_t PS1Value = 0, PS2Value = 0;
	if (Counter >= 100) Counter = 0;
	PS1Values[Counter] = ANA_GetSensorValue(ADC_PS1);
	PS2Values[Counter] = ANA_GetSensorValue(ADC_PS2);
	Counter ++;
	for (uint8_t i = 0; i<100 ; i ++)
	{
		PS1Value += PS1Values[i];
		PS2Value += PS2Values[i];
	}
	if (DUMMYMODE) return; //Pressure is simulated in dummy mode
	PUM[P1].GetPressure = (uint32_t) (PS1Value / 100);
	PUM[P2].GetPressure = (uint32_t) (PS2Value / 100);
}
//-----------------------------------------------------------------------------
//! \brief      Calculates the flow
//! \details    Calculates the flow based on pulse count per second. Needed to detect no flow
//! \params		None
void PUM_HandleFlowMeters (void)
{
	static uint8_t TickTime;
	static uint32_t NoFlowCounter[2];
	if (DUMMYMODE) return; //Flow is simulated in dummy mode

	TickTime ++;
	if (TickTime < 9) return;
	TickTime = 0;

	for (uint8_t i = P1; i <= P2; i++)
	{
		PUM[i].GetFlow = PUM[i].Pulses * PUM[i].UlPerPulse; //Ul/s x 60 = Ul / min
		PUM[i].Pulses = 0;
		if ((PUM[i].GetFlow == 0) && (PUM[i].PulseTimeOld == PUM[i].PulseTime))
		{
			//PUM[i].Status = sOFF;
			PUM[i].GetFlowTimeBased = 0;
		}
		if (PUM[i].GetFlowTimeBased == 0)
		{
			if (NoFlowCounter[i] < 0xffff) NoFlowCounter[i] ++;
			if (NoFlowCounter[i] > NOFLOWCUTOFFTIME * 10)
			{
				PUM[i].Status = sUNITERROR;
			}
		}
		else
		{
			NoFlowCounter[i] = 0;
		}
	}


}
//-----------------------------------------------------------------------------
//! \brief        Handles the PID of the pumps based on pressure
//! \details      Calculates the PID for the pumps based on the required and measured pressure
//! \params       None
void PUM_HandlePIDPressure (void)
{
	for (uint8_t i = 0; i < 2; i++)
	{
		if ((PUM[i].SetControlType == PUM_PRESSURECONTROL) || (PUM[i].SetControlType == PUM_BOTHCONTROL)) //Only use if pressure control
		{
			if (PUM[i].GetPressure < PUM[i].SetPressure)
			{
				if ((PUM[i].SetControlType == PUM_PRESSURECONTROL) || (PUM[i].SetControlType == PUM_BOTHCONTROL))
				{
					PUM[i].RegulateFlow = 0;
					PUM[i].RegulatePressure = 1;
				}
			}
			if ((PUM[i].Status == sINACTIVE) || (PUM[i].Status == sUNITERROR) || (PUM[i].SetPressure == 0) || ((PUM[i].RegulatePressure == 0) && (PUM[i].RegulateFlow == 0)))
			{
				PUM_SetControl (i,0);
				PUM[i].ErrorP_Pump = 0;
				PUM[i].ErrorP_Pump_Old = 0;
				PUM[i].ErrorD_Pump = 0;
				PUM[i].ErrorI_Pump = 0;
				PUM[i].PID_Pump = 0;
				PUM[i].PumpControl = 0;
				PUM[i].PumpControlOld = 0;
				PUM[i].Percentage = 0;
			}
			else if (PUM[i].RegulatePressure == 1)
			{
				PUM[i].ErrorP_Pump= (float)(PUM[i].SetPressure) - (float)(PUM[i].GetPressure); //get error and bring into control range
				PUM[i].ErrorI_Pump= PUM[i].ErrorI_Pump + (PUM[i].ErrorP_Pump);//bring into control range
				PUM[i].ErrorD_Pump= (PUM[i].ErrorP_Pump - PUM[i].ErrorP_Pump_Old); //bring into control range
				PUM[i].ErrorP_Pump_Old = PUM[i].ErrorP_Pump;
				PUM[i].P_Pump = SetPressureP; //100.0;
				PUM[i].I_Pump = SetPressureI; //0.5;
				PUM[i].D_Pump = SetPressureD; //0.5;
				PUM[i].PID_Pump = PUM[i].ErrorP_Pump * PUM[i].P_Pump; // Calculate P
				PUM[i].PID_Pump = PUM[i].PID_Pump + ((PUM[i].ErrorI_Pump * PUM[i].I_Pump)); //Calculate I and add to P
				PUM[i].PID_Pump = PUM[i].PID_Pump + (((PUM[i].ErrorD_Pump * PUM[i].D_Pump))); // Calculate D and add to PI
				PUM[i].PumpControl = PUM[i].PID_Pump;// + (PumpPreset * PresetScale);
				if (PUM[i].PID_Pump < 0)
				{
					PUM[i].PumpControl = 0;
				}
				else
				{
					PUM[i].PumpControl = PUM[i].PID_Pump;
				}
				if (PUM[i].PumpControlOld == 0)
				{
					PUM[i].PumpControlOld = PUM[i].PumpControl;
				}
				if (PUM[i].PumpControl > 10000)
				{
					PUM[i].PumpControl = 10000;
				}
				PUM[i].GetVolumeOld = PUM[i].GetVolume;
				PUM_SetControl (i, PUM[i].PumpControl/100);
			}
		}
	}
}

//-----------------------------------------------------------------------------
//! \brief        Handles the PID of the pumps based on flow
//! \details      Calculates the PID for the pumps based on the required and measured flow
//! \params       None
void PUM_HandlePIDFlow (void)
{
	uint32_t GetFlow;
	for (uint8_t i = 0; i < 2; i++)
	{
		if ((PUM[i].SetControlType == PUM_FLOWCONTROL) || (PUM[i].SetControlType == PUM_BOTHCONTROL)) //Only use if flow control
		{
			if (PUM[i].GetFlow < PUM[i].SetFlow)
			{
				if ((PUM[i].SetControlType == PUM_FLOWCONTROL) || (PUM[i].SetControlType == PUM_BOTHCONTROL))
				{
					PUM[i].RegulateFlow = 1;
					PUM[i].RegulatePressure = 0;
				}
			}
			if ((PUM[i].Status == sINACTIVE) || (PUM[i].Status == sUNITERROR) || (PUM[i].SetFlow == 0) || ((PUM[i].RegulatePressure == 0) && (PUM[i].RegulateFlow == 0)))
			{
				PUM_SetControl (i,0);
				PUM[i].ErrorP_Pump = 0;
				PUM[i].ErrorP_Pump_Old = 0;
				PUM[i].ErrorD_Pump = 0;
				PUM[i].ErrorI_Pump = 0;
				PUM[i].PID_Pump = 0;
				PUM[i].PumpControl = 0;
				PUM[i].PumpControlOld = 0;
				PUM[i].Percentage = 0;
			}
			else if (PUM[i].RegulateFlow == 1)
			{
				GetFlow = PUM[i].GetFlowTimeBased;
				PUM[i].ErrorP_Pump= (float)(PUM[i].SetFlow) - (float)(GetFlow); //get error and bring into control range
				PUM[i].ErrorI_Pump= PUM[i].ErrorI_Pump + (PUM[i].ErrorP_Pump);//bring into control range
				PUM[i].ErrorD_Pump= (PUM[i].ErrorP_Pump - PUM[i].ErrorP_Pump_Old); //bring into control range
				PUM[i].ErrorP_Pump_Old = PUM[i].ErrorP_Pump;
				PUM[i].P_Pump = SetP; //100.0;
				PUM[i].I_Pump = SetI; //0.5;
				PUM[i].D_Pump = SetD; //0.5;
				PUM[i].PID_Pump = (PUM[i].ErrorP_Pump * PUM[i].P_Pump); // Calculate P;
				PUM[i].PID_Pump = PUM[i].PID_Pump + ((PUM[i].ErrorI_Pump * PUM[i].I_Pump)); //Calculate I and add to P
				PUM[i].PID_Pump = PUM[i].PID_Pump + (((PUM[i].ErrorD_Pump * PUM[i].D_Pump))); // Calculate D and add to PI
				if (PUM[i].PID_Pump < 10)
				{
					PUM[i].PumpControl = 0;
				}
				else
				{
					PUM[i].PumpControl = PUM[i].PID_Pump + PUM_Offset;
				}
				if (PUM[i].PumpControlOld == 0)
				{
					PUM[i].PumpControlOld = PUM[i].PumpControl;
				}
				if (PUM[i].PumpControl > 10000)
				{
					PUM[i].PumpControl = 10000;
				}
				PUM[i].PumpControlOld = PUM[i].PumpControl;
				PUM[i].GetVolumeOld = PUM[i].GetVolume;
				if ((PUM[i].SteamMode == 0))
				{
					PUM_SetControl (i, PUM[i].PumpControl/100);
				}
				else
				{
					if (i<2)
					{
						if ((HEA[i].GetTemperature < HEA[i].SetTemperature))//&&(HEA[i].EfastValidValue < MAXEFAST))
						{
							PUM_SetControl(i, 0);
						}
						else
						{
							if (i==0)
								PUM_SetControl(i, 40);
							else
							{
								if (PUM[i].GetFlow < 100)
									PUM_SetControl(i, 65);
								else
									PUM_SetControl(i, 50);
							}
						}
						if ((HEA[i].EfastValidValue) > MAXEFAST)
						{
							if (i==0)
								PUM_SetControl(i, 40);
							else
							{
								if (PUM[i].GetFlow < 100)
									PUM_SetControl(i, 65);
								else
									PUM_SetControl(i, 50);
							}
						}
					}
				}
			}
		}
	}
}
//-----------------------------------------------------------------------------
//! \brief        Initializes the pumps module
//! \details      Sets parameters for the PID control
//! \params       None
void PUM_Init(void)
{
	PUM[P1].UlPerPulse = EEP_ReadEEPROM(PUM_ULPERPULSEP1);
	if (PUM[P1].UlPerPulse == 0)
	{
		PUM[P1].UlPerPulse = PUM_ULPERPULSE;
		EEP_WriteEEPROM(PUM_ULPERPULSEP1, PUM_ULPERPULSE);
	}
	PUM[P2].UlPerPulse = EEP_ReadEEPROM(PUM_ULPERPULSEP2);
	if (PUM[P2].UlPerPulse == 0)
	{
		PUM[P2].UlPerPulse = PUM_ULPERPULSE;
		EEP_WriteEEPROM(PUM_ULPERPULSEP2, PUM_ULPERPULSE);
	}
	//Set control type to phase cutting
	PUM[P1].ControlType = 1;
	PUM[P2].ControlType = 1;
}
uint8_t PMax;
//-----------------------------------------------------------------------------
//! \brief        Sets the control value of the selected pump
//! \details      Sets the phase cutting control value from the PID
//! \param[in]    uint8_t newPump (0 - 1)
//! \param[in]    uint8_t newPercentage (0 - 100)
uint8_t PUM_SetControl (uint8_t newPump, uint8_t newPercentage)
{
	uint8_t ReturnValue = TASK_READY;
	PUM[newPump].Percentage = newPercentage;
	if (PUM[P2].Percentage > PMax) PMax = PUM[P2].Percentage;
	if ((newPercentage >= 20) && (newPercentage <= 100) && (newPump < 2))
	{
		PUM[newPump].Percentage = newPercentage;
		ReturnValue = TASK_READY;
	}
	else if ((newPercentage > 0)&&(newPercentage <= 40)&&(newPump==0))
	{
		PUM[newPump].Percentage = 30;//newPercentage = 40;
	}
	else if ((newPercentage > 0)&&(newPercentage <= 65)&&(newPump==1)&&(PUM[newPump].GetFlow < 100))
	{
		PUM[newPump].Percentage = 65;//newPercentage = 40;
	}
	else if ((newPercentage > 0)&&(newPercentage <= 50)&&(newPump==1)&&(PUM[newPump].GetFlow >= 100))
	{
		PUM[newPump].Percentage = 50;//newPercentage = 40;
	}
	return ReturnValue;
}

//-----------------------------------------------------------------------------
//! \brief        Gets the pressure of the selected pump
//! \details      Converts the value from mBar to 0.1 bar
//! \param[in]    uint8_t newPump (0 - 1)
uint16_t PUM_GetPressure (uint8_t newPump)
{
	{
		return PUM[newPump].GetPressure / 100; //Convert to x 0.1 bar
	}
}

//-----------------------------------------------------------------------------
//! \brief        Gets the flow of the selected pump
//! \details      Converts the value from ul/s to ml/min
//! \param[in]    uint8_t newPump (0 - 1)
uint16_t PUM_GetFlow (uint8_t newPump)
{
	{
		return PUM[newPump].GetFlowTimeBased / 100; //Convert to x 0.1 ml/min
	}
}


//-----------------------------------------------------------------------------
//! \brief        Gets the volume of the selected pump
//! \details      Converts the value from ul to ml
//! \param[in]    uint8_t newPump (0 - 1)
uint16_t PUM_GetVolume (uint8_t newPump)
{
	{
		return PUM[newPump].GetVolume/1000;
	}
}

//-----------------------------------------------------------------------------
//! \brief        Sets the status of the selected pump
//! \details      Sets the status to ACTIVE or INACTIVE
//! \param[in]    uint8_t newPump (P1 - P2)
//! \param[in]    enuStatus newStatus (sINACTIVE, sACTIVE)
void PUM_Set (uint8_t newPump, uint32_t newStatus)
{
	/*
	PUM_ULPERPULSE is now saved in EEPROM and set at startup
	 int Temp = (-1325.0f / (PUM[newPump].SetFlow / 100)) + 445; //Compensation for low flow
	if (newPump == P1)
		PUM[newPump].UlPerPulse = (uint32_t) Temp;
	else
		PUM[newPump].UlPerPulse = 404;
		*/
	PUM[newPump].GetVolume = 0; //Reset the volume
	if (DUMMYMODE) return; //In dummymode don't start the pump
	PUM[newPump].Status = newStatus;
	if (newStatus == sACTIVE)
	{
		PUM[newPump].StartRegulating = 0;
	}

}
//-----------------------------------------------------------------------------
//! \brief        Sets the control type of the selected pump
//! \details      Sets the way to control the pump. Eater pressure or flow or both
//! \param[in]    uint8_t newPump (0 - 1)
//! \param[in]    uint32_t newControlType (0 = None, 1 = Flow, 2 = Pressure, 3 = Both)
void PUM_SetControlType (uint8_t newPump, uint32_t newControlType)
{
	PUM[newPump].SetControlType = newControlType;
}

//-----------------------------------------------------------------------------
//! \brief        Sets the volume to be dispensed by the selected pump
//! \details      Global function to set the volume to be dispensed by the selected pump
//! \param[in]    uint8_t newPump (0 - 1)
//! \param[in]    uint32_t newVolume in ul
void PUM_SetVolume (uint8_t newPump, uint32_t newVolume)
{
	PUM[newPump].SetVolume = newVolume * 1000;
}

//-----------------------------------------------------------------------------
//! \brief        Sets the pressure of the selected pump to be controlled at
//! \details      Global function to start the pump at certain pressure
//! \param[in]    uint8_t newPump (0 - 1)
//! \param[in]    uint32_t newPressure (0 - 12000) mbar
void PUM_SetPressure (uint8_t newPump, uint32_t newPressure)
{
	PUM[newPump].SetPressure = newPressure;
}
//-----------------------------------------------------------------------------
//! \brief        Sets the flow of the selected pump
//! \details      Global function to start the pump
//! \param[in]    uint8_t newPump (0 - 1)
//! \param[in]    uint32_t newFlow (0 - 200) x 0.1 ml/s for P1 and P2, (0-100) % for P3
void PUM_SetFlow (uint8_t newPump, uint32_t newFlow, uint8_t newSteamMode)
{
	PUM[newPump].SetFlow = newFlow * 100; //Convert to ul/s
	PUM[newPump].SteamMode = newSteamMode;
}
//-----------------------------------------------------------------------------
//! \brief      Handles the pulse interrupt
//! \details    Increases the pulse counter and sets the pulse time
//! \param[in]	uint8_t newPump
void PUM_SetPulses (uint8_t newPump)
{
	PUM[newPump].Pulses ++;
	PUM[newPump].PulseTime = TIM6->CNT;
	FlgFM[newPump] = 1;
}
//-----------------------------------------------------------------------------
//! \brief        Sets the output of the specified pump
//! \details      Switches the output for the specified pump on or off
//! \param[in]    uint8_t newPump (0 - 1)
//! \param[in]    enuStatus newStatus (ON - OFF)
void PUM_SetPumpGPIO (uint8_t newPump, enuStatus newStatus)
{
	uint16_t x1, x2;
	if (DUMMYMODE) return;
	if (PUM[newPump].ControlType == 0)
	{

		x1 = PhaseCounterPumps/2;
		if (PUM[newPump].Percentage > 100) PUM[newPump].Percentage = 100;
		x2 = PhaseListPumps[(PUM[newPump].Percentage/10)];
		//Make sure pump is switched off if power = 0
		if ((PUM[newPump].Percentage == 0) || (PUM[newPump].Status == sINACTIVE) || (PhaseCounterPumps%2 == 0))
		{
			if (newPump == P1)
			{
				P1_OFF();
			}
			else
			{
				P2_OFF();
			}
		}
		else
		{
			if ((1<<(x1)) & (x2)) //This phase is on
			{
				if (newPump == P1)
				{
					P1_ON();
				}
				else
				{
					P2_ON();
				}
			}
			else
			{
				if (newPump == P1)
				{
					P1_OFF();
				}
				else
				{
					P2_OFF();
				}
			}
		}
	}
	else
	{
		if (newStatus == sOFF)
		{
			if (PUM[newPump].Percentage != 100)
			{
				if (newPump == 0)
				{
					P1_OFF();
				}
				else
				{
					P2_OFF();
				}
			}
		}
		else if (newStatus == sON)
		{
			if (PUM[newPump].Percentage > 10)
			{
				if (newPump == 0)
				{
					P1_ON();
				}
				else
				{
					P2_ON();
				}
			}
		}
	}
}

//---------------- End of file -----------------------
