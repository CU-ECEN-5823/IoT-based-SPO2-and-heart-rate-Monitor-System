/* This header is a header file for the scheduler - scheduler.c
 * scheduler.h
 *
 *  Created on: 15 Sep 2021
 *      Author: nihalt
 */

#ifndef SRC_SCHEDULER_H_
#define SRC_SCHEDULER_H_

#include "app.h"
#include "em_core.h"
#include <stdint.h>
#include "ble.h"

void createEventI2CTransfer();
void createEventTimerWaitUs_IRQ(); // Function for creating an event to say that the timer is up
void createEventMeasureTempSi7021(); // Creating an event to measure the temperature from Si7021 sensor
uint32_t nextEvent (); // Getting the next event from the event handler and passing the event to the calling function
//void state_machine_temp (uint32_t event); // Passing the event number sifted from the scheduler
void state_machine_temp (sl_bt_msg_t *evt); // Passing the pointer to the evt data structure

//extern enum eventList;

// This is an enum declared to identify and declare the list of events that are
// expected to happen
enum eventList {
  event_NoEvent = 0,                // No Event
  event_measureTempSi7021 = 1,      // UF IRQ
  event_timerWaitUS_IRQ = 2,        // COMP1 IRQ
  event_I2CTransfer_IRQ=3,          // I2C Transfer IRQ
  event_Error,                      // Error Handler if I2C Write or Read Fails
  num_Events
//  event_LEDON=2
};


typedef enum uint32_t
{
      state_Idle,
      state_SensorOn,
      state_I2CWriteComplete,
      state_I2CReadWait,
      state_I2CReadComplete,
      numberOfStates
} State_t;

#endif /* SRC_SCHEDULER_H_ */
