/*
 * ws2812.h
 *
 *  Created on: 24 sty 2014
 *      Author: slawek
 */

#ifndef WS2812_H_
#define WS2812_H_

#include <avr/io.h>

//#define WS2812_BRIGHTNESS //odkomentuj jeśli ma działać globalna modyfikacja jasności

//inicjuje pin wyjściowy wg ustawień w ws2812_syms.h
extern void ws2812_init();

/*
 * zapisuje do bufora diody o nr ledno (od 0 do ilość diod-1), trzy kanały pwm
 */
extern void ws2812_set_rgb(uint16_t ledno, uint8_t pwm_red, uint8_t pwm_green, uint8_t pwm_blue);
extern void ws2812_set_color(uint16_t ledno, uint32_t rgb);

/*
 * Ustawia kolory proste (bez zamazywania pozostałych kolorów)
 */
extern void ws2812_set_red(uint16_t ledno, uint8_t pwm);
extern void ws2812_set_green(uint16_t ledno, uint8_t pwm);
extern void ws2812_set_blue(uint16_t ledno, uint8_t pwm);

/*
 * Ustawia kolory złożone
 */
extern void ws2812_set_yellow(uint16_t ledno, uint8_t pwm);
extern void ws2812_set_magenta(uint16_t ledno, uint8_t pwm);
extern void ws2812_set_cyan(uint16_t ledno, uint8_t pwm);
extern void ws2812_set_white(uint16_t ledno, uint8_t pwm);

/*
 * Składa trzy oddzielne kanały R,G,B do postaci 32-bitowej
 */
extern uint32_t ws2812_color(uint8_t r, uint8_t g, uint8_t b);

/*
 * Ustawia globalny poziom jasnosci świecenia. 0-wygaszenie, 255 - maksymalna
 * jasność (== wyłączenie modyfikacji jasności).
 * Działa jeśli kompilacja ze zdefiniowanym symbolem WS2812_BRIGHTNESS.
 */
extern void ws2812_max_brightness(uint8_t br);
/*
 * Zwraca wartość globalnego poziomu jasności
 */
extern uint8_t ws2812_get_brightness();

/*
 * Zwraca wartość RGB koloru zmodyfikowaną o poziom jasnosci 'br'. Działa niezależnie
 * od modyfikacji dokonywanych przez ws2812_max_brightness.
 */
extern uint32_t ws2812_adj_brightness(uint32_t rgb, uint8_t br);

/*
 * Zwraca ilość diod w łańcuchu
 */
uint16_t ws2812_num_leds();

/*
 * Zeruje bufor wyjściowy
 */
extern void ws2812_clear();

/*
 * Wysyła bufor do LEDów. Między kolejnymi wywołaniami wymagany jest odstęp min 50µs
 */
extern void ws2812_display();

#endif /* WS2812_H_ */
