//-----------------------------------------------------------------------------
//! \file       work.c
//! \author     R. Weimar. RMB Holland bv
//! \brief      Contains routines for the general sequence
//! \details
//! \Attention
//! \Created on: May 5, 2022
//-----------------------------------------------------------------------------
#include "AM2302.h"
#include "gpio.h"
#include "tim.h"
#include "work.h"
//! \Local sensor container
static StcSensor	Sensor;
//! \Local Timer Value array
static uint16_t TimerValues[82];

//-----------------------------------------------------------------------------
//! \brief      Stores the time in the TimerValues array
//! \details    Writes a value in the TimerValue array
//! \param[in]	uint8_t newCounter
//! \param[in]	uint16_t newTime
void AM_SetTimerValue(uint8_t newCounter, uint16_t newTime)
{
	TimerValues[newCounter] = newTime;
}
//-----------------------------------------------------------------------------
//! \brief      Returns the humidity
//! \details    Returns the humidity
//! \param[out]	float Humidity
float AM_GetHumidity (void)
{
	if (DUMMYMODE)
		return 545;
	else
		return Sensor.Humidity;
}
//-----------------------------------------------------------------------------
//! \brief      Returns the temperature
//! \details    Returns the temperature
//! \param[out]	uint16_t Temperature
float AM_GetTemperature (void)
{
	if (DUMMYMODE)
		return 221;
	else
		return Sensor.Temperature;
}
//! \brief      Updates the humidity and temperature values
//! \details    Starts the read sequence
//! \param		None
void AM_UpdateValues (void)
{
	Sensor.Counter = 1;
}
//! \brief      Returns the sensor counter
//! \details    Returns the sensor counter
//! \param[out]	uint8_t Sensor.Counter
uint8_t AM_GetSensorCounter (void)
{
	return Sensor.Counter;
}
//! \brief      Increases the sensor counter
//! \details    Increases the sensor counter
//! \param		None
void AM_IncreaseSensorCounter (void)
{
	Sensor.Counter++;
}

//-----------------------------------------------------------------------------
//! \brief      Handles the reading of the sensor
//! \details    Non blocking code to handle the sensor
//! \param      None
void AM_HandleSensor (void)
{
	if (Sensor.Counter == 0)
	{
		return;
	}
	else if (Sensor.Counter == 1)
	{
		AM_SetSensorGPIO(GPIO_MODE_OUTPUT_PP);
		DataLow();
		Sensor.Counter = 2;
	}
	else if ((Sensor.Counter >= 2) && (Sensor.Counter < 17)) //15 ms
	{
		Sensor.Counter ++;
	}
	else if (Sensor.Counter == 17)
	{
		DataHigh();
		AM_SetSensorGPIO(GPIO_MODE_INPUT);
		Sensor.Counter = 18;
	}
	else if (Sensor.Counter == 100)
	{
		for (uint8_t i = 1; i<80; i++)
		{
			TimerValues[i] = TimerValues[i+1] - TimerValues[i];
		}
		for (uint8_t j = 0; j < 5; j++)
		{
			Sensor.RawValues[j] = 0;
			for (uint8_t i = 1 + j * 16; i<16 + j * 16; i+=2)
			{
				if ((TimerValues[i+1] > 55) && (TimerValues[i+1] < 85)) //Logical 1
				{
					Sensor.RawValues[j] = (Sensor.RawValues[j] << 1) + 1;
				}
				else
				{
					Sensor.RawValues[j] = (Sensor.RawValues[j] << 1);
				}
			}
		}
		Sensor.CheckSum = (uint8_t) (Sensor.RawValues[0]+Sensor.RawValues[1]+Sensor.RawValues[2]+Sensor.RawValues[3]);
		if (Sensor.CheckSum == Sensor.RawValues[4])
		{
			Sensor.Humidity = Sensor.RawValues[0] << 8 | Sensor.RawValues[1];
			Sensor.Temperature = (Sensor.RawValues[2] & 0x7f) << 8 | Sensor.RawValues[3];
		}
		else
		{
			//Checksum error. Action needed
		}
		Sensor.Counter = 0;
	}
}
//-----------------------------------------------------------------------------
//! \brief      Sets the sensor GPIO to input or output
//! \details    Based on the parameter sets the sensor to input or output
//! \param      None
void AM_SetSensorGPIO (uint32_t newType)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	/*Configure GPIO pin : PtPin */
	GPIO_InitStruct.Pin = HT1_Pin;
	if (newType == GPIO_MODE_INPUT)
	{
		GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
		HAL_GPIO_Init(HT1_GPIO_Port, &GPIO_InitStruct);
		HAL_NVIC_SetPriority(EXTI3_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(EXTI3_IRQn);
	}
	else
	{
		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
		HAL_GPIO_Init(HT1_GPIO_Port, &GPIO_InitStruct);
	}

}
