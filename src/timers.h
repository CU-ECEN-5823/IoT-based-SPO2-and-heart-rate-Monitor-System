/* This function initializes the timers - timers.c
   timers.h

    Created on: 7 September 2021
        Author: Nihal T
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
