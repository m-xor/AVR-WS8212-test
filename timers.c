/*
 * timers.c
 *
 *  Created on: 9 mar 2014
 *      Author: slawek
 */

#include <avr/io.h>
#include <avr/interrupt.h>

volatile uint8_t c_red, c_green, c_blue; //prędkość przemieszczania

void init_soft_timers()
{
	TCCR0A = _BV(WGM01); //CTC bez wyjścia na piny
	//*** testowo ***
	//TCCR0A |= _BV(COM0A0); //togle PD6(6) on compare match
	//DDRD |= _BV(PD6);	//PD6 jako wyjście do testów
	//*** koniec ****
	OCR0A = (F_CPU/102400)-1; //odmierzanie 10ms
	TCCR0B = _BV(CS00) | _BV(CS02); //preskaler 1024
	TIMSK0 = _BV(OCIE0A);	//odmaskuj przerwanie compare match
}

ISR(TIMER0_COMPA_vect)
{
	uint8_t tmp;

	tmp = c_red;
	if(tmp) c_red = --tmp;

	tmp = c_green;
	if(tmp) c_green = --tmp;

	tmp = c_blue;
	if(tmp) c_blue = --tmp;

//	PORTD ^=_BV(PD4);
}


//odliczanie 50µs dla zatrzasku diodek
void init_semaphore()
{
	TCCR2A = _BV(WGM21); //CTC bez wyjścia na piny
	//*** testowo ***
	//TCCR2A |= _BV(COM2A0); //togle PB3(11) on compare match
	//DDRB |= _BV(PB3);	//PB3 jako wyjście do testów
	//*** koniec ****
	OCR2A = (F_CPU/1000000UL*50/8); //odmierzanie 50µs (nie pominiejszane o 1 by uniknąć resetu preskalera)
	TCCR2B = _BV(CS21); //preskaler 8
}

//zacznij odliczanie
void semaphore_lock()
{
	TCNT2 = 0; //zresetuj licznik
	TIFR2 = _BV(OCF2A); //wyzeruj flagę
}

//wstrzymaj aż minie zadany czas
void semaphore_wait()
{
	while( ! ( TIFR2 & _BV(OCF2A) ) );
}
