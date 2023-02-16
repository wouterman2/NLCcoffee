//-----------------------------------------------------------------------------
//! \file       valves.c
//! \author     R. Weimar. RMB Holland bv
//! \brief      Contains routines for the valves
//! \details
//! \Attention
//! \Created on: May 9, 2022
//-----------------------------------------------------------------------------
#include "valves.h"
#include "main.h"
#include "work.h"

//! \Local container HC595 status
uint8_t OutputRegister;
//! \Global Valid Valve Settings
//Valve options wit output buffer
//0x00: All valves closed. Release valve is open
//0x04: Circuit 1 set for milk, Circuit 2 for none
//0x08: Circuit 1 set for coffee, Circuit 2 for off
//0x20: Circuit 1 off, circuit 2 milk
//0x24: Circuit 1 set for milk, Circuit 2 for water
//0x2B: Circuit 1 set for coffee, Circuit 2 for milk
//0x42: Circuit 1 set for water, Circuit 2 for none
//0x50: Circuit 1 off, circuit 2 water
//0x52: Circuit 1 set for water, Circuit 2 for water
//0x54: Circuit 1 set for milk, Circuit 2 for milk
//0x62: Circuit 1 set for water, Circuit 2 for milk
uint8_t VAL_ValidSettings[] = {0x00,0x04,0x0B,0x20,0x24,0x2B,0x42,0x50,0x52,0x54,0x62};
uint8_t VAL_CircuitSettings[] = {0x00,0x0B,0x04,0x42,0x00,0x20,0x50,0x00,0x24,0x52};

//-----------------------------------------------------------------------------
//! \brief      Returns the output register of the valve
//! \details    Returns the bit encoded valve status
//! \param      None
uint8_t VAL_GetStatus (void)
{
	uint8_t ValveSetting = 0;
	//return OutputRegister;
	for (uint8_t i = 0; i<10;i++)
	{
		if (VAL_CircuitSettings[i] == OutputRegister) ValveSetting = i;
	}
	if ((ValveSetting == 4) || (ValveSetting == 7)) ValveSetting = 0;
	return ValveSetting;
}

//-----------------------------------------------------------------------------
//! \brief      Sets or resets the specified valve
//! \details    Sets GPIO and enables the shiftregister
//! \param[in]	uint8_t newCircuitSetting (CIRCUIT1_OFF,CIRCUIT1_COFFEE,CIRCUIT1_MILK,CIRCUIT1_WATER,CIRCUIT2_OFF,CIRCUIT2_MILK,CIRCUIT2_WATER,CIRCUIT1_2_OFFOFF,CIRCUIT1_2_MILKMILK,CIRCUIT1_2_WATERWATER)
//! \param[in]	uint8_t newIncludeS3  (0 = Don't include S3, 1 = Include S3) Only for switching off
void VAL_SetCircuit (uint8_t newCircuitSetting, uint8_t newIncludeS3)
{
	if (newCircuitSetting == CIRCUIT_1_OFF)
	{
		if (newIncludeS3) OutputRegister &= ~VAL_S3; //Reset the S3
		OutputRegister &= ~VAL_ALL_CIRCUIT_1; //Reset the valves in circuit 1
	}
	else if (newCircuitSetting == CIRCUIT_2_OFF)
	{
		if (newIncludeS3) OutputRegister &= ~VAL_S3; //Reset the S3
		OutputRegister &= ~VAL_ALL_CIRCUIT_2; //Reset the valves in circuit 2
	}
	else if (newCircuitSetting == CIRCUIT_1_2_OFF)
	{
			OutputRegister = 0;
	}
	else if ((newCircuitSetting == CIRCUIT_1_2_MILK) || (newCircuitSetting == CIRCUIT_1_2_WATER))
	{
		OutputRegister = 0; //Reset the Output buffer to 0
		OutputRegister |= VAL_CircuitSettings[newCircuitSetting]; //Set the valves in circuit 1 and 2
	}
	else if ((newCircuitSetting == CIRCUIT_1_COFFEE) || (newCircuitSetting == CIRCUIT_1_MILK) || (newCircuitSetting == CIRCUIT_1_WATER))
	{
		OutputRegister &= ~VAL_ALL_CIRCUIT_1; //Reset the valves in circuit 1
		OutputRegister |= VAL_CircuitSettings[newCircuitSetting]; //Set the valves in circuit 1
	}
	else if ((newCircuitSetting == CIRCUIT_2_MILK) || (newCircuitSetting == CIRCUIT_2_WATER))
	{
		OutputRegister &= ~VAL_ALL_CIRCUIT_2; //Reset the valves in circuit 2
		OutputRegister |= VAL_CircuitSettings[newCircuitSetting]; //Set the valves in circuit 2
	}
}

//-----------------------------------------------------------------------------
//! \brief      Sets or resets the specified valve
//! \details    Sets GPIO and enables the shiftregister
//! \param[in]	uint8_t newValve (S1,S11,S12,S2,S21,S22,S3,VAL_ALL,VAL_BUFFER)
//! \param[in]	uint8_t newStatus (VAL_SET, VAL_RESET)
void VAL_Set (uint8_t newValve, uint8_t newStatus)
{
	/*if (newValve == VAL_BUFFER)
	{
		OutputRegister = newStatus;
	}
	else if ((newValve == VAL_ALL) || (newValve == VAL_ALL_CIRCUIT_1) || (newValve == VAL_ALL_CIRCUIT_2))
	{
	*/
		if (newStatus == VAL_RESET)
		{
			OutputRegister &= ~ newValve;
		}
		else if (newStatus == VAL_SET)
		{
			OutputRegister |= newValve;
		}
	/*}
	else if ((newValve > 0) && (newValve <= NROFVALVES))
	{
		if (newStatus == VAL_SET)
		{
			OutputRegister |= (1<<newValve);
		}
		else if (newStatus == VAL_RESET)
		{
			OutputRegister &= ~(1<<newValve);
		}
	}
	*/
}

//-----------------------------------------------------------------------------
//! \brief      Initializes the valves modules
//! \details    Sets GPIO and enables the shiftregister
//! \param      None
void VAL_Init (void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOB, SER595_Pin|EN595_Pin|RCLK595_Pin|SRCLK595_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pins : SER595_Pin EN595_Pin RCLK595_Pin SRCLK595_Pin */
	GPIO_InitStruct.Pin = SER595_Pin|EN595_Pin|RCLK595_Pin|SRCLK595_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	VAL_Handle();
	//Enable the HC595
	EN595_H();
}
//-----------------------------------------------------------------------------
//! \brief      Handles the outputs of the shift register
//! \details    According to the output register controls the valves
//! \param      None
void VAL_Handle(void)
{
	static uint8_t OutputRegisterOld = 255; //Switches all valves off at startup
	static uint8_t StepCounter = 0;

	if (DUMMYMODE) return; //In dummymode, don't switch on the valves

	if (OutputRegister == OutputRegisterOld)
	{
		StepCounter = 0;
		return;
	}

	if (StepCounter > 0) SRCLK595_TOGGLE(); //Toggle the clock pin
	if (StepCounter == 0)
	{
		SER595_L();
		RCLK595_L();
		SRCLK595_L(); //Set clock pulse low
	}
	else if (StepCounter == 2)
	{
		SER595_H();
	}
	else if ((StepCounter >= 2)	&& (StepCounter < 20) && (StepCounter % 2 == 0)) //StepCounter is even and between the values
	{
		if ((OutputRegister >> (StepCounter/2-2)) & 0x01) //On
		{
			SER595_H();
		}
		else
		{
			SER595_L();
		}
	}
	else if (StepCounter == 20)
	{
		SER595_L();
		RCLK595_H();
	}
	else if (StepCounter == 21)
	{
		RCLK595_L();
		OutputRegisterOld = OutputRegister;
	}
	StepCounter++;
}
