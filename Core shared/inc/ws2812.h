/**
 * @file    ws2812.h
 * @author  Frank (pq_liu@foxmail.com)
 * @brief   WS2812
 * @version 0.1
 * @date    2021-01-16
 * 
 * @copyright Copyright (c) 2021
 * 
 */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __WS2812_H
#define __WS2812_H

#ifdef __cplusplus
extern "C" {
#endif
/* Includes ------------------------------------------------------------------*/
#include "stm32l1xx_hal.h"
#include <stdio.h>
#include <stdlib.h>

//  __
// |  |_|   0b111111111111110000000000  high level
//  _   
// | |__|   0b111111100000000000000000  low level
//#define WS2812_HIG  (0x06)
//#define WS2812_LOW  (0x04)
#define WS2812_HIG  (0xfffc00)//(0x06) was 3 bits, now is 24 bits
#define WS2812_LOW  (0xfe0000)//(0x04) was 3 bits, now is 24 bits
#define WS2812_BITS	24
//WS2812������
#define WS2812_NUM   5

/* typedef -------------------------------------------------------------------*/
typedef struct
{
    uint8_t hig;
    uint8_t mid;
    uint8_t low;
} ws2812ColorBit_t;

typedef struct
{
    uint32_t Info;                      //reserve
    union 
    {
        uint8_t Buff[72];
        struct 
        {
            uint8_t G[24];               //G First
            uint8_t R[24];
            uint8_t B[24];               //
        }RGB;
    }Col[WS2812_NUM];
} ws2812_t;

/* types ---------------------------------------------------------------------*/
extern ws2812_t g_ws2812;

uint32_t WS2812_Color(uint8_t red, uint8_t green, uint8_t blue);
void WS2812_Set( uint8_t num, uint32_t RGB, uint8_t intensity);
void WS2812_OneSet( uint8_t num, uint32_t RGB );
void WS2812_Show(void);
void WS2812_CloseAll(void);
void WS2812_SetAll(uint32_t RGB);

void WS2812_ColorWipe(uint32_t c, uint16_t wait);

void WS2812_SingleBreatheRainbow(uint16_t wait);
void WS2812_RainbowRotate(uint16_t wait);

void WS2812_TheaterChase(uint32_t c, uint16_t wait);
void WS2812_TheaterChaseRainbow(uint16_t wait);

void WS2812_RandAll(uint16_t wait);
void WS2812_RandColorWipe(uint16_t wait);

void WS2812_Show(void);

#ifdef __cplusplus
}
#endif

#endif /* __WS2812_H */

/******************************** END *****************************************/
