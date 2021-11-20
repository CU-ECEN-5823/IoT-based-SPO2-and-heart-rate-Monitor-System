/*
 * ble.h
 *
 *  Created on: 1 Oct 2021
 *      Author: nihalt
 *      Attribute: Prof. David Sluiter
 *                 UINT8_TO_BITSTREAM
 *                 UINT32_TO_BITSTREAM
 *                 UINT32_TO_FLOAT
 */

#ifndef SRC_BLE_H_
#define SRC_BLE_H_

// Including header files
#include "sl_bt_api.h"
#include <stdint.h>
#include "sl_bluetooth.h"
#include "gatt_db.h"
#include "timers.h"
#include "app.h"


// Macros
#define UINT8_TO_BITSTREAM(p, n)        { *(p)++ = (uint8_t)(n); } // Converts the 8 bit data into a bit stream
#define UINT32_TO_BITSTREAM(p, n)       { *(p)++ = (uint8_t)(n); *(p)++ = (uint8_t)((n) >> 8); \
                                          *(p)++ = (uint8_t)((n) >> 16); *(p)++ = (uint8_t)((n) >> 24); } // Converts the 32 bit data into a bit stream
#define UINT32_TO_FLOAT(m, e)           (((uint32_t)(m) & 0x00FFFFFFU) | (uint32_t)((int32_t)(e) << 24)) // Converts the 32 bit data into float

// A structure to store the variables and flags
typedef struct {
  bd_addr myAddress;
  uint8_t myAddressType;

  uint8_t advertisingSetHandle;
  uint8_t connectionHandle;

  bool flag_conection;
  bool flag_indication;
  bool flag_indication_in_progress;
} ble_data_struct_t;

void state_machine_ble(sl_bt_msg_t *evt); // Function deals with the blue booting and state machine
ble_data_struct_t* getBleDataPtr(); // Function to get the pointer to the function ble_data_struct_t
void ble_Init(void); // Initializing the blue tooth event

#endif /* SRC_BLE_H_ */
