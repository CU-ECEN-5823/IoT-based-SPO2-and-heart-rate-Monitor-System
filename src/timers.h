/* This function initializes the timers - timers.c
 * timers.h
 *
 *  Modified on: 8 Dec 2021
 *      Author:
 *          Author 1: Nihal T
 *          Author 2: Sudarshan J
 *
 */

#ifndef SRC_TIMERS_H_
#define SRC_TIMERS_H_
#include "em_letimer.h"
#include "em_cmu.h"
#include "gpio.h"
#include <stdint.h>

#define LETIMER_nBITS (16)

// Function prototypes
void oscillatorSelect();
void preScaller();
uint32_t ticksToLoad(const uint32_t time_ms);
void timer_Init();
int32_t settingPrescalerValue(const int time_ms);
void timerWaitUs_blocking(uint32_t us_wait);
void timerWaitUs_IRQ(uint32_t us_wait);


#endif /*SRC_TIMERS_H_*/
