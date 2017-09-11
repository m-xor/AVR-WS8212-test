/*
 * ws2812_syms.h
 *
 *  Created on: 24 sty 2014
 *      Author: slawek
 */

#ifndef WS2812_SYMS_H_
#define WS2812_SYMS_H_

//--------- stałe definiowalne ------
#define WS2812_LED_NUM				10		// Ile ledów w taśmie. Każda dioda
											// zabiera 3 bajty RAMu.
#define WS2812_PIN_LETTER			D		// na którym porcie jest pin wyjściowy
#define WS2812_PIN_NR				PD2		// pin wyjściowy (PD2==Arduino pin 2)
//#define WS2812_GAMMA						// odkomentuj jeśli kolor ma być poddany korekcji gamma

/*
 * F_CPU nie jest przekazywany do gas więc ustawiamy go z palucha
 * (niezbędne dla zapewnienia właściwego timingu)
 */
#ifndef F_CPU
	//#undef F_CPU	//debug
	#define F_CPU 16000000UL
#endif
//-------------------------------------


/*
 *  Makra upraszczające dostęp do portów
 *  M.Kardaś
 *  http://mirekk36.blogspot.com/2011/12/jezyk-c-makra-upraszczajace-dostep-do.html
 */
// *** PORT
#define PORT(x) SPORT(x)
#define SPORT(x) (PORT##x)
// *** PIN
#define PIN(x) SPIN(x)
#define SPIN(x) (PIN##x)
// *** DDR
#define DDR(x) SDDR(x)
#define SDDR(x) (DDR##x)

#endif /* WS2812_SYMS_H_ */
