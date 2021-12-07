/*
 * ble.c
 *
 *  Created on: 1 Oct 2021
 *      Author: nihalt
 *      Attribute: Prof. David Sluiter
 *                 getBleDataPtr()
 *                 Sequence of BLE events are directed by Prof. David Sluiter
 *
 *                 Boot Event Case is attributed to the Thermometer SoC Example
 */

#include "ble.h"

#define INCLUDE_LOG_DEBUG 1
#include "src/log.h"

#define SCANNING_MODE (0)                 // 0 - Passive scanning, 1 - Active scanning
#define PHY (1)                           // 1 - 1M PHY, 4 - Coded PHY, 5 - 1M PHY and Coded PHY
#define SCAN_INTERVAL(n) (n/0.625f)       // Computes the value that is to be entered when the time in ms is passed
#define SCAN_WINDOW(n) (n/0.625f)         // Computes the value that is to be entered when the time in ms is passed
#define SCAN_INTERVAL_MS (50)             // Setting the scan interval
#define SCAN_WINDOW_MS (25)               // Computes the value that is to be entered when the time in ms is passed
#define DISCOVER_MODE (2)                 // 0 - Discover only limited discoverable devices, 1 - Discover limited and generic discoverable devices, 2 - Discover all devices.


#define SM_CONFIG_FLAGS (0x0F)

// BLE private data
ble_data_struct_t ble_data; // Declaring a private data to initiate a function to get the pointer to this data structure

/**************************************************************************//**
 * This function obtains the pointer to the structure ble_data_struct_t
 *
 * @param:
 *      no params
 * @return:
 *      returns the address of the pointer
 *****************************************************************************/
ble_data_struct_t* getBleDataPtr()
{
  return (&ble_data);
} // getBleDataPtr()


/**************************************************************************//**
 * This function initializes the ble_data structure to its initial state values
 *
 * @param:
 *      no params
 * @return:
 *      returns the address of the pointer
 *****************************************************************************/
void ble_Init(void)
{
  ble_data_struct_t *ble_data_ptr = getBleDataPtr();

  // Initializing the flags to false at the start of the program
  ble_data_ptr->flag_conection = false;
  ble_data_ptr->flag_indication_temp = false;
  ble_data_ptr->flag_indication_in_progress = false;
  ble_data_ptr->flag_indication_button_state = false;
  ble_data_ptr->flag_bonded = false;
  ble_data_ptr->button_0_flag = false;
  ble_data_ptr->button_1_flag = false;
}


/**************************************************************************//**
 * This function is a state machine that deals with the booting event of the
 * blue tooth events like booting, advertising, connection and timeout events.
 *
 * @param:
 *      no params
 * @return:
 *      returns the address of the pointer
 *****************************************************************************/
void ble_handler(sl_bt_msg_t *evt) {

  // Variable declaration
  sl_status_t sc;

  buffer_seq unload_buffer_seq_unwrap;

  ble_data_struct_t *ble_data_ptr = getBleDataPtr(); // Getting the pointer to the ble_data_ptr

  uint8_t system_id[8], cb_buffer_load[11]={0}, cb_buffer_unload[11]={0};

  float float_temp;
  uint8_t int_button;

  switch (SL_BT_MSG_ID(evt->header))
  {

    /******************************************************************************************/
    /******************************************************************************************/
    /****************************Events Common to Client and Server****************************/
    /******************************************************************************************/
    /******************************************************************************************/


    /****************************Boot Event****************************/
    case sl_bt_evt_system_boot_id:

      displayInit();

      /**Get Identity**/
      // Checking if the boot was successful (non 0 is an error)
      sc = sl_bt_system_get_identity_address(&(ble_data_ptr->myAddress), &(ble_data_ptr->myAddressType));

      // Printing the error message if the boot fails
      if (sc != 0)
        LOG_ERROR("!!! BT Stack Boot Fail !!!\nError Code: 0x%x",sc);

//      // Pad and reverse unique ID to get System ID.  // For debugging purpose
//      system_id[0] = ble_data_ptr->myAddress.addr[5]; // For debugging purpose
//      system_id[1] = ble_data_ptr->myAddress.addr[4]; // For debugging purpose
//      system_id[2] = ble_data_ptr->myAddress.addr[3]; // For debugging purpose
//      system_id[3] = 0xFF;                            // For debugging purpose
//      system_id[4] = 0xFE;                            // For debugging purpose
//      system_id[5] = ble_data_ptr->myAddress.addr[2]; // For debugging purpose
//      system_id[6] = ble_data_ptr->myAddress.addr[1]; // For debugging purpose
//      system_id[7] = ble_data_ptr->myAddress.addr[0]; // For debugging purpose

      // Printing the error message if the Server Config Failed fails
      if (sc != 0)
        LOG_ERROR("!!! Server Config Failed !!!\nError Code: 0x%x",sc);

      displayPrintf(DISPLAY_ROW_BTADDR, "%x:%x:%x:%x:%x:%x", \
                    ble_data_ptr->myAddress.addr[0],
                    ble_data_ptr->myAddress.addr[1],
                    ble_data_ptr->myAddress.addr[2],
                    ble_data_ptr->myAddress.addr[3],
                    ble_data_ptr->myAddress.addr[4],
                    ble_data_ptr->myAddress.addr[5]);

          #if DEVICE_IS_BLE_SERVER
          /*------------------------------Server Events------------------------------*/

          //      LOG_INFO("System Boot Event"); // For debugging purpose

                displayPrintf(DISPLAY_ROW_NAME, "Server");
                displayPrintf(DISPLAY_ROW_ASSIGNMENT, "A9");


                /**Creating Advertising Set**/
                // Create an advertising set.
                sc = sl_bt_advertiser_create_set(&(ble_data_ptr->advertisingSetHandle));

                // Printing the error message if the Advertising Set Not Created fails
                if (sc != 0)
                  LOG_ERROR("!!! Advertising Set Not Created !!!\nError Code: 0x%x",sc);


                /**Advertising Timing Set**/
                // Set advertising interval to 100ms.
                sc = sl_bt_advertiser_set_timing(ble_data_ptr->advertisingSetHandle, // advertising set handle
                                                 400, // min. adv. interval (milliseconds * 1.6)
                                                 400, // max. adv. interval (milliseconds * 1.6)
                                                 0,   // adv. duration
                                                 0);  // max. num. adv. events

                // Printing the error message if the boot fails
                if (sc != 0)
                  LOG_ERROR("!!! Advertising Timming Set Failed !!!\nError Code: 0x%x",sc);


                /**Advertising Start**/
                // Start general advertising and enable connections.
                sc = sl_bt_advertiser_start(ble_data_ptr->advertisingSetHandle,
                                            sl_bt_advertiser_general_discoverable,
                                            sl_bt_advertiser_connectable_scannable);

                // Printing the error message if the Advertising Start fails
                if (sc != 0)
                  LOG_ERROR("!!! Advertising Start Failed !!!\nError Code: 0x%x",sc);


          //      LOG_INFO("Started Advertising!!!!\n"); // For debugging purpose

                displayPrintf(DISPLAY_ROW_CONNECTION, "Advertising");

          #else
          /*------------------------------Client Events------------------------------*/

                displayPrintf(DISPLAY_ROW_NAME, "Client");
                displayPrintf(DISPLAY_ROW_ASSIGNMENT, "A9");

                 /**Setting Scanning Mode**/
                 // Setting the scanning mode
                 sc = sl_bt_scanner_set_mode(PHY, SCANNING_MODE); // Refer to the macro to find the PHY mode and mode of scanning
                 // Printing the error message if the Scanning Mode Set fails
                 if (sc != 0)
                   LOG_ERROR("!!! Scanning Mode Set Failed !!!\nError Code: 0x%x",sc);

                 /**Setting Scanning Timing**/
                 // Setting the scanning mode
                 sc = sl_bt_scanner_set_timing(PHY, SCAN_INTERVAL(SCAN_INTERVAL_MS), SCAN_WINDOW(SCAN_WINDOW_MS));
                 // Printing the error message if the Scanning Timing Set fails
                 if (sc != 0)
                   LOG_ERROR("!!! Scanning Timing Set Failed !!!\nError Code: 0x%x",sc);

                 /**Setting Scanning Parameters**/
                 // Setting the scanning mode
                 sc = sl_bt_connection_set_default_parameters(60,                                         // Minimum value for the connection event interval (Value x 1.25 ms)
                                                              60,                                         // Maximum value for the connection event interval (Value x 1.25 ms)
                                                              3,                                          // Peripheral latency, which defines how many connection intervals the peripheral can skip if it has no data to send
                                                              80,                                         // Supervision timeout (Value x 10 ms)
                                                              0,                                          // Minimum value for the connection event length. (Value x 0.625 ms)
                                                              0x4); // 0xFFFF                             // Maximum value for the connection event length. (Value x 0.625 ms));
                 // Printing the error message if the Scanning Parameters Set fails
                 if (sc != 0)
                   LOG_ERROR("!!! Scanning Parameters Set Failed !!!\nError Code: 0x%x",sc);

                 /**Start Scanning**/
                 // Setting the scanning mode
                 sc = sl_bt_scanner_start(PHY, DISCOVER_MODE);
                 // Printing the error message if the Start Scanning fails
                 if (sc != 0)
                   LOG_ERROR("!!! Start Scanning Failed !!!\nError Code: 0x%x",sc);

                 displayPrintf(DISPLAY_ROW_CONNECTION, "Discovering");

          #endif


    break;

    /*************************Connection Open Event*************************/
    // Once the connection to the server is initiated this id is activated
    case sl_bt_evt_connection_opened_id: // handle open event

        #if DEVICE_IS_BLE_SERVER
        /*------------------------------Server Events------------------------------*/

        //      LOG_INFO("Connection Initiated"); // For debugging purpose

              ble_data_ptr->flag_conection = true; // Setting the connection flag to true
              ble_data_ptr->connectionHandle = evt->data.evt_connection_opened.connection; // Storing the connection handle in our structure

              /**Advertising Stop**/
              // Stop advertising since the connection is initiated
              sc = sl_bt_advertiser_stop(ble_data_ptr->advertisingSetHandle);

              // Printing the error message if the Advertising Stop fails
              if (sc != 0)
                LOG_ERROR("!!! Advertising Stop Failed !!!\nError Code: 0x%x",sc);


              /**Setting Connection Parameters**/
              // Setting the parameters for connection
              sc = sl_bt_connection_set_parameters(evt->data.evt_connection_opened.connection, // Connection Handle
                                                   60,                                         // Minimum value for the connection event interval (Value x 1.25 ms)
                                                   60,                                         // Maximum value for the connection event interval (Value x 1.25 ms)
                                                   3,                                          // Peripheral latency, which defines how many connection intervals the peripheral can skip if it has no data to send
                                                   80,                                         // Supervision timeout (Value x 10 ms)
                                                   0,                                          // Minimum value for the connection event length. (Value x 0.625 ms)
                                                   0xFFFF);                                    // Maximum value for the connection event length. (Value x 0.625 ms)

              // Printing the error message if the Setting Parameters fails
              if (sc != 0)
                LOG_ERROR("!!! Setting Parameters Failed !!!\nError Code: 0x%x",sc);


              /**Security Manager Configuration**/
              sc = sl_bt_sm_configure(SM_CONFIG_FLAGS,
                                      sm_io_capability_displayyesno);
              // Printing the error message if the Server Write Failed fails
              if (sc != 0)
                LOG_ERROR("!!! Security Manager Configuration Failed !!!\nError Code: 0x%x",sc);

              // This code will be run only if the temperature state machine is required to run if we have an active connection and the indication are ON
        #if NOP_INDICATION_CONNECTION == 1
              if ((ble_data_ptr->flag_conection == true) && (ble_data_ptr->flag_indication_temp == true) && (ble_data_ptr->flag_bonded == true))
              {
                  LETIMER_IntEnable(LETIMER0, LETIMER_IEN_UF);
        //          LOG_INFO("Enabled"); // For debugging purpose
              }
        #endif

              /**Setting up the timer to poll for the circular buffer**/
              // We will poll every second
              sc = sl_bt_system_set_soft_timer ((32768*2),   // 1 second is equal to 32768 ticks
                                                 3,           // handle = 3
                                                 0);;         // repeating

              // Printing the error message if the Server Write Failed fails
              if (sc != 0)
                LOG_ERROR("!!! Soft Timer Setup Failed !!!\nError Code: 0x%x",sc);


              displayPrintf(DISPLAY_ROW_CONNECTION, "Advertising");
              displayPrintf(DISPLAY_ROW_TEMPVALUE, "");
              gpioLed1SetOff();

              displayPrintf(DISPLAY_ROW_CONNECTION, "Connected");


        #else
        /*------------------------------Client Events------------------------------*/

              displayPrintf(DISPLAY_ROW_CONNECTION, "Connected");
              displayPrintf(DISPLAY_ROW_BTADDR2, "%x:%x:%x:%x:%x:%x", \
                            SERVER_BT_ADDRESS.addr[0],
                            SERVER_BT_ADDRESS.addr[1],
                            SERVER_BT_ADDRESS.addr[2],
                            SERVER_BT_ADDRESS.addr[3],
                            SERVER_BT_ADDRESS.addr[4],
                            SERVER_BT_ADDRESS.addr[5]);

              ble_data_ptr->connectionHandle = evt->data.evt_connection_opened.connection; // Storing the connection handle in our structure

              /**Security Manager Configuration**/
              sc = sl_bt_sm_configure(SM_CONFIG_FLAGS,
                                      sm_io_capability_displayyesno);
              // Printing the error message if the Server Write Failed fails
              if (sc != 0)
                LOG_ERROR("!!! Security Manager Configuration Failed !!!\nError Code: 0x%x",sc);

        #endif

              /**Forgetting all Bonded Devices**/
              // Start general advertising and enable connections.
              sc = sl_bt_sm_delete_bondings();

              // Printing the error message if the Advertising Start fails
              if (sc != 0)
                LOG_ERROR("!!! Deleting Bonded Devices Failed !!!\nError Code: 0x%x",sc);

    break;

    /*************************Connection Close Event*************************/
    case sl_bt_evt_connection_closed_id: // handle close event

        #if DEVICE_IS_BLE_SERVER
      /*------------------------------Server Events------------------------------*/

              LOG_INFO("Connection Closed. Reason : %x\n", evt->data.evt_connection_closed.reason);

              ble_data_ptr->flag_conection = false;

              /**Starting Advertising**/
              // Start general advertising and enable connections.
              sc = sl_bt_advertiser_start(ble_data_ptr->advertisingSetHandle,
                                          sl_bt_advertiser_general_discoverable,
                                          sl_bt_advertiser_connectable_scannable);

              // Printing the error message if the Advertising Start fails
              if (sc != 0)
                LOG_ERROR("!!! Advertising Start Failed !!!\nError Code: 0x%x",sc);


              // This code will be run only if the temperature state machine is required to run if we have an active connection and the indication are ON.  Set NOP_INDICATION_CONNECTION in app.h
        #if NOP_INDICATION_CONNECTION == 1
              if ((ble_data_ptr->flag_conection == false) || (ble_data_ptr->flag_indication_temp == false))
                {
                  LETIMER_IntDisable(LETIMER0, LETIMER_IEN_UF);
        //          LOG_INFO("Disabled");
                  sl_bt_gatt_set_characteristic_notification(ble_data_ptr->connectionHandle,
                                                             gattdb_temperature_measurement,
                                                             0);
                  ble_data_ptr->flag_indication_temp = false;
                  ble_data_ptr->flag_indication_button_state = false;
                  ble_data_ptr->flag_bonded = false;
                }
        #endif

              displayPrintf(DISPLAY_ROW_CONNECTION, "Advertising");
              displayPrintf(DISPLAY_ROW_TEMPVALUE, "");
              gpioLed0SetOff();
              gpioLed1SetOff();

              MAX_30101_Reset();
              MAX_30101_ShutDown();

              createEventSystemError();

        #else
        /*------------------------------Client Events------------------------------*/

              displayPrintf(DISPLAY_ROW_CONNECTION, "Discovering");
              displayPrintf(DISPLAY_ROW_BTADDR2, "");
              displayPrintf(DISPLAY_ROW_TEMPVALUE, "");

              LOG_INFO("Connection Closed. Reason : %x\n", evt->data.evt_connection_closed.reason);

              /**Disabling the Indications**/
              // Create an advertising set.
              sc = sl_bt_gatt_set_characteristic_notification(ble_data_ptr->connectionHandle,
                                                              ble_data_ptr->myCharacteristicHandle_temp,
                                                              0);
              // Printing the error message if the Indication Set fails
              if (sc != 0)
                LOG_ERROR("!!! Indication Set failed !!!\nError Code: 0x%x",sc);

              /**Disabling the Indications**/
              // Create an advertising set.
              sc = sl_bt_gatt_set_characteristic_notification(ble_data_ptr->connectionHandle,
                                                              ble_data_ptr->myCharacteristicHandle_button_state,
                                                              0);
              // Printing the error message if the Indication Set fails
              if (sc != 0)
                LOG_ERROR("!!! Indication Set failed !!!\nError Code: 0x%x",sc);

              ble_data_ptr->flag_indication_temp = false;
              ble_data_ptr->flag_indication_button_state = false;

              LOG_INFO("Scanning");

              /**Start Scanning**/
              // Setting the scanning mode
              sc = sl_bt_scanner_start(PHY, DISCOVER_MODE);
              // Printing the error message if the Start Scanning fails
              if (sc != 0)
                LOG_ERROR("!!! Start Scanning Failed !!!\nError Code: 0x%x",sc);

        #endif


              /**Forgetting all Bonded Devices**/
              // Start general advertising and enable connections.
              sc = sl_bt_sm_delete_bondings();

              // Printing the error message if the Advertising Start fails
              if (sc != 0)
                LOG_ERROR("!!! Advertising Start Failed !!!\nError Code: 0x%x",sc);

    break;

    /***********************Connection Parameters Are Set***********************/
    case sl_bt_evt_connection_parameters_id:

        #if DEVICE_IS_BLE_SERVER
        /*------------------------------Server Events------------------------------*/

              // Uncomment this to get the parameter set values
        //      LOG_INFO("\n\nExpected:\nConnection Interval:75ms\nSlave Latency: 3\nSupervision Timeout: 600ms or above\n\n");
        //      LOG_INFO("\n\nActual:\nConnection Interval:%dms\nSlave Latency: %d\nSupervision Timeout: %d\n\n",
        //               (int)((evt->data.evt_connection_parameters.interval*1.25)),
        //               ((int)(evt->data.evt_connection_parameters.latency)),
        //                (int)((evt->data.evt_connection_parameters.timeout*10)));

              displayPrintf(DISPLAY_ROW_CONNECTION, "Connected");

        #else
        /*------------------------------Client Events------------------------------*/

        #endif

    break;

    /**************************Soft Timer Trigger Event**************************/
    case sl_bt_evt_system_soft_timer_id:
      switch(evt->data.evt_system_soft_timer.handle)
      {
        case 2:
          displayUpdate();
        break;

        case 3:
          // We will process the indication only when the length of bufer is not 0 and there is no other indication in progress
          if(ble_data_ptr->flag_indication_in_progress == false && cbfifo_length() != 0)
          {
              // Dequeueing the buffer
              cbfifo_dequeue(cb_buffer_unload, (sizeof(cb_buffer_unload)/sizeof(uint8_t)));

              // Unwrapping the array and storing the values in their original form
              unload_buffer_seq_unwrap.charHandle = ((cb_buffer_unload[0]<<8) |
                                                     (cb_buffer_unload[1]<<0));
              unload_buffer_seq_unwrap.bufferLen = ((cb_buffer_unload[2]<<24) |
                                                    (cb_buffer_unload[3]<<16) |
                                                    (cb_buffer_unload[4]<<8)  |
                                                    (cb_buffer_unload[5]<<0));
              unload_buffer_seq_unwrap.buffer[0] = cb_buffer_unload[6];
              unload_buffer_seq_unwrap.buffer[1] = cb_buffer_unload[7];
              unload_buffer_seq_unwrap.buffer[2] = cb_buffer_unload[8];
              unload_buffer_seq_unwrap.buffer[3] = cb_buffer_unload[9];
              unload_buffer_seq_unwrap.buffer[4] = cb_buffer_unload[10];

              // Checking for the type of character handle and then acting accordingly
              if (unload_buffer_seq_unwrap.charHandle == gattdb_heart_rate_led)
              {
//                  printf("Button Sending indications"); // For debugging purpose
                  // Sending indication // For debugging purpose
                  sc = sl_bt_gatt_server_send_indication(ble_data_ptr->connectionHandle,
                                                         gattdb_heart_rate_led,
                                                         1,
                                                         unload_buffer_seq_unwrap.buffer);
        //              LOG_INFO("Indication Sent");
                  ble_data_ptr->flag_indication_in_progress = true;

                  // Printing the error message if the Sending Indication fails
                  if (sc != 0)
                      LOG_ERROR("!!! Buffer Sending Indication Failed !!!\nError Code: 0x%x",sc);

                  // Writing attribute value to the GATT server
                  sc = sl_bt_gatt_server_write_attribute_value(gattdb_heart_rate_led,
                                                               0,
                                                               1,
                                                               unload_buffer_seq_unwrap.buffer);

                  // Printing the error message if the Server Write Failed fails
                  if (sc != 0)
                    LOG_ERROR("!!! Server Write Failed !!!\nError Code: 0x%x",sc);

                  LOG_INFO("LED Indications sent from buffer : %d indications left in the buffer", (cbfifo_length()/11));

              }
              else if (unload_buffer_seq_unwrap.charHandle == gattdb_heart_rate_measurement)
              {
//                  // For debugging purpose only
//                  printf("Temperature Sending indications");
//                  printf("\nOriginal\n%d\t%d\t%d\t%d\t%d\n%d\n%d\n\n",
//                         unload_buffer_seq_unwrap.buffer[0],
//                         unload_buffer_seq_unwrap.buffer[1],
//                         unload_buffer_seq_unwrap.buffer[2],
//                         unload_buffer_seq_unwrap.buffer[3],
//                         unload_buffer_seq_unwrap.buffer[4],
//                         sizeof(unload_buffer_seq_unwrap.buffer),
//                         unload_buffer_seq_unwrap.charHandle);

                  // Sending indication
                  sc = sl_bt_gatt_server_send_indication(ble_data_ptr->connectionHandle,
                                                         gattdb_heart_rate_measurement,
                                                         unload_buffer_seq_unwrap.bufferLen,
                                                         unload_buffer_seq_unwrap.buffer);

    //              LOG_INFO("Indication Sent");
                  ble_data_ptr->flag_indication_in_progress = true;

                  // Printing the error message if the Sending Indication fails
                  if (sc != 0)
                    LOG_ERROR("!!! Temperature Sending Indication Failed !!!\nError Code: 0x%x",sc);

                  LOG_INFO("Temperature indication sent from buffer : %d indications left in the buffer", (cbfifo_length()/11));
              }
          }
        break;
      }
      break;

    /*******************************External Event*******************************/
    case sl_bt_evt_system_external_signal_id:


      // Checking which external event occurred
      if (evt->data.evt_system_external_signal.extsignals == event_PB0Pressed_hr)
      {
          ble_data_ptr->button_0_flag = !ble_data_ptr->button_0_flag;

          // If not bonded then we will use PB0 for confirmation of passkey
          if (ble_data_ptr->flag_bonded == false && ble_data_ptr->button_0_flag == true) // Trigger on the falling edge of the interrupt
          {
              // Confirming Passkey
              sc = sl_bt_sm_passkey_confirm(ble_data_ptr->connectionHandle, 1);

              // Printing the error message if the passkey confirmation fails
              if (sc != 0)
                LOG_ERROR("!!! Pass Key Confirmation Failed !!!\nError Code: 0x%x",sc);
          }

          #if DEVICE_IS_BLE_SERVER
          /*------------------------------Server Events------------------------------*/
                    // If bonded then we will use PB0 for sending indications to the client
                    if(ble_data_ptr->flag_conection == true /*&&
                       ble_data_ptr->flag_indication_button_state == true &&
                       ble_data_ptr->flag_bonded == true*/)
                    {
                        // For rising edge of the switch
                        if(ble_data_ptr->button_0_flag == true)
                        {
                            ble_data_ptr->button_state_value = 0x01;

                            displayPrintf(DISPLAY_ROW_9, "Button Pressed");

                        }
                        // For falling edge of the switch
                        else
                        {
                            ble_data_ptr->button_state_value = 0x00;

                            displayPrintf(DISPLAY_ROW_9, "Button Released");
                        }
                    }
                    // Default state of button
                    else
                    {
                        ble_data_ptr->button_state_value = 0x00;

                        displayPrintf(DISPLAY_ROW_9, "Button Released");
                    }

                    // Writing the default button state
                    sc = sl_bt_gatt_server_write_attribute_value(gattdb_button_state,
                                                                 0,
                                                                 1,
                                                                 &(ble_data_ptr->button_state_value));

                    // Printing the error message if the Server Write Failed fails
                    if (sc != 0)
                      LOG_ERROR("!!! Server Write Failed !!!\nError Code: 0x%x",sc);

                    if (ble_data_ptr->flag_conection == true &&
                        ble_data_ptr->flag_indication_button_state == true &&
                        ble_data_ptr->flag_bonded == true &&
                        ble_data_ptr->flag_indication_in_progress == true &&
                        cbfifo_length() == cbfifo_capacity())
                     {
                       LOG_ERROR("Buffer Full!! This indication will not be stored and will be lost.");
                     }
                    // If indications are in flight then we will execute the below
                    else if (ble_data_ptr->flag_conection == true &&
                        ble_data_ptr->flag_indication_button_state == true &&
                        ble_data_ptr->flag_bonded == true &&
                        ble_data_ptr->flag_indication_in_progress == true &&
                        cbfifo_length() != cbfifo_capacity())
                    {
                        cb_buffer_load[0] = (uint8_t) ((gattdb_button_state >> 8) & 0x00FF);
                        cb_buffer_load[1] = (uint8_t) ((gattdb_button_state >> 0) & 0x00FF);
                        cb_buffer_load[2] = (uint8_t) ((sizeof(ble_data_ptr->button_state_value) >> 24) & 0x000000FF);
                        cb_buffer_load[3] = (uint8_t) ((sizeof(ble_data_ptr->button_state_value) >> 16) & 0x000000FF);
                        cb_buffer_load[4] = (uint8_t) ((sizeof(ble_data_ptr->button_state_value) >> 8) & 0x000000FF);
                        cb_buffer_load[5] = (uint8_t) ((sizeof(ble_data_ptr->button_state_value) >> 0) & 0x000000FF);
                        cb_buffer_load[6] = ble_data_ptr->button_state_value;
                        cb_buffer_load[7] = 0;
                        cb_buffer_load[8] = 0;
                        cb_buffer_load[9] = 0;
                        cb_buffer_load[10] = 0;

                        // Enqueue to the buffer
                        cbfifo_enqueue(cb_buffer_load, (sizeof(cb_buffer_load)/sizeof(uint8_t)));

                        LOG_INFO("Button indication added to buffer : %d indications left in the buffer", (cbfifo_length()/11));
                    }
                    // If indications are not in flight then send indications
                    else if (ble_data_ptr->flag_conection == true &&
                             ble_data_ptr->flag_indication_button_state == true &&
                             ble_data_ptr->flag_bonded == true &&
                             ble_data_ptr->flag_indication_in_progress == false)
                    {
                        // Sending indication
                        sc = sl_bt_gatt_server_send_indication(ble_data_ptr->connectionHandle,
                                                               gattdb_button_state,
                                                               1,
                                                               &(ble_data_ptr->button_state_value));
              //              LOG_INFO("Indication Sent");
                        ble_data_ptr->flag_indication_in_progress = true;

                        // Printing the error message if the Sending Indication fails
                        if (sc != 0)
                          LOG_ERROR("!!! Sending Indication Failed !!!\nError Code: 0x%x",sc);

                    }
          #else
          /*------------------------------Client Events------------------------------*/

          #endif
      }
      // Event for the PB1 button
      else if (evt->data.evt_system_external_signal.extsignals == event_PB1Pressed_hr)
        {
          ble_data_ptr->button_1_flag = !ble_data_ptr->button_1_flag;
      #if DEVICE_IS_BLE_SERVER
      /*------------------------------Server Events------------------------------*/
      #else
      /*------------------------------Client Events------------------------------*/
                if(!ble_data_ptr->button_1_flag) // Trigger on the falling edge of the interrupt
                {
                    gpioLed0Toggle();
                    // Writing the default button state
                    sc = sl_bt_gatt_read_characteristic_value(ble_data_ptr->connectionHandle,
                                                              ble_data_ptr->myCharacteristicHandle_button_state);

                    // Printing the error message if the Server Write Failed fails
                    if (sc != 0)
                      LOG_ERROR("!!! GATT Read Failed !!!\nError Code: 0x%x",sc);
                }

                if ( ble_data_ptr->button_0_flag &&  ble_data_ptr->button_1_flag) // Both rising edge of the interrupts
                {
                   /**Disabling the Indications**/
                   // Create an advertising set.
                   sc = sl_bt_gatt_set_characteristic_notification(ble_data_ptr->connectionHandle,
                                                                   ble_data_ptr->myCharacteristicHandle_button_state,
                                                                   (!(ble_data_ptr->flag_indication_button_state))*2);
                   // Printing the error message if the Indication Set fails
                   if (sc != 0)
                     LOG_ERROR("!!! Indication Set failed !!!\nError Code: 0x%x",sc);

                   ble_data_ptr->flag_indication_button_state =! (ble_data_ptr->flag_indication_button_state);
                }


      #endif
      }
    break;

    /**************************GATT Procedure Complete**************************/
    case sl_bt_evt_gatt_procedure_completed_id:

      if (evt->data.evt_gatt_procedure_completed.result == 0x110F)
      {

//          printf("Error code is : %x",evt->data.evt_gatt_procedure_completed.result);
          // Writing the default button state
          sc = sl_bt_sm_increase_security(ble_data_ptr->connectionHandle);

          // Printing the error message if the Server Write Failed fails
          if (sc != 0)
            LOG_ERROR("!!! Increasing Security Failed !!!\nError Code: 0x%x",sc);
      }

    break;

    /****************************Confirm Bonded Event***************************/
    case sl_bt_evt_sm_bonded_id:

      displayPrintf(DISPLAY_ROW_CONNECTION, "Bonded");
      displayPrintf(DISPLAY_ROW_PASSKEY, "");
      displayPrintf(DISPLAY_ROW_ACTION, "");

      ble_data_ptr->flag_bonded = true;

      LOG_INFO("- - - Bonding Successful - - -");

#if DEVICE_IS_BLE_SERVER
      /*------------------------------Server Events------------------------------*/
      #if NOP_INDICATION_CONNECTION == 1
            if ((ble_data_ptr->flag_conection == true) && (ble_data_ptr->flag_indication_temp == true) && (ble_data_ptr->flag_bonded == true))
            {
              LETIMER_IntEnable(LETIMER0, LETIMER_IEN_UF);
      //        LOG_INFO("Enable"); // For debugging purpose
            }
            else if ((ble_data_ptr->flag_conection == false) || (ble_data_ptr->flag_indication_temp == false) || (ble_data_ptr->flag_bonded == false))
            {
              LETIMER_IntDisable(LETIMER0, LETIMER_IEN_UF);
      //        LOG_INFO("Disable"); // For debugging purpose
            }
      #endif
      #else
      /*------------------------------Client Events------------------------------*/
#endif

    break;


    /***************************Confirm Pass key Event**************************/
    case sl_bt_evt_sm_confirm_passkey_id:

        displayPrintf(DISPLAY_ROW_PASSKEY, "%06d", evt->data.evt_sm_confirm_passkey.passkey);
        displayPrintf(DISPLAY_ROW_ACTION, "Confirm with PB0");

    break;

    /****************************Bonding Failed Event***************************/
    case sl_bt_evt_sm_bonding_failed_id:

      LOG_ERROR("!!! Bonding Failed !!!\nReason: 0x%x",evt->data.evt_sm_bonding_failed.reason);

      displayPrintf(DISPLAY_ROW_PASSKEY, "");
      displayPrintf(DISPLAY_ROW_ACTION, "");

    break;

    #if DEVICE_IS_BLE_SERVER
    /***************************************************************************/
    /********************************Server Only********************************/
    /***************************************************************************/

    /******************Server Characteristics are Changed Event******************/
    case sl_bt_evt_gatt_server_characteristic_status_id:

    //      LOG_INFO("Changes Noticed"); // For debugging purpose only

          // Temperature handling for indication enable and in flight
          if ((evt->data.evt_gatt_server_characteristic_status.characteristic) == gattdb_heart_rate_measurement && \
              (evt->data.evt_gatt_server_characteristic_status.status_flags) == 0x01) // 1 if Characteristic client configuration has been changed.
          {
    //        ble_data.connectionHandle = evt->data.evt_gatt_server_characteristic_status.characteristic;
            if ((evt->data.evt_gatt_server_characteristic_status.client_config_flags) == 0x02) // sl_bt_gatt_server_client_configuration_t is set to 2 if indications are enabled
            {
                ble_data_ptr->flag_indication_temp = true; // Setting true if there is a change in characteristics and the change is indication flag being set

    //            printf("indication set"); // For debugging purpose

                gpioLed0SetOn();
            }
            else
            {
                ble_data_ptr->flag_indication_temp = false; // Setting true if there is a change in characteristics and the change is indication flag being cleared

    //            printf("indication clear"); // For debugging purpose

                gpioLed0SetOff();

                displayPrintf(DISPLAY_ROW_TEMPVALUE, "");

            }
          }
          else if ((evt->data.evt_gatt_server_characteristic_status.characteristic) == gattdb_heart_rate_measurement && \
              (evt->data.evt_gatt_server_characteristic_status.status_flags) == 0x02) // 2 if Characteristic confirmation has been received
          {
              ble_data_ptr->flag_indication_in_progress = false; // This is a flag that will be set when the indication is acknowledged by the server.
          }

          // Button State handling for indication enable and in flight
          if ((evt->data.evt_gatt_server_characteristic_status.characteristic) == gattdb_heart_rate_led && \
              (evt->data.evt_gatt_server_characteristic_status.status_flags) == 0x01) // 1 if Characteristic client configuration has been changed.
          {
    //        ble_data.connectionHandle = evt->data.evt_gatt_server_characteristic_status.characteristic;
            if ((evt->data.evt_gatt_server_characteristic_status.client_config_flags) == 0x02) // sl_bt_gatt_server_client_configuration_t is set to 2 if indications are enabled
            {
                ble_data_ptr->flag_indication_button_state = true; // Setting true if there is a change in characteristics and the change is indication flag being set

                gpioLed1SetOn();
            }
            else
            {
                ble_data_ptr->flag_indication_button_state = false; // Setting true if there is a change in characteristics and the change is indication flag being cleared

                gpioLed1SetOff();

                displayPrintf(DISPLAY_ROW_9, "Button Released");
            }
          }
          else if ((evt->data.evt_gatt_server_characteristic_status.characteristic) == gattdb_heart_rate_led && \
              (evt->data.evt_gatt_server_characteristic_status.status_flags) == 0x02) // 2 if Characteristic confirmation has been received
          {
              ble_data_ptr->flag_indication_in_progress = false; // This is a flag that will be set when the indication is acknowledged by the server.
              LOG_INFO("Indication Complete");
          }


          // This code will be run only if the temperature state machine is required to run if we have an active connection and the indication are ON. Set NOP_INDICATION_CONNECTION in app.h
        #if NOP_INDICATION_CONNECTION == 1
              if ((ble_data_ptr->flag_conection == true) && (ble_data_ptr->flag_indication_temp == true) && (ble_data_ptr->flag_bonded == true))
              {
                LETIMER_IntEnable(LETIMER0, LETIMER_IEN_UF);
        //        LOG_INFO("Enable"); // For debugging purpose
              }
              else if ((ble_data_ptr->flag_conection == false) || (ble_data_ptr->flag_indication_temp == false) || (ble_data_ptr->flag_bonded == false))
              {
                LETIMER_IntDisable(LETIMER0, LETIMER_IEN_UF);
        //        LOG_INFO("Disable"); // For debugging purpose
              }

        #endif

    break;


    /**********************Server Indication Timeout Event**********************/
    case sl_bt_evt_gatt_server_indication_timeout_id:

        // When the indication times out we will log it and then reset the system
        LOG_ERROR("Sever Indication Time Out. Connection Handle: %d\n\nReconnect to proceed",evt->data.evt_gatt_server_indication_timeout.connection);
        sl_bt_system_reset(sl_bt_system_boot_mode_normal);

    break;

    /***********************Confirm Bonding Request Event***********************/
    case sl_bt_evt_sm_confirm_bonding_id:

      // Confirm Bonding Request
      sc = sl_bt_sm_bonding_confirm(ble_data_ptr->connectionHandle,
                                    1);
      // Printing the error message if the Server Write Failed fails
      if (sc != 0)
        LOG_ERROR("!!! Bonding Confirmation Failed !!!\nError Code: 0x%x",sc);

    break;

    #else
    /***************************************************************************/
    /********************************Client Only********************************/
    /***************************************************************************/

    /************************Client Scanner Report Event************************/
    // This event will get triggered for every advertiser that is found
    case sl_bt_evt_scanner_scan_report_id:

//      // For debugging purpose printing all the devices that were discovered
//      LOG_INFO("\nAdvertiser : %x:%x:%x:%x:%x:%x",
//               evt->data.evt_scanner_scan_report.address.addr[0],
//               evt->data.evt_scanner_scan_report.address.addr[1],
//               evt->data.evt_scanner_scan_report.address.addr[2],
//               evt->data.evt_scanner_scan_report.address.addr[3],
//               evt->data.evt_scanner_scan_report.address.addr[4],
//               evt->data.evt_scanner_scan_report.address.addr[5]);

      // Checking for the server address that is required
      if(serverFound(evt))
      {
          LOG_INFO("\nAdvertiser : %x:%x:%x:%x:%x:%x",
                   evt->data.evt_scanner_scan_report.address.addr[0],
                   evt->data.evt_scanner_scan_report.address.addr[1],
                   evt->data.evt_scanner_scan_report.address.addr[2],
                   evt->data.evt_scanner_scan_report.address.addr[3],
                   evt->data.evt_scanner_scan_report.address.addr[4],
                   evt->data.evt_scanner_scan_report.address.addr[5]);

          /**Stop Scanning**/
          // Stop Scanning
          sc = sl_bt_scanner_stop();
          // Printing the error message if the Scanning Stop fails
          if (sc != 0)
            LOG_ERROR("!!! Scanning Stop Failed !!!\nError Code: 0x%x",sc);


          /**Connection Open**/
          // Opening the connection to the server
          sc = sl_bt_connection_open(evt->data.evt_scanner_scan_report.address,
                                     evt->data.evt_scanner_scan_report.address_type,
                                     PHY,
                                     NULL);
          // Printing the error message if the Connection fails
          if (sc != 0)
            LOG_ERROR("!!! Connection Failed !!!\nError Code: 0x%x",sc);
      }
    break;

    /***************************Service by UUID Event***************************/
    // This event will get triggered only once the service matches the UUID passed
    // in the calling function
    case sl_bt_evt_gatt_service_id:
      if(evt->data.evt_gatt_service.uuid.len == 2)
      {
          ble_data_ptr->myServiceHandle_temp = evt->data.evt_gatt_service.service;
      }
      else if(evt->data.evt_gatt_service.uuid.len == 16)
      {
          ble_data_ptr->myServiceHandle_button_state = evt->data.evt_gatt_service.service;
      }

    break;

    /************************Characteristic by UUID Event************************/
    // This event will get triggered only once the characteristic matches the
    // UUID passed in the calling function
    case sl_bt_evt_gatt_characteristic_id:

      if(evt->data.evt_gatt_characteristic.uuid.len == 2)
      {
          printf("!!!!%u!!!!", evt->data.evt_gatt_characteristic.uuid.len);
          ble_data_ptr->myCharacteristicHandle_temp= evt->data.evt_gatt_characteristic.characteristic;
      }
      else if(evt->data.evt_gatt_characteristic.uuid.len == 16)
      {
          printf("!!!!%u!!!!", evt->data.evt_gatt_characteristic.uuid.len);
          ble_data_ptr->myCharacteristicHandle_button_state = evt->data.evt_gatt_characteristic.characteristic;
      }

    break;

    /*************************Characteristic Value Event*************************/
    case sl_bt_evt_gatt_characteristic_value_id:
//      LOG_INFO("INDICATION_RECIEVED");

      // Checking if the event was triggered by the recipient of indication or any other communication
      if(evt->data.evt_gatt_characteristic_value.att_opcode == sl_bt_gatt_handle_value_indication)
      {
          if((evt->data.evt_gatt_characteristic_value.characteristic) == (ble_data_ptr->myCharacteristicHandle_temp))
          {
//              // For debugging purpose
//              LOG_INFO("0 - %d\n1 - %d\n2 - %d\n3 - %d\n4 - %d",
//                       evt->data.evt_gatt_characteristic_value.value.data[0],
//                       evt->data.evt_gatt_characteristic_value.value.data[1],
//                       evt->data.evt_gatt_characteristic_value.value.data[2],
//                       evt->data.evt_gatt_characteristic_value.value.data[3],
//                       evt->data.evt_gatt_characteristic_value.value.data[4]);


              // Obtaining the characteristic value received from indication.
              float_temp = FLOAT_TO_INT32(evt->data.evt_gatt_characteristic_value.value.data);

              LOG_INFO("The present temperature is: %f C",float_temp);

              displayPrintf(DISPLAY_ROW_TEMPVALUE, "%d C", (uint32_t)float_temp);

              /**Indication Confirmation**/
              // Sending indication confirmation to the server
              sc = sl_bt_gatt_send_characteristic_confirmation(ble_data_ptr->connectionHandle);

              // Printing the error message if the Connection fails
              if (sc != 0)
                LOG_ERROR("!!! Indication Confirmation Failed !!!\nError Code: 0x%x",sc);
          }
          else if((evt->data.evt_gatt_characteristic_value.characteristic) == (ble_data_ptr->myCharacteristicHandle_button_state))
          {
              /**Indication Confirmation**/
              // Sending indication confirmation to the server
              sc = sl_bt_gatt_send_characteristic_confirmation(ble_data_ptr->connectionHandle);

              // Printing the error message if the Connection fails
              if (sc != 0)
                LOG_ERROR("!!! Indication Confirmation Failed !!!\nError Code: 0x%x",sc);

              int_button = evt->data.evt_gatt_characteristic_value.value.data[0];

              if(int_button == 0)
              {
                  displayPrintf(DISPLAY_ROW_9, "Button Released");
              }
              else
              {
                  displayPrintf(DISPLAY_ROW_9, "Button Pressed");
              }
          }
      }
      else if(evt->data.evt_gatt_characteristic_value.att_opcode == sl_bt_gatt_read_response)
      {
          int_button = evt->data.evt_gatt_characteristic_value.value.data[0];

          if(int_button == 0)
          {
              displayPrintf(DISPLAY_ROW_9, "Button Released");
          }
          else
          {
              displayPrintf(DISPLAY_ROW_9, "Button Pressed");
          }
      }
      break;

    #endif
  }
} // handle_ble_event()


/**************************************************************************//**
 * This function performs the comparison in the scanner report. For each
 * advertiser this comparison will run against the #DEFINE of the server
 * address.
 *
 * @param:
 *      evt the structure is passed to access the address of the advertiser
 *
 * @return:
 *      returns true if the address matches else returns false
 *****************************************************************************/
bool serverFound(sl_bt_msg_t *evt)
{
  if((evt->data.evt_scanner_scan_report.address.addr[0] == SERVER_BT_ADDRESS.addr[0]) &&
     (evt->data.evt_scanner_scan_report.address.addr[1] == SERVER_BT_ADDRESS.addr[1]) &&
     (evt->data.evt_scanner_scan_report.address.addr[2] == SERVER_BT_ADDRESS.addr[2]) &&
     (evt->data.evt_scanner_scan_report.address.addr[3] == SERVER_BT_ADDRESS.addr[3]) &&
     (evt->data.evt_scanner_scan_report.address.addr[4] == SERVER_BT_ADDRESS.addr[4]) &&
     (evt->data.evt_scanner_scan_report.address.addr[5] == SERVER_BT_ADDRESS.addr[5]) &&
     (evt->data.evt_scanner_scan_report.address_type == 0 &&
     (evt->data.evt_scanner_scan_report.packet_type == 0)))
  {
    return true;
  }
  else
  {
    return false;
  }
}


/**************************************************************************//**
 * This function performs converts the received buffer into a floating point
 * number.
 *
 * @param:
 *      value_start_little_endian is the pointer to the array. This array is in
 *      little endian format
 *
 * @return:
 *      returns the floating point number after converting from the buffer.
 *****************************************************************************/
float FLOAT_TO_INT32(const uint8_t *value_start_little_endian)
{
  uint8_t signByte = 0;
  int32_t mantissa;
  // input data format is:
  // [0] = flags byte
  // [3][2][1] = mantissa (2's complement)
  // [4] = exponent (2's complement)
  // BT value_start_little_endian[0] has the flags byte
  int8_t exponent = (int8_t)value_start_little_endian[4];
  // sign extend the mantissa value if the mantissa is negative
  if (value_start_little_endian[3] & 0x80)
  { // msb of [3] is the sign of the mantissa
    signByte = 0xFF;
  }
  mantissa = (int32_t) (value_start_little_endian[1] << 0) |
                       (value_start_little_endian[2] << 8) |
                       (value_start_little_endian[3] << 16) |
                       (signByte <<24);
  // value = 10^exponent * mantissa, pow() returns a double type
  return (float) (pow((float)10, (float)exponent) * (float)mantissa);
} // FLOAT_TO_INT32
