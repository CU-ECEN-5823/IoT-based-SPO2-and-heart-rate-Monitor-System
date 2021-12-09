/* This header is a header file for interrupt service routines - irq.c
 * iqr.h
 *
 *  Modified on: 8 Dec 2021
 *      Author:
 *          Server Code: Nihal T
 *          Client Code: Sudarshan J
 */

#ifndef SRC_IRQ_H_
#define SRC_IRQ_H_
#include "em_letimer.h"

// Function prototypes
//void LETIMER0_IRQInit();                 // Initializing the LETIMER0 IRQ routine
void LETIMER0_IRQHandler();                // LETIMER0 IRQ handler
uint32_t letimerMilliseconds();            // Gives the milliseconds since last boot
void I2C0_IRQHandler(void);                // I2C0 IRQ handler
void GPIO_EVEN_IRQHandler(void);           // Even Pins GPIO handler
void GPIO_ODD_IRQHandler(void);            // Odd Pins GPIO handler
void NVIC_Init(void);                      // Initializes all the required NVICs
void I2C_event(void);


#endif /* SRC_IRQ_H_ */
