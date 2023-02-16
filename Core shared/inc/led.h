/*
 * led.h
 *
 *  Created on: Jun 4, 2022
 *      Author: Rene
 */


#ifndef INC_LED_H_
#define INC_LED_H_
/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "ws2812.h"

#define WATERTANKLEDS	0 //1 is also water tank LED
#define OUTLETLED		2
#if (LED_SPI == 2)
#define STANDBYLED		3
#else
#define STANDBYLED		0
#endif
#define NONE			-1
#define SWITCHEDOFF 	0
#define SWITCHEDON 		1
#define BLINKING 		2
#define BREATHING 		3
#define FADEIN 			4
#define FADEOUT			5

typedef struct
{
	uint32_t Color;			//0-0xFFFFFF
	uint32_t ColorOld;		//0-0xFFFFFF
	int64_t ColorNext;
	uint8_t Effect;	//See enuLedEffect
	uint8_t EffectOld;	//See enuLedEffect
	int16_t EffectNext;	//Next effect that is started after the current effect cycle is finished. (To improve transistions)
	uint8_t Intensity;		//0-255
	uint8_t IntensityOld; 	//0-255
	int16_t IntensityNext;
	uint16_t BlinkTimeOn;	//0-1000 ms
	int32_t BlinkTimeOnNext;	//0-1000 ms
	uint16_t BlinkTimeOff;	//0-1000 ms
	int32_t BlinkTimeOffNext;	//0-1000 ms
	uint8_t BreathInterval; //0-255 ms Time between the 255 steps of intensity. Default 10
	int16_t BreathIntervalNext; //0-255 ms Time between the 255 steps of intensity. Default 10
	uint8_t FadeInInterval; //0-255 ms Time between the 255 steps of intensity. Default 10
	int16_t FadeInIntervalNext; //0-255 ms Time between the 255 steps of intensity. Default 10
	uint8_t FadeOutInterval;//0-255 ms Time between the 255 steps of intensity. Default 10
	int16_t FadeOutIntervalNext;//0-255 ms Time between the 255 steps of intensity. Default 10
	uint16_t Counter;
	uint16_t TimeCycleCounter;
	uint16_t BreathIntensity;//0-255 Intensity of the fade in 255 = 100%
	uint8_t FadeIntensity;  //0-255 Intensity of the fade in 255 = 100%
	uint16_t TimeCycles; //Number of cycles or time to run this setting
	int32_t TimeCyclesNext; //Number of cycles or time to run this setting
	uint8_t RestorePrevious; //After above time or cycles restore the previous setting or not.
	int16_t RestorePreviousNext; //After above time or cycles restore the previous setting or not.
}stcLed;

void LED_SetNext (uint8_t newLED);

extern stcLed LED[WS2812_NUM];
extern void LED_Handle (void);
extern void LED_Init(void);
extern void LED_Set (uint8_t newLED, uint8_t newEffect, uint8_t newR, uint8_t newG, uint8_t newB, uint8_t newIntensity, uint16_t newBlinkTimeOn, uint16_t newBlinkTimeOff, uint8_t newStepTime, uint16_t newTimeCycles, uint8_t newRestorePrevious);
extern void LED_RestorePrevious (uint8_t newLED);
extern void LED_StorePrevious (uint8_t newLED);
#endif /* INC_LED_H_ */
