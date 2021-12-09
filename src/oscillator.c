/*
 * oscillator.c
 *
 *  Modified on: 8 Dec 2021
 *      Author:
 *          Author 1: Nihal T
 *          Author 2: Sudarshan J
 *
 */


#include "oscillator.h"
#include "app.h"

// Include logging for this file
#define INCLUDE_LOG_DEBUG 1
#include "src/log.h"

/**************************************************************************//**
 * This function initializes the oscillators
 *
 * For energy modes EM0,EM1 and EM2 we will set up Low Frequency Crystal
 * Oscillator (ULFRCO)
 *
 * For energy modes EM3 we will set up Ultra-Low Frequency RC Oscillator (LFXO)
 *
 * Refer to the data sheet for the details listed below
 *****************************************************************************/
void oscillator_Init()
{
//  int32_t x, frequency;
  if (LOWEST_ENERGY_MODE == 3)
  {
      CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_ULFRCO); // Selecting the ULFRCO from the LFA clock branch
      //CMU_ClockSelectGet(cmuClock_LFA);
      CMU_OscillatorEnable(cmuOsc_ULFRCO, true, true); // Enabling the above ULFRCO clock
  }
  else if ((LOWEST_ENERGY_MODE < 3) && (LOWEST_ENERGY_MODE >= 0))
  {
      CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFXO); // Selecting the LFXO from the LFA clock branch
      //CMU_ClockSelectGet(cmuClock_LFA);
      CMU_OscillatorEnable(cmuOsc_LFXO, true, true); // Enabling the above LFXO clock
  }
  CMU_ClockEnable(cmuClock_LFA, true); // Enabling the LFA clock (master branch of the above 2)

  CMU_ClockEnable(cmuClock_LETIMER0, true); // Enabling the LETIMER0 clock

//  CMU_ClockPrescSet(cmuClock_LFA, 4);
  preScaller(LETIMER_PERIOD_MS); // Pre-scaling the timer

  //x=CMU_ClockPrescGet(cmuClock_LFA);


//  // See if we have the right frequency
//  frequency = CMU_ClockFreqGet (cmuClock_LFA);
////  LOG_INFO("LFA clock freq = %d", (int) frequency);
//  frequency = CMU_ClockFreqGet (cmuClock_LETIMER0);
////  LOG_INFO("LETIMER0 clock freq = %d", (int) frequency);
}
