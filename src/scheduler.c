/*
 * scheduler.c
 *
 *  Created on: 15 Sep 2021
 *      Author: nihalt
 */

#include "scheduler.h"

// Include logging for this file
#define INCLUDE_LOG_DEBUG 1
#include "src/log.h"

/**************************************************************************//**
 * GLOBAL variable declaration
 *****************************************************************************/
uint32_t eventHandler=0;


/**************************************************************************//**
 * This is a state machine that is designed for measuring the temperature at
 * regular intervals.
 *
 * It contains 5 states and also an error state in case the I2C fails to
 * succeed. The failure of I2C will reset the states and restart from the
 * beginning.
 *
 * @param:
 *      no params
 *
 * @return:
 *      returns the event that is next in line
 *****************************************************************************/
void state_machine_temp (sl_bt_msg_t *evt)
{

  // Changed for Assignment 5
  // Since the evt is a pointer, we will drill into the data structure to get the event code
  int32_t event = evt->data.evt_system_external_signal.extsignals, sc=0;

  ble_data_struct_t *ble_data_ptr = getBleDataPtr();

  uint8_t htm_temperature_buffer[5]={0};
  uint8_t *p = &htm_temperature_buffer[1];
  uint32_t htm_temperature_flt;

  State_t currentState;
  static State_t nextState = state_Idle;

   currentState = nextState;

  switch (currentState)
  {
    /****************************State 1****************************/
    case state_Idle:
      nextState = state_Idle; // default
      if (event == event_measureTempSi7021) //When LETIMER_UF happens
      {
//          LOG_INFO("State1\n");

          sensorEnable();

//          timerWaitUs_blocking(80000); // For debugging purpose only

          timerWaitUs_IRQ(80000);
          nextState = state_SensorOn;
      }
      if (event == event_Error)
      {
          eventHandler = 0;
          nextState = state_Idle;
      }

    break;

    /****************************State 2****************************/
    case state_SensorOn:
      nextState = state_SensorOn; // default
//      if (1)
      if (event == event_timerWaitUS_IRQ) //When LETIMER_COMP1 happens
      {
//          LOG_INFO("State2\n");

          sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);

          i2c_Write();
          nextState = state_I2CWriteComplete;
      }
      if (event == event_Error)
      {
          eventHandler = 0;
          nextState = state_Idle;
      }

    break;

    /****************************State 3****************************/
    case state_I2CWriteComplete:
      nextState = state_I2CWriteComplete; // default

      if (event == event_I2CTransfer_IRQ) //When LETIMER_COMP1 happens
      {
//          LOG_INFO("State3\n");
//          timerWaitUs_blocking(10800); // For debugging purpose only
          sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1);


          timerWaitUs_IRQ(10800);


          nextState = state_I2CReadWait;
      }
      if (event == event_Error)
      {
          eventHandler = 0;
          nextState = state_Idle;
      }

    break;

    /****************************State 4****************************/
    case state_I2CReadWait:
      nextState = state_I2CReadWait; // default

      if (event == event_timerWaitUS_IRQ) //When LETIMER_COMP1 happens
      {
//          LOG_INFO("State4\n");
          i2c_Read();
          sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);
          nextState = state_I2CReadComplete;
      }
      if (event == event_Error)
      {
          eventHandler = 0;
          nextState = state_Idle;
      }

    break;

    /****************************State 5****************************/
    case state_I2CReadComplete:
      nextState = state_I2CReadComplete; // default

      if (event == event_I2CTransfer_IRQ) //When LETIMER_COMP1 happens
      {
          sensorDisable();
          sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1);
//          LOG_INFO("State5\n");

          float temperature_32 = (convertToDegrees (concatenatingBytes(2),'C'));


          htm_temperature_flt = UINT32_TO_FLOAT(temperature_32*1000, -3);

          UINT32_TO_BITSTREAM(p, htm_temperature_flt);

          LOG_INFO("The present temperature is: %f C",(float)temperature_32);


          // We will send indication only if there is an active connection and indication is enabled
          if ((ble_data_ptr->flag_indication == true) && (ble_data_ptr->flag_conection == true))
          {
              // Sending indication
              sc = sl_bt_gatt_server_send_indication(evt->data.evt_connection_opened.connection,
                                                     gattdb_temperature_measurement,
                                                     sizeof(htm_temperature_buffer),
                                                     htm_temperature_buffer);
//              LOG_INFO("Indication Sent");
              ble_data_ptr->flag_indication_in_progress = true;

              // Printing the error message if the Sending Indication fails
              if (sc != 0)
                LOG_ERROR("!!! Sending Indication Failed !!!\nError Code: 0x%x",sc);

              // Writing the measurement type
              sc = sl_bt_gatt_server_write_attribute_value(gattdb_temperature_type,
                                                           0,
                                                           sizeof(htm_temperature_buffer[0]),
                                                           p);

              // Printing the error message if the Server Write Failed fails
              if (sc != 0)
                LOG_ERROR("!!! Server Write Failed !!!\nError Code: 0x%x",sc);
          }

          nextState = state_Idle;
      }
      if (event == event_Error)
      {
          eventHandler = 0;
          nextState = state_Idle;
      }

    break;

    /**************************Default State**************************/
    default:
    break;
  } // switch
} // state_machine()



/**************************************************************************//**
 * This function identifies the event next in line and passes it to the calling
 * function
 *
 * This code will run only when the event occurs and the interrupt flag is
 * raised
 *
 * @param:
 *      no params
 *
 * @return:
 *      returns the event that is next in line
 *****************************************************************************/
uint32_t nextEvent () {
  uint32_t i=0;
  for (i=0; i<=num_Events ;i++)
  {
      if (eventHandler & (1<<(i-1)))
      {
          CORE_DECLARE_IRQ_STATE;

          CORE_ENTER_CRITICAL();

          eventHandler = (eventHandler & (~(1<<(i-1))));

          eventHandler=0;

          CORE_EXIT_CRITICAL();
//          int32_t returning = i; // For the purpose of debugging
          return i;
      }
  }
//  int32_t returning=0; // For the purpose of debugging
  return 0;
}


/**************************************************************************//**
 * This function creates an event in the event handler. An event is set by
 * setting the corresponding bit to 1. The bit number is the same order as
 * of eventList.
 *
 * This is to measure the temperature (for UF event)
 *
 * @param:
 *      no params
 *
 * @return:
 *      no return
 *****************************************************************************/
void createEventMeasureTempSi7021()
{
    CORE_DECLARE_IRQ_STATE;

    CORE_ENTER_CRITICAL();

//    eventHandler = (eventHandler | (1<<(event_measureTempSi7021-1))); // setting the first bit to 1

    sl_bt_external_signal(event_measureTempSi7021);

    CORE_EXIT_CRITICAL();

//    LOG_INFO("EVENT 1 SET");
}

/**************************************************************************//**
 * This function creates an event in the event handler. An event is set by
 * setting the corresponding bit to 1. The bit number is the same order as
 * of eventList.
 *
 * This is to say that the timer is up for TimerWaitUS (for COMP1 event)
 *
 * @param:
 *      no params
 *
 * @return:
 *      no return
 *****************************************************************************/
void createEventTimerWaitUs_IRQ()
{
    CORE_DECLARE_IRQ_STATE;

    CORE_ENTER_CRITICAL();

//    eventHandler = (eventHandler | (1<<(event_timerWaitUS_IRQ-1))); // setting the first bit to 1

    sl_bt_external_signal(event_timerWaitUS_IRQ);

    CORE_EXIT_CRITICAL();

//    LOG_INFO("EVENT 2 SET");
}

/**************************************************************************//**
 * This function creates an event in the event handler. An event is set by
 * setting the corresponding bit to 1. The bit number is the same order as
 * of eventList.
 *
 * This is to say that the timer is up for TimerWaitUS (for COMP1 event)
 *
 * @param:
 *      no params
 *
 * @return:
 *      no return
 *****************************************************************************/
void createEventI2CTransfer()
{
    CORE_DECLARE_IRQ_STATE;

    CORE_ENTER_CRITICAL();

//    eventHandler = (eventHandler | (1<<(event_I2CTransfer_IRQ-1))); // setting the first bit to 1

    sl_bt_external_signal(event_I2CTransfer_IRQ);

    CORE_EXIT_CRITICAL();

//    LOG_INFO("I2CWrite Event");
}

//void schedulerSetLEDON()
//{
//  CORE_DECLARE_IRQ_STATE;
//
//  CORE_ENTER_CRITICAL();
//
//  eventHandler = (eventHandler | (1<<(event_LEDON-1)));
//
//  CORE_EXIT_CRITICAL();
//
////  gpioLed0SetOn();
////  nextEvent ();
//}
//
//
//void schedulerSetLEDOFF()
//{
//  CORE_DECLARE_IRQ_STATE;
//
//  CORE_ENTER_CRITICAL();
//
//  eventHandler = (eventHandler | (1<<(event_LEDOFF-1)));
//
//  CORE_EXIT_CRITICAL();
//
////  gpioLed0SetOff();
////  nextEvent ();
//}



