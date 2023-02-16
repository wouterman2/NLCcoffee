//-----------------------------------------------------------------------------
//! \file       ws2812.c
//! \author     R. Weimar. RMB Holland bv
//! \brief      Contains routines for the controll of the WS2812
//! \details
//! \Attention
//! \Created on: Apr 27, 2022
//-----------------------------------------------------------------------------
#include "ws2812.h"
#include "work.h"
//-----------------------------------------------------------------------------
#define delay_ms(x)                 HAL_Delay(x)
//-----------------------------------------------------------------------------
#if (LED_SPI == 2)
extern SPI_HandleTypeDef hspi2;
extern DMA_HandleTypeDef hdma_spi2_tx;
#else
extern SPI_HandleTypeDef hspi1;
#endif
//-----------------------------------------------------------------------------
//! \Ws2812 container
ws2812_t g_ws2812={0};
uint8_t BB;
//-----------------------------------------------------------------------------
//! \brief      Calculates the color value
//! \details    Converts RGB to color integer
//! \param[in]  uint8_t red		red value (0 - 255)
//! \param[in]  uint8_t green	green value (0 - 255)
//! \param[in]  uint8_t blue	blue value (0 - 255)
//! \param[out]	uint32_t color value
uint32_t WS2812_Color(uint8_t red, uint8_t green, uint8_t blue)
{
    return red << 16 | green << 8 | blue;
}
//-----------------------------------------------------------------------------
//! \brief      Sets an LED in the array to a certain value
//! \details    Calculates and sets the color
//! \param[in]  uint8_t num		number of the LED in the array
//! \param[in]  uint32_t RGB	Color number
//! \param[in]  uint8_t intensity	Intensity of the LED (0 - 255)
void WS2812_Set( uint8_t num, uint32_t RGB, uint8_t intensity )
{
    uint8_t i;
    uint16_t R = ((RGB >> 16) & 0xff) * intensity / 255;
    uint16_t G = ((RGB >> 8) & 0xff) * intensity / 255;
    uint16_t B = ((uint8_t) RGB) * intensity / 255;
    uint32_t TempR[8], TempG[8], TempB[8];
    BB = B;
    RGB = R << 16 | G << 8 | B; //Colors corrected for intensity
    //MSB First
    for( i = 0; i < 8; ++i,RGB>>=1 )
	{
		(RGB & 0x00010000) != 0 ? (TempR[7-i] = WS2812_HIG) : (TempR[7-i] = WS2812_LOW);
		(RGB & 0x00000100) != 0 ? (TempG[7-i] = WS2812_HIG) : (TempG[7-i] = WS2812_LOW);
		(RGB & 0x00000001) != 0 ? (TempB[7-i] = WS2812_HIG) : (TempB[7-i] = WS2812_LOW);
	}

	for (uint8_t j = 0; j<8;j++)
	{
		for (i = 0; i < 3; i++)
		{
			g_ws2812.Col[num].RGB.R[i + (j * 3)] = TempR[j] >> (16-8*i);
			g_ws2812.Col[num].RGB.G[i + (j * 3)] = TempG[j] >> (16-8*i);
			g_ws2812.Col[num].RGB.B[i + (j * 3)] = TempB[j] >> (16-8*i);
		}
	}
}
//-----------------------------------------------------------------------------
//! \brief      Sets an LED in the array to a certain value
//! \details    Calculates and sets the color
//! \param[in]  uint8_t num		number of the LED in the array
//! \param[in]  uint32_t RGB	Color number
void WS2812_OneSet( uint8_t num, uint32_t RGB )
{
    uint8_t i;
    uint32_t TempR[8], TempG[8], TempB[8];

    //MSB First
    for( i = 0; i < 8; ++i,RGB>>=1 )
    {
        (RGB & 0x00010000) != 0 ? (TempR[i] = WS2812_HIG) : (TempR[i] = WS2812_LOW);
        (RGB & 0x00000100) != 0 ? (TempG[i] = WS2812_HIG) : (TempG[i] = WS2812_LOW);
        (RGB & 0x00000001) != 0 ? (TempB[i] = WS2812_HIG) : (TempB[i] = WS2812_LOW);
    }

	for (uint8_t j = 0; j<8;j++)
	{
		for (i = 0; i < 3; i++)
		{
			g_ws2812.Col[num].RGB.R[i + (j * 3)] = TempR[i] >> (16-8*i);
			g_ws2812.Col[num].RGB.G[i + (j * 3)] = TempG[i] >> (16-8*i);
			g_ws2812.Col[num].RGB.B[i + (j * 3)] = TempB[i] >> (16-8*i);
		}
	}
}


void WS2812_Show(void)
{
#if (LED_SPI == 2)
    HAL_SPI_Transmit_DMA(&hspi2, (uint8_t*)&g_ws2812.Col[0].Buff, 72*WS2812_NUM);
#elif (LED_SPI == 1)
    HAL_SPI_Transmit_IT(&hspi1, (uint8_t*)&g_ws2812.Col[0].Buff, 72);
#endif
}

void WS2812_CloseAll(void)
{
    uint16_t i;

    for (i = 0; i < WS2812_NUM; ++i)
    {
        WS2812_OneSet(i, 0);
    }
    WS2812_Show();
}

