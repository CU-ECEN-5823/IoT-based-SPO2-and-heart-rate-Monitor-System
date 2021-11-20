/* This header is a header file for interrupt service routines - irq.c
 * iqr.h
 *
 *  Created on: 10 Sep 2021
 *      Author: nihalt
 */

#ifndef SRC_IRQ_H_
#define SRC_IRQ_H_
#include "em_letimer.h"

// Function prototypes
//void LETIMER0_IRQInit(); // Initializing the LETIMER0 IRQ routine
void LETIMER0_IRQHandler(); // LETIMER0 IRQ handler
uint32_t letimerMilliseconds(); // Gives the milliseconds since last boot
void I2C0_IRQHandler(void); // I2C0 IRQ handler


#endif /* SRC_IRQ_H_ */
