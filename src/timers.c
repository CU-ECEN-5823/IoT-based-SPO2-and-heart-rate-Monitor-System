/*
 * timers.c
 *
 *  Modified on: 8 Dec 2021
 *      Author:
 *          Author 1: Nihal T
 *          Author 2: Sudarshan J
 *
 */

#include "timers.h"
#include "app.h"

#include <stdio.h>


// Include logging for this file
#define INCLUDE_LOG_DEBUG 1
#include "src/log.h"

// This part was made dynamic in Assignment 3 - Start
//// If the energy modes are anything other than 3 then the 16 bit counter register will not be sufficient and so we will need a pre-scaler
//#if LOWEST_ENERGY_MODE == 3
//#define preScallerValue (1)
//#else
//#define preScallerValue (4)
//#endif
// This part was made dynamic in Assignment 3 - Stop

static uint32_t preScallerValue=1;

/**************************************************************************//**
 * This function takes in time (in milliseconds) and returns the number of
 * machine cycles that the controller should run to get the desired time
 * @param:
 *      time (in milli-seconds)
 * @return:
 *      the number of machine cycles to run to get the above time
 *****************************************************************************/
uint32_t ticksToLoad(const uint32_t time_ms)
{
//  uint32_t val_timer = CMU_ClockFreqGet(cmuClock_LETIMER0);
  uint32_t x =  ((time_ms*CMU_ClockFreqGet(cmuClock_LETIMER0))/1000); // Warning: (time_ms/1000)*CMU_ClockFreqGet(cmuClock_LETIMER0) will return a rounded off time
  return x;
}

/**************************************************************************//**
 * This function takes in time (in milliseconds) and returns the pre-scaler
 * in order to accommodate the counting into a 16 bit register without overflow
 *
 * @param:
 *      time_ms (in milli-seconds)
 * @return:
 *      pre-scaler for the timer
 *****************************************************************************/
int32_t settingPrescalerValue(const int time_ms)
{
  uint32_t req_ticks = 0, max_ticks_of_LETIMER=0;

  req_ticks = (CMU_ClockFreqGet(cmuClock_LETIMER0)*time_ms)/1000;

  max_ticks_of_LETIMER = 1<<(LETIMER_nBITS-1); // LETIMER is a 16 bit counter so the max value it can take is 2^15

  while ((req_ticks/preScallerValue)>max_ticks_of_LETIMER)
  {
      preScallerValue <<= 1;
  }
//  int32_t val = preScallerValue;
  return preScallerValue;
}


/**************************************************************************//**
 * This function scales the LETIMER0 based on the scaler values that was earlier
 * defined in the macro
 * @param:
 *      no params
 * @return:
 *      no return value
 *****************************************************************************/
void preScaller(const int time_ms)
{
  //int32_t beforeScaller=CMU_ClockFreqGet(cmuClock_LETIMER0);
  CMU_ClockDivSet(cmuClock_LETIMER0, settingPrescalerValue(time_ms));
//  int32_t scallerValue=CMU_ClockDivGet(cmuClock_LETIMER0);
//  int32_t afterScaller=CMU_ClockFreqGet(cmuClock_LETIMER0);
}


/**************************************************************************//**
 * This function takes in time (in micro-seconds) and creates a hard delay of
 * the passed time
 *
 * @param:
 *      time (in micro-seconds)
 * @return:
 *      no return
 *****************************************************************************/
void timerWaitUs_blocking(uint32_t us_wait)
{
    uint32_t getCurrentCount = LETIMER_CounterGet(LETIMER0); // Getting the current CNT of the timmer
  //  int32_t us = us_wait;

    uint32_t us_ticks = ticksToLoad(us_wait/1000); // The number of ticks that need to be load in order to delay by the passed in parameter
    uint32_t max_tick=LETIMER_CompareGet(LETIMER0,0); // Finding the maximum number of ticks that can be counted by the timer
//    uint32_t testcurrent = LETIMER_CounterGet(LETIMER0); // For debugging purpose

    uint32_t temp = us_ticks;

    // When the desired ticks are greater than the max possible tick we will free run for n cycles until the ticks reach bellow max_ticks
    while (temp>max_tick)
    {
        while(LETIMER_CounterGet(LETIMER0)!=(getCurrentCount+1));
        temp -= max_tick;
    }

    // If the current count can accommodate the ticks required then simply run till current-required
    if (temp<getCurrentCount)
    {
        while(LETIMER_CounterGet(LETIMER0)!=(getCurrentCount-temp));

    }
    // If the current count cannot accommodate the ticks required then simply run till max_tick-(current-required) - more like wrap around
    // NOTE: this will work only till max_ticks range so the first wrap around for complete cycles is required to get it below the max_ticks range
    else
    {
        while(LETIMER_CounterGet(LETIMER0)!=((max_tick)-(temp-getCurrentCount)));

    }
}



/**************************************************************************//**
 * This function takes in time (in micro-seconds) and creates a interrupt
 * based delay
 *
 * @param:
 *      time (in micro-seconds)
 * @return:
 *      no return
 *****************************************************************************/
void timerWaitUs_IRQ(uint32_t us_wait)
{
  uint32_t getCurrentCount = LETIMER_CounterGet(LETIMER0); // Getting the current CNT of the timmer
//  int32_t us = us_wait;

    uint32_t max_tick=LETIMER_CompareGet(LETIMER0,0); // Finding the maximum number of ticks that can be counted by the timer before UF happens (In our case its 3 seconds)
    //  LETIMER_CompareGet gives number of ticks remaining to reach 0 sum of current + remining will give total

    uint32_t us_ticks = ticksToLoad(us_wait/1000); // The number of ticks that need to be load in order to delay by the passed in parameter
//    uint32_t testcurrent = LETIMER_CounterGet(LETIMER0); // For debugging purpose

    uint32_t temp = us_ticks;

    // Desired ticks should be lesser than the max ticks possible
    if (temp>max_tick)
    {
        LOG_ERROR("\nThe wait time is beyond the allowed range of time");

    }
    // If the current count can accommodate the ticks required then simply run till current-required
    else if (temp<getCurrentCount)
    {
//        int32_t val = getCurrentCount-temp;
//        int32_t val_1 = LETIMER_CounterGet(LETIMER0);


        LETIMER_CompareSet(LETIMER0, 1, (getCurrentCount-temp)); // Setting the desired time period as COMP1
//        LOG_INFO("Start Timmer");

        LETIMER_IntClear(LETIMER0, LETIMER_IF_COMP1); // By default the COMP1 will be set to 0 so UF and COMP1 will get triggered. When enabling the timer may not wait and will give an interrupt immediately. So better to clear it.
        LETIMER_IntEnable(LETIMER0, LETIMER_IEN_COMP1);
//        while(LETIMER_CounterGet(LETIMER0)!=(getCurrentCount-temp));
    }
    // If the current count cannot accommodate the ticks required then simply run till max_tick-(current-required) - more like wrap around
    // NOTE: this will work only till max_ticks range so the first wrap around for complete cycles is required to get it below the max_ticks range
    else // Only this section of the code will have a little extra delay only because of the UF and reload action
    {
//        int32_t val = ((max_tick)-(temp-getCurrentCount));
//        int32_t val_1 = LETIMER_CounterGet(LETIMER0);
        LETIMER_CompareSet(LETIMER0, 1, ((max_tick)-(temp-getCurrentCount))); // Setting the desired time period as COMP1


//        LOG_INFO("Start Timmer");
        LETIMER_IntClear(LETIMER0, LETIMER_IF_COMP1); // By default the COMP1 will be set to 0 so UF and COMP1 will get triggered. When enabling the timer may not wait and will give an interrupt immediately. So better to clear it.
        LETIMER_IntEnable(LETIMER0, LETIMER_IEN_COMP1);
//        while(LETIMER_CounterGet(LETIMER0)!=((max_tick)-(temp-getCurrentCount)));

    }
}

/**************************************************************************//**
 * This function initializes and enables the LETIMER0
 * @param:
 *      no params
 * @return:
 *      no return value
 *****************************************************************************/
void timer_Init()
{
  // Creating a typedef of the default settings
  // Other parameters like run until SW stops, load comp0 on underflow etc are all the same as default
  LETIMER_Init_TypeDef init = LETIMER_INIT_DEFAULT;

  init.comp0Top= true; // Changing the comp0Top to true so that the comp0 value is loaded into the counter when it starts to run

  LETIMER_Init(LETIMER0, &init); // Initializing the counter by passing the typedef that was created and passing address of it to the LETIMER0 Register

  // This code will be run only if the temperature state machine is required to run if we have an active connection and the indication are ON. Set NOP_INDICATION_CONNECTION in app.h
#if NOP_INDICATION_CONNECTION == 1
  LETIMER_IntDisable(LETIMER0, LETIMER_IEN_UF);
#else
  LETIMER_IntEnable(LETIMER0, LETIMER_IEN_UF);
#endif

  // Setting the COMP0 and COMP1 registers so that we can use them as interrupts
  LETIMER_CompareSet(LETIMER0, 0, ticksToLoad(LETIMER_PERIOD_MS)); // Setting the total period (ON time+OFF time) as COMP0
//  LETIMER_CompareSet(LETIMER0, 1, ticksToLoad(LETIMER_ON_TIME_MS)); // Setting the ON time period as COMP1
//  int32_t test=LETIMER_CompareGet(LETIMER0, 1);

  LETIMER_Enable(LETIMER0, true); // Enabling the the LETIMERO to start running
}

