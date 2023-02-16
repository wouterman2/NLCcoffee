//-----------------------------------------------------------------------------
//! \file       led.c
//! \author     R. Weimar. RMB Holland bv
//! \brief      Contains routines for the LED module
//! \details
//! \Attention
//! \Created on: June 9, 2022
//-----------------------------------------------------------------------------
#include "led.h"
#include "ws2812.h"
//-----------------------------------------------------------------------------
//! \LED container
stcLed LED[WS2812_NUM];
stcLed LEDPREVIOUS[WS2812_NUM];

void LED_StorePrevious (uint8_t newLED)
{
	uint8_t	FirstLED;
	uint8_t	LastLED;

	FirstLED = newLED;
	LastLED = newLED;
	if (newLED == WATERTANKLEDS) LastLED ++;
	for (uint8_t i = FirstLED; i<= LastLED; i ++)
	{
		LEDPREVIOUS[i].BreathInterval = LED[i].BreathInterval;
		LEDPREVIOUS[i].FadeInInterval = LED[i].FadeInInterval;
		LEDPREVIOUS[i].FadeOutInterval = LED[i].FadeOutInterval;
		LEDPREVIOUS[i].BlinkTimeOn = LED[i].BlinkTimeOn;
		LEDPREVIOUS[i].BlinkTimeOff = LED[i].BlinkTimeOff;
		LEDPREVIOUS[i].Effect = LED[i].Effect;
		LEDPREVIOUS[i].Color = LED[i].Color;
		LEDPREVIOUS[i].Intensity = LED[i].Intensity;
	}
}
void LED_RestorePrevious (uint8_t newLED)
{
	uint8_t	FirstLED;
	uint8_t	LastLED;

	FirstLED = newLED;
	LastLED = newLED;
	if (newLED == WATERTANKLEDS) LastLED ++;
	for (uint8_t i = FirstLED; i<= LastLED; i ++)
	{
		LED[i].BreathInterval = LEDPREVIOUS[i].BreathInterval;
		LED[i].FadeInInterval = LEDPREVIOUS[i].FadeInInterval;
		LED[i].FadeOutInterval = LEDPREVIOUS[i].FadeOutInterval;
		LED[i].BlinkTimeOn = LEDPREVIOUS[i].BlinkTimeOn;
		LED[i].BlinkTimeOff = LEDPREVIOUS[i].BlinkTimeOff;
		LED[i].Effect = LEDPREVIOUS[i].Effect;
		LED[i].Color = LEDPREVIOUS[i].Color;
		LED[i].Intensity = LEDPREVIOUS[i].Intensity;
		LED[i].TimeCycleCounter = 0;
	}
}

//-----------------------------------------------------------------------------
//! \brief      Routine to set an LED in the array
//! \details    Sets color, intensity, timing and effect of the LED
//! \details    Note: LED 1 and LED 2 in the array are considered one LED
//! \details    newLED = 2 > STANDBYLED = LED 3 of the array // Was LED 0
//! \details    newLED = 0 > WATERTANKLEDS = LED 0 + LED 1 of the array //Was LED 1 + LED 2
//! \details    newLED = 1 > OUTLETLED = LED 2 of the array // Was LED3
//! \param[in]  uint8_t newLED, enuLedEffect newEffect, uint8_t newR
//! \param[in]  uint8_t newG, uint8_t newB, uint8_t newIntensity
//! \param[in]  uint16_t newBlinkTimeOn, uint16_t newBlinkTimeOff, uint8_t newStepTime
//! \param[in]  uint16_t newTimeCycles, uint8_t newRestorePrevious
void LED_Set (uint8_t newLED, uint8_t newEffect, uint8_t newR, uint8_t newG, uint8_t newB, uint8_t newIntensity, uint16_t newBlinkTimeOn, uint16_t newBlinkTimeOff, uint8_t newStepTime, uint16_t newTimeCycles, uint8_t newRestorePrevious)
{
	uint8_t	FirstLED;
	uint8_t	LastLED;

	FirstLED = newLED;
	LastLED = newLED;
	if (newLED == WATERTANKLEDS) LastLED ++;
	//Store previous setting if previous setting needs to be restored and TimeCycles > 0 and Effect != FADEIN or FADEOUT or SWITCHEDOFF
	//Restore previous is only done for SWITCHEDON, BLINKING and BREATHING. Others will be ignored.
	if ((newTimeCycles > 0) &&(newRestorePrevious) && (newEffect != FADEIN) && (newEffect != FADEOUT) && (newEffect != SWITCHEDOFF))
	{
		LED_StorePrevious (newLED);
	}

	for (uint8_t i = FirstLED; i<= LastLED; i ++)
	{
		if (newEffect == BREATHING) LED[i].BreathIntervalNext = newStepTime;
		if (newEffect == FADEIN) LED[i].FadeInIntervalNext= newStepTime;
		if (newEffect == FADEOUT) LED[i].FadeOutIntervalNext= newStepTime;
		if (newEffect == BLINKING)
		{
			LED[i].BlinkTimeOnNext = newBlinkTimeOn;
			LED[i].BlinkTimeOffNext = newBlinkTimeOff;
		}
		LED[i].EffectNext = newEffect;
		if (newR + newB + newG > 0) //If all are 0 color is unchanged
		{
			LED[i].ColorNext = newR << 16 | newG << 8 | newB;
		}
		else
		{
			LED[i].ColorNext = LED[i].Color;
		}
		LED[i].IntensityNext = newIntensity;
		//Set TimeCycles and RestorePrevious if TimeCycles > 0 and Effect is SWITCHEDON, BLINKING or BREATHING
		if (((newEffect == SWITCHEDON) || (newEffect == BLINKING) || (newEffect == BREATHING)) && (newTimeCycles > 0))
		{
			LED[i].TimeCyclesNext = newTimeCycles;
			LED[i].RestorePreviousNext = newRestorePrevious;
			LED[i].Counter = 0;
		}
		else
		{
			LED[i].TimeCyclesNext = 0;
			LED[i].RestorePreviousNext = 0;
		}
	}
}
void LED_SetNext (uint8_t newLED)
{
	if (LED[newLED].EffectNext == -1)
	{
		LED[newLED].ColorNext = -1;
		LED[newLED].IntensityNext = -1;
		LED[newLED].BlinkTimeOnNext = -1;
		LED[newLED].BlinkTimeOffNext = -1;
		LED[newLED].BreathIntervalNext = -1;
		LED[newLED].FadeInIntervalNext = -1;
		LED[newLED].FadeOutIntervalNext = -1;
		LED[newLED].TimeCyclesNext = -1;
		LED[newLED].RestorePreviousNext = -1;
		return;
	}
	else if (LED[newLED].EffectNext  == BREATHING)
	{
		LED[newLED].BreathInterval = LED[newLED].BreathIntervalNext;
		LED[newLED].BreathIntervalNext = -1;
	}
	else if (LED[newLED].EffectNext == FADEIN)
	{
		LED[newLED].FadeInInterval = LED[newLED].FadeInIntervalNext;
		LED[newLED].FadeInIntervalNext = -1;
	}
	else if (LED[newLED].EffectNext == FADEOUT)
	{
		LED[newLED].FadeOutInterval = LED[newLED].FadeOutIntervalNext;
		LED[newLED].FadeOutIntervalNext = -1;
		LED[newLED].FadeIntensity = 255;
	}
	else if (LED[newLED].EffectNext == BLINKING)
	{
		LED[newLED].BlinkTimeOn = LED[newLED].BlinkTimeOnNext;
		LED[newLED].BlinkTimeOnNext = -1;
		LED[newLED].BlinkTimeOff = LED[newLED].BlinkTimeOffNext ;
		LED[newLED].BlinkTimeOffNext = -1;
		LED[newLED].Counter = 0;
	}
	LED[newLED].Effect = LED[newLED].EffectNext ;
	LED[newLED].EffectNext = -1;
	LED[newLED].Color = LED[newLED].ColorNext ;
	LED[newLED].ColorNext = -1;
	LED[newLED].Intensity = LED[newLED].IntensityNext ;
	LED[newLED].IntensityNext = -1;
	LED[newLED].TimeCycles = LED[newLED].TimeCyclesNext;
	LED[newLED].TimeCyclesNext = -1;
	LED[newLED].RestorePrevious = LED[newLED].RestorePreviousNext;
	LED[newLED].RestorePreviousNext = -1;
	LED[newLED].TimeCycleCounter = 0;
}
//-----------------------------------------------------------------------------
//! \brief      Initializes the LED module
//! \details    Set all LED off
//! \params     None
void LED_Init(void)
{
	WS2812_CloseAll();
	for (uint8_t i = 0; i < WS2812_NUM; i++)
	{
		LED[i].Effect = SWITCHEDOFF;
		LED[i].EffectNext = NONE;
		LED[i].Color = 0;
		LED[i].ColorNext = -1;
		LED[i].Intensity = 0;
		LED[i].IntensityNext = -1;
		LED[i].BlinkTimeOn = 500;
		LED[i].BlinkTimeOnNext = -1;
		LED[i].BlinkTimeOff = 500;
		LED[i].BlinkTimeOffNext = -1;
		LED[i].BreathInterval = 5;
		LED[i].BreathIntervalNext = -1;
		LED[i].FadeInInterval = 10;
		LED[i].FadeInIntervalNext = -1;
		LED[i].FadeOutInterval = 20;
		LED[i].FadeOutIntervalNext = -1;
		LED[i].TimeCycles = 0;
		LED[i].RestorePrevious = 0;
	}
}
//-----------------------------------------------------------------------------
//! \brief      Handles the LED control
//! \details    Calculates the intensity of the LED and sets if needed
//! \params		None
void LED_Handle (void)
{
	uint8_t NeedsUpdate = 0;


	for (uint8_t i = 0; i < WS2812_NUM;i++)
	{
		NeedsUpdate= 0;
		if (i==1) i=2;
		if (LED[i].Color != LED[i].ColorOld)
		{
			NeedsUpdate = 1;
			LED[i].ColorOld = LED[i].Color;

		}
		else if (LED[i].Intensity != LED[i].IntensityOld)
		{
			NeedsUpdate = 1;
			LED[i].IntensityOld = LED[i].Intensity;

		}
		else if (LED[i].Effect != LED[i].EffectOld)
		{
			LED[i].Counter = 0;
			LED[i].BreathIntensity = 0;
			LED[i].FadeIntensity = 0;
			if (LED[i].Effect == FADEOUT) LED[i].FadeIntensity = 255;
			NeedsUpdate = 1;
			LED[i].EffectOld = LED[i].Effect;
		}
		if (LED[i].Effect == BLINKING)
		{
			if (LED[i].Counter >= LED[i].BlinkTimeOn + LED[i].BlinkTimeOff) //Needs to switch off
			{
				if (LED[i].TimeCycles > 0)
				{
					if (LED[i].TimeCycleCounter >= LED[i].TimeCycles)
					{
						if (LED[i].RestorePrevious)
						{
							if ((LEDPREVIOUS[i].Effect != BREATHING) && (LEDPREVIOUS[i].Effect != FADEIN))
							LED_RestorePrevious(i);
						}
						else
						{
							LED[i].Effect = SWITCHEDOFF;
						}
						NeedsUpdate = 1;
					}
				}
				LED[i].Counter = 0;
				NeedsUpdate = 1;
			}
			else if (LED[i].Counter == LED[i].BlinkTimeOn) //Needs to switch on
			{
				if (LED[i].TimeCycles > 0)
				{
					if (LED[i].TimeCycleCounter >= LED[i].TimeCycles)
					{
						if (LED[i].RestorePrevious)
						{
							if  ((LEDPREVIOUS[i].Effect == BREATHING) || (LEDPREVIOUS[i].Effect == FADEIN))
								LED_RestorePrevious(i);
						}
						else
						{
							LED[i].Effect = SWITCHEDOFF;
						}
						NeedsUpdate = 1;
					}
					else
					{
						LED[i].TimeCycleCounter ++;
					}
				}
				NeedsUpdate = 1;
			}
			else if (LED[i].Counter == 0) //Make sure LED is on after first started
			{
				NeedsUpdate = 1;
			}
			LED[i].Counter ++;
		}
		else if (LED[i].Effect == BREATHING)
		{
			LED[i].Counter ++;
			if (LED[i].Counter >= LED[i].BreathInterval)
			{
				LED[i].Counter = 0;
				NeedsUpdate = 1;
			}
		}
		else if (LED[i].Effect == FADEIN)
		{
			if (LED[i].FadeIntensity < 255)
			{
				LED[i].Counter ++;
				if (LED[i].Counter >= LED[i].FadeInInterval)
				{
					LED[i].Counter = 0;
					NeedsUpdate = 1;
				}
			}
			else
			{
				LED[i].Effect = SWITCHEDON;
			}
		}
		else if (LED[i].Effect == FADEOUT)
		{
			if (LED[i].FadeIntensity > 0)
			{
				LED[i].Counter ++;
				if (LED[i].Counter >= LED[i].FadeOutInterval)
				{
					LED[i].Counter = 0;
					NeedsUpdate = 1;
				}
			}
			else
			{
				LED[i].Effect = SWITCHEDOFF;
			}
		}
		else if (LED[i].Effect == SWITCHEDOFF)
		{
			if (LED[i].EffectNext > SWITCHEDOFF)
			{
				LED_SetNext(i);
				LED[i].BreathIntensity = 0; // Make sure breathing is starting with breathing up
				NeedsUpdate = 1;
			}
		}
		else if (LED[i].Effect == SWITCHEDON)
		{
			//If this LED setting is time limited, count the time and switch off, or restore previous after time elapsed.
			if (LED[i].TimeCycles > 0)
			{
				if (LED[i].TimeCycleCounter > LED[i].TimeCycles)
				{
					if (LED[i].RestorePrevious)
					{
						LED_RestorePrevious(i);
					}
					else
					{
						LED[i].Effect = SWITCHEDOFF;
					}
					NeedsUpdate = 1;
				}
				else
				{
					LED[i].TimeCycleCounter ++;
				}
			}
			else
			{
				LED[i].TimeCycleCounter = 0;
			}

			if (LED[i].EffectNext > SWITCHEDOFF)
			{
				LED_SetNext(i);
				if (LED[i].Effect == BREATHING)
					LED[i].BreathIntensity = 255; //Make sure breathing starts with breathing down
				NeedsUpdate = 1;
			}
		}
		if (NeedsUpdate)
		{
			if (LED[i].Effect == SWITCHEDOFF)
			{
				if (i==0) WS2812_Set(1, LED[0].Color,0);//Also other water tank LED
				WS2812_Set(i, LED[i].Color,0);

			}
			else if (LED[i].Effect == SWITCHEDON)
			{
				if (i==0) WS2812_Set(1, LED[0].Color,LED[0].Intensity);//Also other water tank LED
				WS2812_Set(i, LED[i].Color,LED[i].Intensity);
			}
			else if (LED[i].Effect == BLINKING)
			{
				if (LED[i].Counter >= LED[i].BlinkTimeOn) //Is on, needs to switch off
				{
					if ((LED[i].EffectNext == SWITCHEDON) || (LED[i].EffectNext == FADEOUT))
					{
						LED_SetNext (i);
					}
					else
					{
						if (i==0) WS2812_Set(1, LED[0].Color,0);//Also other water tank LED
						WS2812_Set(i, LED[i].Color,0);
					}
				}
				else //Is off, needs to switch on
				{
					if (((LED[i].EffectNext != SWITCHEDON) || (LED[i].EffectNext != FADEOUT)) && (LED[i].EffectNext >=0))
					{
						LED_SetNext (i);
					}
					else
					{
						if (i==0) WS2812_Set(1, LED[0].Color,LED[0].Intensity);//Also other water tank LED
						WS2812_Set(i, LED[i].Color,LED[i].Intensity);
					}

				}
			}
			else if (LED[i].Effect == BREATHING)
			{
				LED[i].BreathIntensity ++;
				if (LED[i].BreathIntensity > 510)
				{
					if ((LED[i].EffectNext != SWITCHEDON) && (LED[i].EffectNext != FADEOUT))
					{
						LED_SetNext (i);
					}
					LED[i].BreathIntensity = 0;
					if (LED[i].TimeCycles > 0)
					{
						if (LED[i].TimeCycleCounter >= LED[i].TimeCycles)
						{
							if (LED[i].RestorePrevious)
							{
								if  ((LEDPREVIOUS[i].Effect == BREATHING) || (LEDPREVIOUS[i].Effect == FADEIN))
									LED_RestorePrevious(i);
							}
							else
							{
								LED[i].Effect = SWITCHEDOFF;
							}
							NeedsUpdate = 1;
						}
						else
						{
							LED[i].TimeCycleCounter ++;
						}
					}
				}
				if (LED[i].BreathIntensity <= 255)
				{
					if (LED[i].BreathIntensity == 255) //LED at max intensity. If Next is SWITCHED ON or FadeOut, switch to next
					{
						if (LED[i].TimeCycles > 0)
						{
							if (LED[i].TimeCycleCounter >= LED[i].TimeCycles)
							{
								if (LED[i].RestorePrevious)
								{
									if ((LEDPREVIOUS[i].Effect != BREATHING) && (LEDPREVIOUS[i].Effect != FADEIN))
									LED_RestorePrevious(i);
								}
								else
								{
									LED[i].Effect = SWITCHEDOFF;
								}
								NeedsUpdate = 1;
							}
						}
						if ((LED[i].EffectNext == SWITCHEDON) || (LED[i].EffectNext == FADEOUT))
						{
							LED_SetNext (i);
						}
					}
					if (i==0) WS2812_Set(1, LED[0].Color,((uint32_t) LED[0].Intensity * (uint32_t) LED[0].BreathIntensity) / 255);//Also other water tank LED
					WS2812_Set(i, LED[i].Color,((uint32_t) LED[i].Intensity * (uint32_t) LED[i].BreathIntensity) / 255);
				}
				else
				{
					if (i==0) WS2812_Set(1, LED[0].Color,((uint32_t) LED[0].Intensity * (uint32_t) (510-LED[i].BreathIntensity) / 255));//Also other water tank LED
					WS2812_Set(i, LED[i].Color,(uint8_t) ((uint32_t) LED[i].Intensity * (uint32_t) (510-LED[i].BreathIntensity) / 255));
				}
			}
			else if (LED[i].Effect == FADEIN)
			{
				if (LED[i].FadeIntensity < 255)
				{
					LED[i].FadeIntensity ++;
					if (i==0) WS2812_Set(1, LED[0].Color,((uint32_t) LED[0].Intensity * (uint32_t) LED[0].FadeIntensity) / 255);
					WS2812_Set(i, LED[i].Color,((uint32_t) LED[i].Intensity * (uint32_t) LED[i].FadeIntensity) / 255);
				}
			}
			else if (LED[i].Effect == FADEOUT)
			{
				if (LED[i].FadeIntensity > 0)
				{
					LED[i].FadeIntensity --;
					if (i==0) WS2812_Set(1, LED[0].Color,((uint32_t) LED[0].Intensity * (uint32_t) LED[0].FadeIntensity) / 255);
					WS2812_Set(i, LED[i].Color,((uint32_t) LED[i].Intensity * (uint32_t) LED[i].FadeIntensity) / 255);
				}
			}
			WS2812_Show();
			LED[i].EffectOld = LED[i].Effect;
			LED[i].IntensityOld = LED[i].Intensity;
			LED[i].ColorOld = LED[i].Color;
		}
	}
}

