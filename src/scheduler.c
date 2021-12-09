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
#include <string.h>
#include "algo.h"
#include "autocorrelate.h"

/**************************************************************************//**
 * GLOBAL variable declaration
 *****************************************************************************/
uint32_t eventHandler=0;

uint8_t cbfifo_array[ARRAY_CAPACITY];
uint8_t *write = cbfifo_array;
uint8_t *read = cbfifo_array;
uint8_t capacity_full = 0;

#define MASTER_BUFFER (31*10)
#define FINGER_PRESS_BUFFER (3)

uint32_t hr_buffer[MASTER_BUFFER];
uint32_t *hr_buffer_ptr = hr_buffer;

uint32_t finger_press[FINGER_PRESS_BUFFER];

uint32_t calc_hr, heart_rate = 0, count = 0;

/**************************************************************************//**
 * This is a state machine that is designed for measuring the temperature at
 * regular intervals.
 *
 * It contains 5 states and also an error state in case the I2C fails to
 * succeed. The failure of I2C will reset the states and restart from the
 * beginning.
 *
 * Note: When any of the events fail then we will do a standard reset of the
 * controller. This will clear up the stack and reset the state informations
 * too.
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

  uint8_t htm_temperature_buffer[5]={0}, cb_buffer_load[11]={0};
  uint8_t *p = &htm_temperature_buffer[1];
  uint32_t htm_temperature_flt;

  State_t_temp currentState;
  static State_t_temp nextState = state_Idle_temp;

  currentState = nextState;


  // DOS your state machine is indexing into that BT stack union for all events
  // passed to your state machine and you are failing to check for
  // event == sl_bt_evt_system_external_signal_id first !!!
  // evt->data.evt_system_external_signal.extsignals is only valid for
  // sl_bt_evt_system_external_signal_id
  if (SL_BT_MSG_ID(evt->header) != sl_bt_evt_system_external_signal_id) {
      return;
  }


  switch (currentState)
  {
    /****************************State 1****************************/
    case state_Idle_temp:
      nextState = state_Idle_temp; // default
      if (event == event_measureTempSi7021_temp) //When LETIMER_UF happens
      {
//          LOG_INFO("State1\n");

          sensorEnable();

//          timerWaitUs_blocking(80000); // For debugging purpose only

          timerWaitUs_IRQ(80000);
          nextState = state_SensorOn_temp;
      }
      if (event == event_Error_temp)
      {
//          sl_bt_connection_close(ble_data_ptr->connectionHandle);
          sl_bt_system_reset(sl_bt_system_boot_mode_normal);
          nextState = state_Idle_temp;
      }
    break;

    /****************************State 2****************************/
    case state_SensorOn_temp:
      nextState = state_SensorOn_temp; // default
//      if (1)
      if (event == event_timerWaitUS_IRQ_temp) //When LETIMER_COMP1 happens
      {
//          LOG_INFO("State2\n");

          sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);

          i2c_Write();
          nextState = state_I2CWriteComplete_temp;
      }
      if (event == event_Error_temp)
      {
//          sl_bt_connection_close(ble_data_ptr->connectionHandle);
          sl_bt_system_reset(sl_bt_system_boot_mode_normal);
          nextState = state_Idle_temp;
      }

    break;

    /****************************State 3****************************/
    case state_I2CWriteComplete_temp:
      nextState = state_I2CWriteComplete_temp; // default

      if (event == event_I2CTransfer_IRQ_temp) //When LETIMER_COMP1 happens
      {
//          LOG_INFO("State3\n");
//          timerWaitUs_blocking(10800); // For debugging purpose only
          sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1);

//          // To induce error
//          if(letimerMilliseconds()>10000 && letimerMilliseconds()<20000)
//            timerWaitUs_IRQ(1080);
//          else
//            timerWaitUs_IRQ(10800); // Actual

          timerWaitUs_IRQ(10800);

          nextState = state_I2CReadWait_temp;
      }
      if (event == event_Error_temp)
      {
//          sl_bt_connection_close(ble_data_ptr->connectionHandle);
          sl_bt_system_reset(sl_bt_system_boot_mode_normal);
          nextState = state_Idle_temp;
      }

    break;

    /****************************State 4****************************/
    case state_I2CReadWait_temp:
      nextState = state_I2CReadWait_temp; // default

      if (event == event_timerWaitUS_IRQ_temp) //When LETIMER_COMP1 happens
      {
//          LOG_INFO("State4\n");
//          i2c_Read(); // Shouldn't be here
          sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);

          i2c_Read();
          // DOS needs to be BEFORE you initiate the I2C transfer!!!!
          //sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);


          nextState = state_I2CReadComplete_temp;
      }
      if (event == event_Error_temp)
      {
//          sl_bt_connection_close(ble_data_ptr->connectionHandle);
          sl_bt_system_reset(sl_bt_system_boot_mode_normal);
          nextState = state_Idle_temp;
      }

    break;

    /****************************State 5****************************/
    case state_I2CReadComplete_temp:
      nextState = state_I2CReadComplete_temp; // default

      if (event == event_I2CTransfer_IRQ_temp) //When LETIMER_COMP1 happens
      {
//          sensorDisable(); // Commented this for the LCD output
          sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1);
//          LOG_INFO("State5\n");

          float temperature_32 = (convertToDegrees (concatenatingBytes(2),'C'));


          htm_temperature_flt = UINT32_TO_FLOAT(temperature_32*1000, -3);

          UINT32_TO_BITSTREAM(p, htm_temperature_flt);

          LOG_INFO("The present temperature is: %f C",(float)temperature_32);

//          displayPrintf(DISPLAY_ROW_TEMPVALUE, "");

          if (ble_data_ptr->flag_conection == true &&
              ble_data_ptr->flag_indication_hr_led == true &&
              ble_data_ptr->flag_bonded == true &&
              ble_data_ptr->flag_indication_in_progress == true &&
              cbfifo_length() == cbfifo_capacity())
           {
             LOG_ERROR("Buffer Full!! This indication will not be stored and will be lost.");
           }
          // If indications are in flight then we will execute the below
          else if ((ble_data_ptr->flag_indication_hr == true) &&
              (ble_data_ptr->flag_conection == true) &&
              (ble_data_ptr->flag_indication_in_progress == false) &&
              cbfifo_length() != cbfifo_capacity())
          {
              // Sending indication
              sc = sl_bt_gatt_server_send_indication(ble_data_ptr->connectionHandle,
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

              displayPrintf(DISPLAY_ROW_TEMPVALUE, "%d C", (int)temperature_32);
          }
          else if ((ble_data_ptr->flag_indication_hr == true) &&
                   (ble_data_ptr->flag_conection == true) &&
                   (ble_data_ptr->flag_indication_in_progress == true))
          {
//              printf("%x\n%x\n%x\n",gattdb_temperature_type, sizeof(htm_temperature_buffer), htm_temperature_buffer[1]);
              cb_buffer_load[0] = (uint8_t) ((gattdb_temperature_type >> 8) & 0x00FF);
              cb_buffer_load[1] = (uint8_t) ((gattdb_temperature_type >> 0) & 0x00FF);
              cb_buffer_load[2] = (uint8_t) ((sizeof(htm_temperature_buffer) >> 24) & 0x000000FF);
              cb_buffer_load[3] = (uint8_t) ((sizeof(htm_temperature_buffer) >> 16) & 0x000000FF);
              cb_buffer_load[4] = (uint8_t) ((sizeof(htm_temperature_buffer) >> 8) & 0x000000FF);
              cb_buffer_load[5] = (uint8_t) ((sizeof(htm_temperature_buffer) >> 0) & 0x000000FF);
              cb_buffer_load[6] = htm_temperature_buffer[0];
              cb_buffer_load[7] = htm_temperature_buffer[1];
              cb_buffer_load[8] = htm_temperature_buffer[2];
              cb_buffer_load[9] = htm_temperature_buffer[3];
              cb_buffer_load[10] = htm_temperature_buffer[4];

//              // For debugging purpose only
//              printf("\nOriginal\n%d\t%d\t%d\t%d\t%d\n%d\n%d\n\n", cb_buffer_load[6],
//                     cb_buffer_load[7],
//                     cb_buffer_load[8],
//                     cb_buffer_load[9],
//                     cb_buffer_load[10],
//                     sizeof(htm_temperature_buffer),
//                     gattdb_temperature_type);
//
//              printf("%d", (sizeof(cb_buffer_load)/sizeof(uint8_t))); // For debugging purpose only
              cbfifo_enqueue(cb_buffer_load, (sizeof(cb_buffer_load)/sizeof(uint8_t)));

              LOG_INFO("Temperature indication added to buffer : %d indications left in the buffer", (cbfifo_length()/11));
          }

          nextState = state_Idle_temp;
      }
      if (event == event_Error_temp)
      {
//          sl_bt_connection_close(ble_data_ptr->connectionHandle);
          sl_bt_system_reset(sl_bt_system_boot_mode_normal);
          nextState = state_Idle_temp;
      }

    break;

    /**************************Default State**************************/
    default:
    break;
  } // switch
} // state_machine()


/**************************************************************************//**
 * This is a state machine that is designed for measuring the temperature at
 * regular intervals.
 *
 * It contains 5 states and also an error state in case the I2C fails to
 * succeed. The failure of I2C will reset the states and restart from the
 * beginning.
 *
 * Note: When any of the events fail then we will do a standard reset of the
 * controller. This will clear up the stack and reset the state informations
 * too.
 *
 * @param:
 *      no params
 *
 * @return:
 *      returns the event that is next in line
 *****************************************************************************/
void state_machine_hr (sl_bt_msg_t *evt)
{

  // Changed for Assignment 5
  // Since the evt is a pointer, we will drill into the data structure to get the event code
  int32_t event = evt->data.evt_system_external_signal.extsignals, sc=0;

  ble_data_struct_t *ble_data_ptr = getBleDataPtr();

  uint8_t hrm_heartrate_buffer[5]={0}, cb_buffer_load[11]={0};
  uint8_t *p = &hrm_heartrate_buffer[1], finger_present, read, write_ptr, read_ptr;
  uint32_t hrm_heartrate_flt;

  State_t_hr currentState;
  static State_t_hr nextState = state_Idle_hr;

  currentState = nextState;

//  printf("%d, %d\n",event, currentState);
  // DOS your state machine is indexing into that BT stack union for all events
  // passed to your state machine and you are failing to check for
  // event == sl_bt_evt_system_external_signal_id first !!!
  // evt->data.evt_system_external_signal.extsignals is only valid for
  // sl_bt_evt_system_external_signal_id
  if (SL_BT_MSG_ID(evt->header) != sl_bt_evt_system_external_signal_id) {
      return;
  }


  switch (currentState)
  {
    /****************************State 1****************************/
    case state_Idle_hr:
      nextState = state_Idle_hr; // default
      if (event == event_measureMAX30101_hr) //When LETIMER_UF happens
      {
          sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);
          MAX_30101_Init();

          sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1);

//          printf("\nState 1 : Powered Up\n");

          nextState = state_Init_hr;
      }
      if (event == event_SystemError_hr)
      {
          nextState = state_Idle_hr;
      }
    break;

    /****************************State 2****************************/
    case state_Init_hr:
      nextState = state_Init_hr; // default

      if (event == event_bufferFullMAX30101_hr) //When LETIMER_UF happens
      {
//          printf("State 2 : Buffer full clear it\n");
          sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);
          i2c_Write_Read_blocking(0x06, &read_ptr, sizeof(read_ptr));

          i2c_Write_Read_blocking(0x04, &write_ptr, sizeof(write_ptr));

          int8_t data_to_read = (write_ptr-read_ptr);

          if (data_to_read<0)
            data_to_read = (32 + data_to_read);



//          printf("\nInterrupt Hit. The difference is : %d\n", (data_to_read));

          for (int i = 0; i<(data_to_read); i++)
          {
            uint8_t result[3]={0};

            i2c_Write_Read_blocking(0x07, result, sizeof(result));

            int32_t reading = ((uint32_t)result[0]<<16 | (uint32_t)result[1]<<8 | (uint32_t)result[2]);

            *(hr_buffer_ptr) = reading;

//            i2c_Write_Read_blocking(0x06, &read_ptr, sizeof(read_ptr));
//            i2c_Write_Read_blocking(0x04, &write_ptr, sizeof(write_ptr));
//
//            printf("\nRd : %d\t Wr : %d\n", read_ptr, write_ptr);

            hr_buffer_ptr++;
          }
//          i2c_Write_Read_blocking(0x06, &read_ptr, sizeof(read_ptr));
//          i2c_Write_Read_blocking(0x04, &write_ptr, sizeof(write_ptr));
//
//          printf("\nRd : %d\t Wr : %d\t Diff : %d\n", read_ptr, write_ptr, (hr_buffer_ptr - hr_buffer));

          if ((hr_buffer_ptr - hr_buffer) == MASTER_BUFFER)
          {
            hr_buffer_ptr = hr_buffer;

            calc_hr = ((12000*2)/(autocorrelate_detect_period(hr_buffer, MASTER_BUFFER, kAC_32bps_unsigned))) - (ble_data_ptr->factor);

            if (calc_hr == 0 || (0 <= calc_hr && calc_hr<=180))
              {
                heart_rate = calc_hr;
              }

            finger_press[(count%FINGER_PRESS_BUFFER)] = calc_hr;

            for (int i = 0; i < FINGER_PRESS_BUFFER; i++)
            {
              if (finger_press[i] == 0 || (0 <= finger_press[i] && finger_press[i]<=120))
                {
                  finger_present = 1;
                  break;
                }
              else if (i == (FINGER_PRESS_BUFFER-1))
                finger_present = 0;
            }

            if (!finger_present)
            {
              heart_rate = 0;
            }

            sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1);

            count++;

//            printf ("\nCalc_hr: %d, Heart Rate:  %d, Period: %d, finger_present: %d ", calc_hr, heart_rate, heart_rate, finger_present);

            LOG_INFO ("Heart Rate:  %d", heart_rate);


            hrm_heartrate_flt = UINT32_TO_FLOAT((float)heart_rate*1000, -3);

            UINT32_TO_BITSTREAM(p, heart_rate);

            if (heart_rate == 0)
            {
                ble_data_ptr->heart_rate_status_led_value = condition_NotUsed;
                displayPrintf(DISPLAY_ROW_9, "Not Pressed");
            }
            else if ((0 < heart_rate) && (heart_rate <= 60))
            {
                ble_data_ptr->heart_rate_status_led_value = condition_Bradycardia;
                displayPrintf(DISPLAY_ROW_9, "Bradycardia");
            }
            else if ((60 < heart_rate) && (heart_rate <= 100))
            {
                ble_data_ptr->heart_rate_status_led_value = condition_Normal;
                displayPrintf(DISPLAY_ROW_9, "Normal");
            }
            else if (100 <= heart_rate)
            {
                ble_data_ptr->heart_rate_status_led_value = condition_Tachycardia;
                displayPrintf(DISPLAY_ROW_9, "Tachycardia");
            }


  //          displayPrintf(DISPLAY_ROW_TEMPVALUE, "");

            if (ble_data_ptr->flag_conection == true &&
                ble_data_ptr->flag_indication_hr == true &&
                ble_data_ptr->flag_bonded == true &&
                ble_data_ptr->flag_indication_in_progress == true &&
                cbfifo_length() == cbfifo_capacity())
             {
               LOG_ERROR("Buffer Full!! This indication will not be stored and will be lost.");
             }
            // If indications are in flight then we will execute the below
            else if ((ble_data_ptr->flag_indication_hr == true) &&
                (ble_data_ptr->flag_conection == true) &&
                (ble_data_ptr->flag_indication_in_progress == false) &&
                cbfifo_length() != cbfifo_capacity())
            {

                // Sending indication
                sc = sl_bt_gatt_server_send_indication(ble_data_ptr->connectionHandle,
                                                       gattdb_heart_rate_measurement,
                                                       sizeof(hrm_heartrate_buffer),
                                                       hrm_heartrate_buffer);
  //              LOG_INFO("Indication Sent");
                ble_data_ptr->flag_indication_in_progress = true;

                // Printing the error message if the Sending Indication fails
                if (sc != 0)
                  LOG_ERROR("!!! Sending Indication Failed !!!\nError Code: 0x%x",sc);

                displayPrintf(DISPLAY_ROW_TEMPVALUE, "%d bpm", (int)heart_rate);
            }
            else if ((ble_data_ptr->flag_indication_hr == true) &&
                     (ble_data_ptr->flag_conection == true) &&
                     (ble_data_ptr->flag_indication_in_progress == true))
            {
  //              printf("%x\n%x\n%x\n",gattdb_temperature_type, sizeof(htm_temperature_buffer), htm_temperature_buffer[1]);
                cb_buffer_load[0] = (uint8_t) ((gattdb_heart_rate_measurement >> 8) & 0x00FF);
                cb_buffer_load[1] = (uint8_t) ((gattdb_heart_rate_measurement >> 0) & 0x00FF);
                cb_buffer_load[2] = (uint8_t) ((sizeof(hrm_heartrate_buffer) >> 24) & 0x000000FF);
                cb_buffer_load[3] = (uint8_t) ((sizeof(hrm_heartrate_buffer) >> 16) & 0x000000FF);
                cb_buffer_load[4] = (uint8_t) ((sizeof(hrm_heartrate_buffer) >> 8) & 0x000000FF);
                cb_buffer_load[5] = (uint8_t) ((sizeof(hrm_heartrate_buffer) >> 0) & 0x000000FF);
                cb_buffer_load[6] = hrm_heartrate_buffer[0];
                cb_buffer_load[7] = hrm_heartrate_buffer[1];
                cb_buffer_load[8] = hrm_heartrate_buffer[2];
                cb_buffer_load[9] = hrm_heartrate_buffer[3];
                cb_buffer_load[10] = hrm_heartrate_buffer[4];

  //              // For debugging purpose only
  //              printf("\nOriginal\n%d\t%d\t%d\t%d\t%d\n%d\n%d\n\n", cb_buffer_load[6],
  //                     cb_buffer_load[7],
  //                     cb_buffer_load[8],
  //                     cb_buffer_load[9],
  //                     cb_buffer_load[10],
  //                     sizeof(htm_temperature_buffer),
  //                     gattdb_temperature_type);
  //
  //              printf("%d", (sizeof(cb_buffer_load)/sizeof(uint8_t))); // For debugging purpose only
                cbfifo_enqueue(cb_buffer_load, (sizeof(cb_buffer_load)/sizeof(uint8_t)));

                LOG_INFO("Heart Rate indication added to buffer : %d indications left in the buffer", (cbfifo_length()/11));
            }



            // Writing attribute value to the GATT server
            sc = sl_bt_gatt_server_write_attribute_value(gattdb_heart_rate_led,
                                                         0,
                                                         sizeof(ble_data_ptr->heart_rate_status_led_value),
                                                         &(ble_data_ptr->heart_rate_status_led_value));

            // Printing the error message if the Server Write Failed fails
            if (sc != 0)
              LOG_ERROR("!!! Server Write Failed !!!\nError Code: 0x%x",sc);


            if (ble_data_ptr->flag_conection == true &&
                ble_data_ptr->flag_indication_hr_led == true &&
                ble_data_ptr->flag_bonded == true &&
                ble_data_ptr->flag_indication_in_progress == true &&
                cbfifo_length() == cbfifo_capacity())
             {
               LOG_ERROR("Buffer Full!! This indication will not be stored and will be lost.");
             }
            // If indications are in flight then we will execute the below
            else if ((ble_data_ptr->flag_indication_hr_led == true) &&
                (ble_data_ptr->flag_conection == true) &&
                (ble_data_ptr->flag_indication_in_progress == false) &&
                cbfifo_length() != cbfifo_capacity())
            {
                // Sending indication
                sc = sl_bt_gatt_server_send_indication(ble_data_ptr->connectionHandle,
                                                       gattdb_heart_rate_led,
                                                       sizeof(ble_data_ptr->heart_rate_status_led_value),
                                                       &(ble_data_ptr->heart_rate_status_led_value));
  //              LOG_INFO("Indication Sent");
                ble_data_ptr->flag_indication_in_progress = true;

                // Printing the error message if the Sending Indication fails
                if (sc != 0)
                  LOG_ERROR("!!! Sending Indication Failed !!!\nError Code: 0x%x",sc);

                displayPrintf(DISPLAY_ROW_TEMPVALUE, "%d bpm", (int)heart_rate);
            }
            else if ((ble_data_ptr->flag_indication_hr_led == true) &&
                     (ble_data_ptr->flag_conection == true) &&
                     (ble_data_ptr->flag_indication_in_progress == true))
            {
  //              printf("%x\n%x\n%x\n",gattdb_temperature_type, sizeof(htm_temperature_buffer), htm_temperature_buffer[1]);
                cb_buffer_load[0] = (uint8_t) ((gattdb_heart_rate_led >> 8) & 0x00FF);
                cb_buffer_load[1] = (uint8_t) ((gattdb_heart_rate_led >> 0) & 0x00FF);
                cb_buffer_load[2] = (uint8_t) ((sizeof(ble_data_ptr->heart_rate_status_led_value) >> 24) & 0x000000FF);
                cb_buffer_load[3] = (uint8_t) ((sizeof(ble_data_ptr->heart_rate_status_led_value) >> 16) & 0x000000FF);
                cb_buffer_load[4] = (uint8_t) ((sizeof(ble_data_ptr->heart_rate_status_led_value) >> 8) & 0x000000FF);
                cb_buffer_load[5] = (uint8_t) ((sizeof(ble_data_ptr->heart_rate_status_led_value) >> 0) & 0x000000FF);
                cb_buffer_load[6] = ble_data_ptr->heart_rate_status_led_value;
                cb_buffer_load[7] = 0;
                cb_buffer_load[8] = 0;
                cb_buffer_load[9] = 0;
                cb_buffer_load[10] = 0;

  //              // For debugging purpose only
  //              printf("\nOriginal\n%d\t%d\t%d\t%d\t%d\n%d\n%d\n\n", cb_buffer_load[6],
  //                     cb_buffer_load[7],
  //                     cb_buffer_load[8],
  //                     cb_buffer_load[9],
  //                     cb_buffer_load[10],
  //                     sizeof(htm_temperature_buffer),
  //                     gattdb_temperature_type);
  //
  //              printf("%d", (sizeof(cb_buffer_load)/sizeof(uint8_t))); // For debugging purpose only
                cbfifo_enqueue(cb_buffer_load, (sizeof(cb_buffer_load)/sizeof(uint8_t)));

                LOG_INFO("LED indication added to buffer : %d indications left in the buffer", (cbfifo_length()/11));
            }



            memset(hr_buffer, 0, (MASTER_BUFFER*sizeof(uint32_t)));

            nextState = state_Idle_hr;
          }
          else
          {
            nextState = state_Init_hr;
          }
      }
      if (event == event_SystemError_hr)
      {
          nextState = state_Idle_hr;
      }
    break;
  }
} // state_machine()


/**************************************************************************//**
 * This is a state machine that is designed for discovering the services and
 * characteristics when a connection between two BLE devices are established.
 *
 * @param:
 *      evt is the pointer that points to the union that holds all the events
 *
 * @return:
 *      no return
 *****************************************************************************/
void state_machine_discovery (sl_bt_msg_t *evt)
{

  // Changed for Assignment 5
  // Since the evt is a pointer, we will drill into the data structure to get the event code
  int32_t event = SL_BT_MSG_ID(evt->header), sc=0;

  ble_data_struct_t *ble_data_ptr = getBleDataPtr();

  State_t_disc currentState;
  static State_t_disc nextState = state_Idle_disc;

   currentState = nextState;

  switch (currentState)
  {
    /****************************State 1****************************/
    case state_Idle_disc:

      nextState = state_Idle_disc; // default
      if (event == sl_bt_evt_connection_opened_id) // When connection Open happens
      {
          LOG_INFO("State1\n"); // For debugging purpose only

          /**Discovery of Services**/
          // Discovering Services by UUID.
          sc = sl_bt_gatt_discover_primary_services_by_uuid(ble_data_ptr->connectionHandle,
                                                            sizeof(hrm_service_uuid),
                                                            &hrm_service_uuid.id[0]);

          // Printing the error message if the Discover Service by UUID fails
          if (sc != 0)
            LOG_ERROR("!!! Discover Heart Rate Service by UUID failed !!!\nError Code: 0x%x",sc);

          nextState = state_Service_temp_disc;
      }
      if (event == event_Error_disc || event == sl_bt_evt_connection_closed_id || event == event_SystemError_hr)
      {
//          sl_bt_system_reset(sl_bt_system_boot_mode_normal);
          nextState = state_Idle_disc;
      }

    break;

    /****************************State 2****************************/
    case state_Service_temp_disc:

      if(evt->header == sl_bt_evt_connection_closed_id)
      {
          nextState = state_Idle_disc;
      }
      else
      {
        nextState = state_Service_temp_disc; // Wait till service is found in the device
      }

      if ((event == sl_bt_evt_gatt_procedure_completed_id) && ((evt->data.evt_gatt_procedure_completed.result) == 0) && (nextState == state_Service_temp_disc)) // When Services by UUID is successfully completed
      {
          LOG_INFO("State2\n"); // For debugging purpose only
          /**Discovery of Characteristics**/
          // Discovering Characteristics by UUID.
          sc = sl_bt_gatt_discover_characteristics_by_uuid(ble_data_ptr->connectionHandle,
                                                           ble_data_ptr->myServiceHandle_hr,
                                                           sizeof(hrm_characterstic_uuid),
                                                           &hrm_characterstic_uuid.id[0]);

          // Printing the error message if the Discover Characterstic by UUID fails
          if (sc != 0)
            LOG_ERROR("!!! Discover Heart Rate Characteristic by UUID failed !!!\nError Code: 0x%x",sc);


          nextState = state_Characteristic_temp_disc;
      }
      if (event == event_Error_disc || event == sl_bt_evt_connection_closed_id || event == event_SystemError_hr)
      {
//          sl_bt_system_reset(sl_bt_system_boot_mode_normal);
          nextState = state_Idle_disc;
      }

    break;

    /****************************State 3****************************/
    case state_Characteristic_temp_disc:

      if(evt->header == sl_bt_evt_connection_closed_id)
      {
          nextState = state_Idle_disc;
      }

      else
      {
          nextState = state_Characteristic_temp_disc; // Wait till service is found in the device
      }
      if ((event == sl_bt_evt_gatt_procedure_completed_id) && (nextState == state_Characteristic_temp_disc)) // When Characteristics by UUID is successfully completed
      {
          LOG_INFO("State3\n"); // For debugging purpose only


          /**Discovery of Services**/
          // Discovering Services by UUID.
          sc = sl_bt_gatt_discover_primary_services_by_uuid(ble_data_ptr->connectionHandle,
                                                            sizeof(hr_led_state_service_uuid),
                                                            &hr_led_state_service_uuid.id[0]);

          // Printing the error message if the Discover Service by UUID fails
          if (sc != 0)
            LOG_ERROR("!!! Discover Heart Rate LED Service by UUID failed !!!\nError Code: 0x%x",sc);

          nextState = state_Service_button_state_disc;
      }
      if (event == event_Error_disc || event == sl_bt_evt_connection_closed_id || event == event_SystemError_hr)
      {
//          sl_bt_system_reset(sl_bt_system_boot_mode_normal);
          nextState = state_Idle_disc;
      }
    break;

    /****************************State 4****************************/
    case state_Service_button_state_disc:

      if(evt->header == sl_bt_evt_connection_closed_id)
      {
          nextState = state_Idle_disc;
      }

      else
      {
          nextState = state_Service_button_state_disc; // Wait till service is found in the device
      }
      if ((event == sl_bt_evt_gatt_procedure_completed_id) && (nextState == state_Service_button_state_disc)) // When Characteristics by UUID is successfully completed
      {
          LOG_INFO("State4\n"); // For debugging purpose only


          /**Discovery of Characteristics**/
          // Discovering Characteristics by UUID.
          sc = sl_bt_gatt_discover_characteristics_by_uuid(ble_data_ptr->connectionHandle,
                                                           ble_data_ptr->myServiceHandle_button_state,
                                                           sizeof(hr_led_state_characterstic_uuid),
                                                           &hr_led_state_characterstic_uuid.id[0]);

          // Printing the error message if the Discover Characterstic by UUID fails
          if (sc != 0)
            LOG_ERROR("!!! Discover  Heart Rate LED Characteristic by UUID failed !!!\nError Code: 0x%x",sc);

          nextState = state_Characteristic_button_state_disc;
      }
      if (event == event_Error_disc || event == sl_bt_evt_connection_closed_id || event == event_SystemError_hr)
      {
//          sl_bt_system_reset(sl_bt_system_boot_mode_normal);
          nextState = state_Idle_disc;
      }
    break;

    /****************************State 5****************************/
    case state_Characteristic_button_state_disc:

      if(evt->header == sl_bt_evt_connection_closed_id)
      {
          nextState = state_Idle_disc;
      }
      else
      {
          nextState = state_Characteristic_button_state_disc; // Wait till characteristic is found in the device
      }

      if ((nextState == state_Characteristic_button_state_disc) && (event == sl_bt_evt_gatt_procedure_completed_id) && ((evt->data.evt_gatt_procedure_completed.result) == 0)) // When Characteristics by UUID is successfully completed
      {
          LOG_INFO("State5\n"); // For debugging purpose only

          /**Enabling Indications**/
          // Enabling Indications.
          sc = sl_bt_gatt_set_characteristic_notification(ble_data_ptr->connectionHandle,
                                                          ble_data_ptr->myCharacteristicHandle_hr,
                                                          2);
          // Printing the error message if the Indication Set fails
          if (sc != 0)
            LOG_ERROR("!!!  Heart Rate Indication Set failed !!!\nError Code: 0x%x",sc);

          ble_data_ptr->flag_indication_hr = true;

          nextState = state_Indication_temp_disc;
      }
      if (event == event_Error_disc || event == sl_bt_evt_connection_closed_id || event == event_SystemError_hr)
      {
//          sl_bt_system_reset(sl_bt_system_boot_mode_normal);
          nextState = state_Idle_disc;
      }
    break;

    /****************************State 6****************************/
    case state_Indication_temp_disc:

      if(evt->header == sl_bt_evt_connection_closed_id)
      {
        nextState = state_Idle_disc;
      }
      else
      {
          nextState = state_Indication_temp_disc; // Wait till characteristic is found in the device
      }
      if ((nextState == state_Indication_temp_disc) && (event == sl_bt_evt_gatt_procedure_completed_id) && ((evt->data.evt_gatt_procedure_completed.result) == 0)) // When Characteristics by UUID is successfully completed
      {
          LOG_INFO("State6\n"); // For debugging purpose only

          /**Enabling Indications**/
          // Enabling Indications.
          sc = sl_bt_gatt_set_characteristic_notification(ble_data_ptr->connectionHandle,
                                                          ble_data_ptr->myCharacteristicHandle_button_state,
                                                          2);
          // Printing the error message if the Indication Set fails
          if (sc != 0)
            LOG_ERROR("!!!  Heart Rate LED Indication Set failed !!!\nError Code: 0x%x",sc);

          if (sc == 0)
            {
              displayPrintf(DISPLAY_ROW_CONNECTION, "Handling Indications");
              displayPrintf(DISPLAY_ROW_PASSKEY, "Hit Button 1 to bond");
            }


          ble_data_ptr->flag_indication_hr_led = true;

          nextState = state_Indication_wait_disc;
      }
      if (event == event_Error_disc || event == sl_bt_evt_connection_closed_id || event == event_SystemError_hr)
      {
//          sl_bt_system_reset(sl_bt_system_boot_mode_normal);
          nextState = state_Idle_disc;
      }
    break;

    /****************************State 7****************************/
    case state_Indication_wait_disc:
          nextState = state_Indication_wait_disc; // Wait till service is found in the device

      if ((event == sl_bt_evt_connection_closed_id)) // When Characteristics by UUID is successfully completed
      {
          LOG_INFO("State7\n"); // For debugging purpose only

          /**Disabling the indications on close event**/
          // Disabling indications on close event.
          sc = sl_bt_gatt_set_characteristic_notification(ble_data_ptr->connectionHandle,
                                                          ble_data_ptr->myCharacteristicHandle_hr,
                                                          0);

          // Printing the error message if the Indication Set fails
          if (sc != 0)
            LOG_ERROR("!!! Indication Set failed !!!\nError Code: 0x%x",sc);

          nextState = state_Idle_disc;
      }
      if (event == event_Error_disc || event == sl_bt_evt_connection_closed_id || event == event_SystemError_hr)
      {
//          sl_bt_system_reset(sl_bt_system_boot_mode_normal);
          nextState = state_Idle_disc;
      }
    break;
  }
}


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
  for (i=0; i<=num_Events_temp ;i++)
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

    sl_bt_external_signal(event_measureMAX30101_hr);

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

    sl_bt_external_signal(event_timerWaitUS_IRQ_temp);

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

    sl_bt_external_signal(event_I2CTransfer_IRQ_hr);

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

/**************************************************************************//**
 * This function creates an event in the event handler. An event is set by
 * setting the corresponding bit to 1. The bit number is the same order as
 * of eventList.
 *
 * This is to say that there is an error that has  during the temperature
 * state machine. This will be encountered by reseting the device.
 *
 * @param:
 *      no params
 *
 * @return:
 *      no return
 *****************************************************************************/
void createEventErrorTemp()
{
    CORE_DECLARE_IRQ_STATE;

    CORE_ENTER_CRITICAL();

//    eventHandler = (eventHandler | (1<<(event_I2CTransfer_IRQ-1))); // setting the first bit to 1

    sl_bt_external_signal(event_Error_temp);

    CORE_EXIT_CRITICAL();

//    LOG_INFO("I2CWrite Event");
}


/**************************************************************************//**
 * This function creates an event in the event handler. An event is set by
 * setting the corresponding bit to 1. The bit number is the same order as
 * of eventList.
 *
 * This is to say that there is an error that has  during the temperature
 * state machine. This will be encountered by reseting the device.
 *
 * @param:
 *      no params
 *
 * @return:
 *      no return
 *****************************************************************************/
void createEventPB0Pressed()
{
    CORE_DECLARE_IRQ_STATE;

    CORE_ENTER_CRITICAL();

//    eventHandler = (eventHandler | (1<<(event_PB0Pressed_temp,-1))); // setting the first bit to 1

    sl_bt_external_signal(event_PB0Pressed_hr);

    CORE_EXIT_CRITICAL();

//    LOG_INFO("I2CWrite Event");
}


/**************************************************************************//**
 * This function creates an event in the event handler. An event is set by
 * setting the corresponding bit to 1. The bit number is the same order as
 * of eventList.
 *
 * This is to say that there is an error that has  during the temperature
 * state machine. This will be encountered by reseting the device.
 *
 * @param:
 *      no params
 *
 * @return:
 *      no return
 *****************************************************************************/
void createEventPB1Pressed()
{
    CORE_DECLARE_IRQ_STATE;

    CORE_ENTER_CRITICAL();

//    eventHandler = (eventHandler | (1<<(event_PB0Pressed_temp,-1))); // setting the first bit to 1

    sl_bt_external_signal(event_PB1Pressed_hr);

    CORE_EXIT_CRITICAL();

//    LOG_INFO("I2CWrite Event");
}


/**************************************************************************//**
 * This function creates an event in the event handler. An event is set by
 * setting the corresponding bit to 1. The bit number is the same order as
 * of eventList.
 *
 * This is to say that there is an interrupt that has been triggered by the
 * MAX30101 Sensor
 *
 * @param:
 *      no params
 *
 * @return:
 *      no return
 *****************************************************************************/
void createEventMAX30101Int()
{
    CORE_DECLARE_IRQ_STATE;

    CORE_ENTER_CRITICAL();

//    eventHandler = (eventHandler | (1<<(event_PB0Pressed_temp,-1))); // setting the first bit to 1

    uint8_t int_reg_0;

    i2c_Write_Read_blocking(0x00, &int_reg_0, sizeof(int_reg_0));

    if (int_reg_0 &  0x80)
    {
      sl_bt_external_signal(event_bufferFullMAX30101_hr);
    }

    CORE_EXIT_CRITICAL();

//    LOG_INFO("I2CWrite Event");
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
void createEventSystemError()
{
    CORE_DECLARE_IRQ_STATE;

    CORE_ENTER_CRITICAL();

//    eventHandler = (eventHandler | (1<<(event_timerWaitUS_IRQ-1))); // setting the first bit to 1

    sl_bt_external_signal(event_SystemError_hr);

    CORE_EXIT_CRITICAL();

//    LOG_INFO("EVENT 2 SET");
}


/**************************************************************************//**
 * Enqueues data onto the FIFO, up to the limit of the available FIFO
 * capacity.
 *
 * This is to say that there is an error that has  during the temperature
 * state machine. This will be encountered by reseting the device.
 *
 * @param:
 *   buf      Pointer to the data
 *   nbyte    Max number of bytes to enqueue
 *
 * @return:
 *   The number of bytes actually enqueued, which could be 0. In case
 *   of an error, returns -1.
 *****************************************************************************/
size_t cbfifo_enqueue(void *buf, size_t nbyte)
{
    /* checks for the capacity of the array buffer */
    uint16_t available_capacity = ARRAY_CAPACITY - (cbfifo_length());
    size_t count = 0;
    if(buf == NULL)
    {
        return -1;
    }

    if(available_capacity > ZERO)
    {
        if(nbyte <= available_capacity)
        {
           while(nbyte--)
            {
                /*write pointer writes to the memory location and increments */
                *write++ = *(uint8_t*)buf++;
                count++;
                if(write == (cbfifo_array + ARRAY_CAPACITY))
                {
                    write = cbfifo_array;
                }

            }
        }
        else
        {
           /*if enough space is not available, only enqueue as much as possible */
            while(available_capacity--)
            {

                *write++ = *(uint8_t*)buf++;
                count++;
                if(write == (cbfifo_array + ARRAY_CAPACITY))
                {
                    write = cbfifo_array;
                }

            }
        }
        /* write pointer has gone around and filled all the array spaces,
        set the capacity full flag */
        if(write == read)
        {
            capacity_full = ONE;
        }
    }
    else
    {
        count = 0;
    }
    return count;
}


/**************************************************************************//**
 * Attempts to remove ("dequeue") up to nbyte bytes of data from the
 * FIFO. Removed data will be copied into the buffer pointed to by buf.
 *
 * To further explain the behavior: If the FIFO's current length is 24
 * bytes, and the caller requests 30 bytes, cbfifo_dequeue should
 * return the 24 bytes it has, and the new FIFO length will be 0. If
 * the FIFO is empty (current length is 0 bytes), a request to dequeue
 * any number of bytes will result in a return of 0 from
 * cbfifo_dequeue.
 *
 * @param:
 *   buf      Destination for the dequeued data
 *   nbyte    Bytes of data requested
 *
 * @return:
 *   The number of bytes actually copied, which will be between 0 and
 *   nbyte.
 *****************************************************************************/
size_t cbfifo_dequeue(void *buf, size_t nbyte)
{
    uint16_t available_dequeue = cbfifo_length();
    uint16_t count = 0;
    if(buf == NULL)
    {
        return -1;
    }
    if(available_dequeue > ZERO)
    {
        /* if bytes are available to dequeue, check the nbyte requirement */
        if(nbyte <= available_dequeue )
        {
            while(nbyte--)
            {
                /*read from pointer into the buffer and increment, go around if 128th loc is reached */
                *(uint8_t*)buf++ = *read;
                *read++ = 0;
                count++;
                if(read == (cbfifo_array + ARRAY_CAPACITY))
                {
                    read = cbfifo_array;
                }
            }
      }
      else
      {
        while(available_dequeue--)
        {
           *(uint8_t*)buf++ = *read;
            *read++ = 0;
            count++;
            if(read == (cbfifo_array + ARRAY_CAPACITY))
            {
                read = cbfifo_array;
            }
        }
      }
        capacity_full = ZERO;
    }
    else
    {
        count = 0;
    }
    return count;
}


/**************************************************************************//**
 *
 * Returns the number of bytes currently on the FIFO.
 *
 * @param:
 *      no params
 *
 * @return:
 *      Number of bytes currently available to be dequeued from the FIFO
 *****************************************************************************/
size_t cbfifo_length()
{
    uint16_t ret;
    if((capacity_full) == 1)
    {
        return ARRAY_CAPACITY;
    }
    if(write < read)
    {
        /*since write address value is less than read, read has to go around till 128 to get length  */
        ret = ((cbfifo_array + ARRAY_CAPACITY) - read) + (write - cbfifo_array);
    }
    else
    {
       ret = write - read;
    }
    return ret;
}


/**************************************************************************//**
 *
 * Returns the FIFO's capacity
 *
 * @param:
 *      no params
 *
 * @return:
 *      The capacity, in bytes, for the FIFO
 *****************************************************************************/
size_t cbfifo_capacity()
{
    return sizeof(cbfifo_array);
}
