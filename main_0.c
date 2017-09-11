/*
 * main.c
 *
 *  Created on: 24 sty 2014
 *      Author: slawek
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "timers.h"		//funkcje związane z timerami
#include "ws2812.h"			//funkcje prymitywne obsługi diodek
#include "effets_1.h"		//jakieś efekty
#include "effets_arduino.h"	//efekty zgapione z biblioteki AdafruitNeoPixel

//#define ARR_SIZE(a) sizeof(a)/sizeof(a[0])

#define MAX_BRIGHT 32

// prędkość przemieszczania światełka (właściwie okres w jakim światełko przeskakuje z pozycji na pozycję)
#define RED_PERIOD		23 //47
#define GREEN_PERIOD	17 //41
#define BLUE_PERIOD		19 //43

void check_rev_dir(uint16_t pos, int8_t *dir, uint16_t num);

/**** main ******/
int main()
{
	ws2812_init();
	init_soft_timers();
	init_semaphore();
	ws2812_max_brightness(MAX_BRIGHT); //ustawienei maksymalnej jasności wyświetlania

//	DDRD |= _BV(PD4); //diodka kontrolna toglowania na tym samym porcie co LEDki
//	PORTD |= _BV(PD4);

	_delay_ms(1000);
	//---- effets_1.c -----
	//rozjaśnienie diodek
	enlight();
	_delay_ms(200);
//	PORTD &=~_BV(PD4);
	//wygaszenie diodek
	delight();
	//wszystkie kolory
	show_me_these_colors();
	blackout();
	_delay_ms(200);

	//----- effets_arduino.c
	//colorWipe(ws2812_adj_brightness(ws2812_color(255,0,0)), 50);
	colorWipe(ws2812_color(255,0,0), 50);
	//colorWipe(ws2812_adj_brightness(ws2812_color(0,255,0)), 50);
	colorWipe(ws2812_color(0,255,0), 50);
	//colorWipe(ws2812_adj_brightness(ws2812_color(0,0,255)), 50);
	colorWipe(ws2812_color(0,0,255), 50);

	//theaterChase(ws2812_adj_brightness(ws2812_color(127,127,127)), 50);
	theaterChase(ws2812_color(127,127,127), 50);
	//theaterChase(ws2812_adj_brightness(ws2812_color(127,0,0)), 50);
	theaterChase(ws2812_color(127,0,0), 50);
	//theaterChase(ws2812_adj_brightness(ws2812_color(0,0,127)), 50);
	theaterChase(ws2812_color(0,0,127), 50);

	rainbow(20);
	rainbowCycle(20);
	theaterChaseRainbow(50);

	//--- gonitwa diodek
	uint16_t p_red=5, p_green=5, p_blue=5; //pozycja
	int8_t z_red=-1, z_green=-1, z_blue=1; //zwrot

	check_rev_dir( p_red, &z_red, ws2812_num_leds() );
	check_rev_dir( p_green, &z_green, ws2812_num_leds() );
	check_rev_dir( p_blue, &z_blue, ws2812_num_leds() );

	ws2812_set_red(p_red,MAX_BRIGHT);
	ws2812_set_green(p_green,MAX_BRIGHT);
	ws2812_set_blue(p_blue,MAX_BRIGHT);

	TCNT0 = 0;
	c_red = RED_PERIOD;
	c_green = GREEN_PERIOD;
	c_blue = BLUE_PERIOD;
	sei();
	while(1)
	{
		if(!c_red)
		{
			ws2812_set_red(p_red,0);
			p_red+=z_red;
			ws2812_set_red(p_red,MAX_BRIGHT);
			check_rev_dir( p_red, &z_red, ws2812_num_leds() );
			c_red = RED_PERIOD;
		}

		if(!c_green)
		{
			ws2812_set_green(p_green,0);
			p_green+=z_green;
			ws2812_set_green(p_green,MAX_BRIGHT);
			check_rev_dir( p_green, &z_green, ws2812_num_leds() );
			c_green = GREEN_PERIOD;
		}

		if(!c_blue)
		{
			ws2812_set_blue(p_blue,0);
			p_blue+=z_blue;
			ws2812_set_blue(p_blue,MAX_BRIGHT);
			check_rev_dir( p_blue, &z_blue, ws2812_num_leds() );
			c_blue = BLUE_PERIOD;
		}

		semaphore_wait();
		ws2812_display();
		semaphore_lock();

	}
}

/*
 *** funkcje ***
 */

/*
 * Odwraca zwrot przemieszczania światełka dla skrajnych położeń
 */
void check_rev_dir(uint16_t pos, int8_t *dir, uint16_t num)
{
	if( ( !pos && *dir<0 ) || ( pos==num-1 && *dir>0 ) )
		*dir*=-1;
}

