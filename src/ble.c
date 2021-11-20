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
  ble_data_ptr->flag_indication = false;
  ble_data_ptr->flag_indication_in_progress= false;
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
void state_machine_ble(sl_bt_msg_t *evt) {

  // Variable declaration
  sl_status_t sc;

  ble_data_struct_t *ble_data_ptr = getBleDataPtr(); // Getting the pointer to the ble_data_ptr

  uint8_t system_id[8];

  switch (SL_BT_MSG_ID(evt->header))
  {
    /****************************Boot Event****************************/
    case sl_bt_evt_system_boot_id:
//      LOG_INFO("System Boot Event"); // For debugging purpose

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

    break;

    /*************************Connection Open Event*************************/
    // Once the connection to the server is initiated this id is activated
    case sl_bt_evt_connection_opened_id: // handle open event
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

      // This code will be run only if the temperature state machine is required to run if we have an active connection and the indication are ON
#if NOP_INDICATION_CONNECTION == 1
      if ((ble_data_ptr->flag_conection == true) && (ble_data_ptr->flag_indication == true))
      {
          LETIMER_IntEnable(LETIMER0, LETIMER_IEN_UF);
//          LOG_INFO("Enabled"); // For debugging purpose
      }
#endif

    break;

    /*************************Connection Close Event*************************/
    case sl_bt_evt_connection_closed_id: // handle close event
//      LOG_INFO("Connection Closed");  // For debugging purpose

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
      if ((ble_data_ptr->flag_conection == false) || (ble_data_ptr->flag_indication == false))
        {
          LETIMER_IntDisable(LETIMER0, LETIMER_IEN_UF);
//          LOG_INFO("Disabled");
          sl_bt_gatt_set_characteristic_notification(evt->data.evt_connection_opened.connection,
                                                     gattdb_temperature_measurement,
                                                     0);
          ble_data_ptr->flag_indication = false;
        }
#endif

    break;

    /***********************Connection Parameters Are Set***********************/
    case sl_bt_evt_connection_parameters_id:

      // Uncomment this to get the parameter set values
//      LOG_INFO("\n\nExpected:\nConnection Interval:75ms\nSlave Latency: 3\nSupervision Timeout: 600ms or above\n\n");
//      LOG_INFO("\n\nActual:\nConnection Interval:%dms\nSlave Latency: %d\nSupervision Timeout: %d\n\n",
//               (int)((evt->data.evt_connection_parameters.interval*1.25)),
//               ((int)(evt->data.evt_connection_parameters.latency)),
//                (int)((evt->data.evt_connection_parameters.timeout*10)));

    break;


    /******************Server Characteristics are changed Event******************/
    case sl_bt_evt_gatt_server_characteristic_status_id:
//      LOG_INFO("Changes Noticed"); // For debugging purpose only

      if ((evt->data.evt_gatt_server_characteristic_status.characteristic) == gattdb_temperature_measurement && \
          (evt->data.evt_gatt_server_characteristic_status.status_flags) == 0x01) // 1 if Characteristic client configuration has been changed.
      {
        ble_data.connectionHandle = evt->data.evt_gatt_server_characteristic_status.characteristic;
        if ((evt->data.evt_gatt_server_characteristic_status.client_config_flags) == 0x02) // sl_bt_gatt_server_client_configuration_t is set to 2 if indications are enabled
        {
            ble_data_ptr->flag_indication = true; // Setting true if there is a change in characteristics and the change is indication flag being set

//            printf("indication set"); // For debugging purpose
        }
        else
        {
            ble_data_ptr->flag_indication = false; // Setting true if there is a change in characteristics and the change is indication flag being cleared

//            printf("indication clear"); // For debugging purpose
        }
      }
      else if ((evt->data.evt_gatt_server_characteristic_status.characteristic) == gattdb_temperature_measurement && \
          (evt->data.evt_gatt_server_characteristic_status.status_flags) == 0x02) // 2 if Characteristic confirmation has been received
      {
          ble_data_ptr->flag_indication_in_progress = false; // This is a flag that will be set when the indication is acknowledged by the server.
      }

      // This code will be run only if the temperature state machine is required to run if we have an active connection and the indication are ON. Set NOP_INDICATION_CONNECTION in app.h
#if NOP_INDICATION_CONNECTION == 1
      if ((ble_data_ptr->flag_conection == true) && (ble_data_ptr->flag_indication == true))
      {
        LETIMER_IntEnable(LETIMER0, LETIMER_IEN_UF);
//        LOG_INFO("Enable"); // For debugging purpose
      }
      else if ((ble_data_ptr->flag_conection == false) || (ble_data_ptr->flag_indication == false))
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

  }
} // handle_ble_event()
