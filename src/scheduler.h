/* This header is a header file for the scheduler - scheduler.c
 * scheduler.h
 *
 *  Modified on: 8 Dec 2021
 *      Author:
 *          Author 1: Nihal T
 *          Author 2: Sudarshan J
 *
 */

#ifndef SRC_SCHEDULER_H_
#define SRC_SCHEDULER_H_

#include "app.h"
#include "em_core.h"
#include <stdint.h>
#include "ble.h"
#include "lcd.h"

void createEventI2CTransfer();
void createEventTimerWaitUs_IRQ();                    // Function for creating an event to say that the timer is up
void createEventMeasureHRMAX30101();                  // Creating an event to measure the temperature from Si7021 sensor
uint32_t nextEvent ();                                // Getting the next event from the event handler and passing the event to the calling function
//void state_machine_temp (uint32_t event);           // Passing the event number sifted from the scheduler
void state_machine_hr (sl_bt_msg_t *evt);             // Passing the pointer to the evt data structure
void state_machine_discovery (sl_bt_msg_t *evt);      // Passing the pointer to the evt data structure
void createEventErrorTemp();                          // Creating an event to handle the error cases
void createEventPB0Pressed();                         // Creating an event to handle the Push Button 0 event
void createEventPB1Pressed();                         // Creating an event to handle the Push Button 1 event
void createEventSystemError();
void createEventMAX30101Int();


// Definitions for CB FIFO
size_t cbfifo_enqueue(void *buf, size_t nbyte);       // Enqueue nbytes from the buf array
size_t cbfifo_dequeue(void *buf, size_t nbyte);       // Dequeue nbytes from the buf array
size_t cbfifo_length();                               // Length of the buffer that has data in it
size_t cbfifo_capacity();                             // Capacity of the buffer


/*capacity of the static buffer is 176 bytes
 * 2 bytes for the characteristic +
 * 4 bytes for the number of bytes to transfer +
 * 5 bytes for the buffer (5 bytes for temperature and 1 byte for button state)*/
#define ARRAY_CAPACITY 176
#define ZERO 0
#define ONE 1


//extern enum eventList;


// This is an enum declared to identify and declare the list of events that are
// expected to happen
//enum eventList_temp {
//  event_NoEvent_temp = 0,                // No Event
//  event_measureTempSi7021_temp = 1,      // UF IRQ
//  event_timerWaitUS_IRQ_temp = 2,        // COMP1 IRQ
//  event_I2CTransfer_IRQ_temp = 3,        // I2C Transfer IRQ
//  event_PB0Pressed_temp,                 // Push Button 0 is pressed
//  event_PB1Pressed_temp,                 // Push Button 1 is pressed
//  event_Error_temp,                      // Error Handler if I2C Write or Read Fails
//  num_Events_temp                        // Number of Events
////  event_LEDON=2
//};


enum eventList_hr {
  event_NoEvent_hr = 0,                   // No Event
  event_timerWaitUS_IRQ_hr,
  event_measureMAX30101_hr,
  event_bufferFullMAX30101_hr,
  event_I2CTransfer_IRQ_hr,
  event_PB0Pressed_hr,                 // Push Button 0 is pressed
  event_PB1Pressed_hr,                 // Push Button 1 is pressed
  event_SystemError_hr,
  num_Events_hr
//  event_LEDON=2
};

typedef enum
{
      state_Idle_hr,                      // Idle State
      state_Init_hr
} State_t_hr;


// All the discovery events are handled by the BLE stack and so there is no need to create an external structure for it
enum eventList_disc {
  event_NoEvent_disc = 0,                // No Event
  event_Error_disc,                      // Error Handler if I2C Write or Read Fails
  num_Events_disc                        // Number of Events
};


typedef enum
{
      state_Idle_disc,                   // Idle State
      state_Service_hr_disc,                // Service by UUID State
      state_Characteristic_hr_disc,         // Characteristic by UUID State
      state_Indication_hr_disc,
      state_Service_hr_led_disc,
      state_Characteristic_hr_led_disc,
      state_Indication_wait_disc
} State_t_disc;


#endif /* SRC_SCHEDULER_H_ */
