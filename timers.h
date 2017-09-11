/*
 * timers.h
 *
 *  Created on: 9 mar 2014
 *      Author: slawek
 */

#ifndef TIMERS_H_
#define TIMERS_H_

/*
 * Licznik bieżącego "położenia" przemieszczających się światełek
 */
extern volatile uint8_t c_red, c_green, c_blue;

extern void init_soft_timers();
extern void init_semaphore();
extern void semaphore_lock();
extern void semaphore_wait();

#endif /* TIMERS_H_ */
