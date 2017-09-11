/*
 * effets_1.c
 *
 *  Created on: 8 mar 2014
 *      Author: slawek
 */

#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include "ws2812.h"
#include "data.h"	//tablica kolorów


void enlight()
{
	uint16_t num = ws2812_num_leds();

	for(uint8_t i=0; i< 255; i+=1)
	{
		for(uint8_t j=0; j<num; j++ )
			ws2812_set_color(j, ws2812_adj_brightness(ws2812_color(255,255,255),i));
		ws2812_display();
		_delay_ms(30);
	}
}

void delight()
{
	uint16_t num = ws2812_num_leds();

	for(uint8_t i=0; i<num/2+1; i++)
	{
		ws2812_set_white(i, 0);
		ws2812_set_white(num-i+1, 0);
		ws2812_display();
		_delay_ms(200);
	}
}

void show_me_these_colors()
{
	uint32_t c;
	uint16_t num = ws2812_num_leds();

	for(uint8_t i=0; i<pgm_read_byte(&tcol_size); i++)
	{	c = pgm_read_dword(&tcol[i]);
		for(uint8_t j=0; j<num; j++ )
		{
			ws2812_set_color(j,ws2812_adj_brightness(c, 255-(j*(255/(num-1))) ));
			ws2812_display();
			_delay_ms(10);
		}
		//_delay_ms(10);
	}
}

void blackout()
{
	uint16_t num = ws2812_num_leds();
	for(uint8_t i=0; i<num; i++)
	{
		ws2812_set_white(i, 0);
		ws2812_display();
		_delay_us(50);
	}
}
