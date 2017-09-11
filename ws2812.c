/*
 * ws2812.c
 *
 *  Created on: 24 sty 2014
 *      Author: slawek
 */

#include <avr/pgmspace.h>
#include <string.h>
#include "ws2812.h"
#include "ws2812_syms.h"

typedef enum { GREEN, RED, BLUE } COLOR;

/*
 * Frame buffer
 * nr LEDa: 0  1  2  3  4  5  6  7  8  9
 * indeks:  012345678901234567890123456789
 *                    1         2
 * kolor:   GRBGRBGRBGRBGRBGRBGRBGRBGRBGRB
 */
uint8_t ws2812_fbuf[WS2812_LED_NUM*3];

static uint8_t brightness;


//ustawia pin wyjściowy
void ws2812_init()
{
	DDR(WS2812_PIN_LETTER) |=_BV(WS2812_PIN_NR);
}

// ledno - numer kolejny LEDa (0 - WS2812_LED_NUM-1)
// pwm - moc swiecenia (0-255)
static inline void ws2812_set(uint16_t ledno, uint8_t pwm, COLOR color)
{
#ifdef WS2812_BRIGHTNESS
	ws2812_fbuf[ledno*3+color] = brightness ? (pwm * brightness) >> 8 : pwm;
#else
	ws2812_fbuf[ledno*3+color] = pwm;
#endif
}

void ws2812_set_rgb(uint16_t ledno, uint8_t pwm_red, uint8_t pwm_green, uint8_t pwm_blue)
{
	ws2812_set(ledno,pwm_red,RED);
	ws2812_set(ledno,pwm_green,GREEN);
	ws2812_set(ledno,pwm_blue,BLUE);
}

void ws2812_set_color(uint16_t ledno, uint32_t rgb)
{
	ws2812_set(ledno,(uint8_t)(rgb>>16),RED);
	ws2812_set(ledno,(uint8_t)(rgb>>8),GREEN);
	ws2812_set(ledno,(uint8_t)rgb,BLUE);
}

void ws2812_set_red(uint16_t ledno, uint8_t pwm)
{
	ws2812_set(ledno,pwm,RED);
}

void ws2812_set_green(uint16_t ledno, uint8_t pwm)
{
	ws2812_set(ledno,pwm,GREEN);
}


void ws2812_set_blue(uint16_t ledno, uint8_t pwm)
{
	ws2812_set(ledno,pwm,BLUE);
}

void ws2812_set_yellow(uint16_t ledno, uint8_t pwm)
{
	ws2812_set(ledno,pwm,RED);
	ws2812_set(ledno,pwm,GREEN);
}

void ws2812_set_magenta(uint16_t ledno, uint8_t pwm)
{
	ws2812_set(ledno,pwm,RED);
	ws2812_set(ledno,pwm,BLUE);
}

void ws2812_set_cyan(uint16_t ledno, uint8_t pwm)
{
	ws2812_set(ledno,pwm,GREEN);
	ws2812_set(ledno,pwm,BLUE);
}

void ws2812_set_white(uint16_t ledno, uint8_t pwm)
{
	ws2812_set_rgb(ledno,pwm,pwm,pwm);
}

uint32_t ws2812_color(uint8_t r, uint8_t g, uint8_t b)
{
  return ((uint32_t)r << 16) | ((uint32_t)g <<  8) | b;
}

void ws2812_max_brightness(uint8_t br)
{
	brightness = br +1;
}

uint8_t ws2812_get_brightness()
{
	return brightness - 1;
}

uint32_t ws2812_adj_brightness(uint32_t rgb, uint8_t br)
{
    uint8_t r = (uint8_t)(rgb >> 16);
    uint8_t g = (uint8_t)(rgb >>  8);
    uint8_t b = (uint8_t)rgb;

	if(++br)
	{
	      r = (r * br) >> 8;
	      g = (g * br) >> 8;
	      b = (b * br) >> 8;
	}

	return ((uint32_t)r << 16) | ((uint32_t)g <<  8) | b;
}

uint16_t ws2812_num_leds()
{
	return WS2812_LED_NUM;
}

void ws2812_clear()
{
	memset(ws2812_fbuf,'\0',WS2812_LED_NUM*3);
}
