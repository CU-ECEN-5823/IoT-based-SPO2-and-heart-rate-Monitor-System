/***************************************************************************//**
 * @file
 * @brief Core application logic.
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 * Date:        08-07-2021
 * Author:      Dave Sluiter
 * Description: This code was created by the Silicon Labs application wizard
 *              and started as "Bluetooth - SoC Empty".
 *              It is to be used only for ECEN 5823 "IoT Embedded Firmware".
 *              The MSLA referenced above is in effect.
 *
 ******************************************************************************/


#include "app.h"

// Include logging for this file
#define INCLUDE_LOG_DEBUG 1
#include "src/log.h"
#include "src/oscillator.h"
#include "src/timers.h"
#include "src/irq.h"
#include "src/scheduler.h"
#include "src/i2c.h"

/*****************************************************************************
 * GLOBAL variable declarations
 *****************************************************************************/
//extern enum eventList eventList_1;

/*****************************************************************************
 * Application Power Manager callbacks
 *****************************************************************************/
#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)



bool app_is_ok_to_sleep(void)
{

  return APP_IS_OK_TO_SLEEP;

} // app_is_ok_to_sleep()



sl_power_manager_on_isr_exit_t app_sleep_on_isr_exit(void)
{

  return APP_SLEEP_ON_ISR_EXIT;

} // app_sleep_on_isr_exit()



#endif // defined(SL_CATALOG_POWER_MANAGER_PRESENT)




/**************************************************************************//**
 * Application Init.
 *****************************************************************************/
SL_WEAK void app_init(void)
{
  // Put your application 1-time init code here
  // This is called once during start-up.
  // Don't call any Bluetooth API functions until after the boot event.


  // Student Edit: Add a call to gpioInit() here
  
  // Setting the power manager when the energy modes are either 1 or 2
  if (LOWEST_ENERGY_MODE == 1 || LOWEST_ENERGY_MODE == 2)
  {
      sl_power_manager_add_em_requirement(LOWEST_ENERGY_MODE);
  }

  // Initializing the Oscillator
  oscillator_Init();

  // Initializing the Timer (LETIMER0)
  timer_Init();

  // Initializing the GPIO Pins
  gpioInit();

  // Initlializing the I2C transfer
  i2c_Init();

  // Initializing the Timer (LETIMER0) Interrupt
//  LETIMER0_IRQInit();

  // Initlializing the blue tooth events
  ble_Init();

  // Initializing the
  NVIC_ClearPendingIRQ(LETIMER0_IRQn); // We will clear all pending flags in the NVIC for LETIMER
  NVIC_EnableIRQ(LETIMER0_IRQn); // Enabling the NVIC IRQ

}


///*****************************************************************************
// * delayApprox(), private to this file.
// * A value of 3500000 is ~ 1 second. After assignment 1 you can delete or
// * comment out this function. Wait loops are a bad idea in general.
// * We'll discuss how to do this a better way in the next assignment.
// *****************************************************************************/
//static void delayApprox(int delay) // @suppress("Unused static function")
//{
//  volatile int i;
//
//  for (i = 0; i < delay; ) {
//      i=i+1;
//  }
//
//} // delayApprox()
//



/**************************************************************************//**
 * Application Process Action.
 *****************************************************************************/
SL_WEAK void app_process_action(void)
{

//// Assignment 3
////
//  // Initializing the event to 0
//  uint32_t event=0;
//
//  // Getting the event that is passed by the nextEvent() function
//  event = nextEvent();
//
////  switch (event)
////  {
////    case (event_measureTempSi7021):
////
////        sensorEnable();
////
//////        gpioLed1Toggle(); // For debugging process
////
////        timerWaitUs_blocking(80000);
////
////        i2c_Write();
////
////        LOG_INFO("\n%d : The Temperature is : %f C\n\n",(loggerGetTimestamp(), 1)); // Logs the temperature
////
////        sensorDisable();
////
////     break;
////  }
//
// // Assignment 4
//  state_machine_temp(event);

  LED_test_seq();
}


/**************************************************************************//**
 * Bluetooth stack event handler.
 * This overrides the dummy weak implementation.
 *
 * @param[in] evt Event coming from the Bluetooth stack.
 *
 * The code here will process events from the Bluetooth stack. This is the only
 * opportunity we will get to act on an event.
 *****************************************************************************/
  void sl_bt_on_event(sl_bt_msg_t *evt)
  {

//    // Assignment 5
//    // Passing the pointer to the sl_bt_msg_t data structure
//    state_machine_ble(evt);
//
//    // Some events require responses from our application code,
//    // and don’t necessarily advance our state machines.
//    // For assignment 5 uncomment the next 2 function calls
//    // handle_ble_event(evt); // put this code in ble.c/.h
//
//    // sequence through states driven by events
//    // state_machine(evt);    // put this code in scheduler.c/.h



  } // sl_bt_on_event()

