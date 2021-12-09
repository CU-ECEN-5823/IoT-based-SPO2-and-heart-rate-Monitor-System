/*
 * i2c.h
 *
 *  Modified on: 8 Dec 2021
 *      Author:
 *          Author 1: Nihal T
 *
 */

#ifndef SRC_I2C_H_
#define SRC_I2C_H_

#include "sl_i2cspm.h"
#include "gpio.h"
#include "em_cmu.h"
#include <em_i2c.h>
#include "sl_i2cspm_instances.h"
#include "timers.h"

// Address of the Si7021 temperature sensor (refer to data sheet)
#define Si7021_SLAVE_ADDRESS_TEMP 0x40

#define MAX_30101_ADDRESS 0x57 // Works fine


// Function Definitions
void i2c_Init(); // Function to initialize the I2C protocol
void sensorEnable(); // Function to enable the sensor
void i2c_Write_blocking(); // Function to write commands to the slave - Interrupt based
uint16_t i2c_Read_blocking(uint8_t len); // Function to write commands to the slave - Interrupt based
void i2c_Write(); // Function to write commands to the slave - Interrupt based
void i2c_Read(); // Function to read the data sent by the slave - Interrupt based
void i2c_Write_Read_blocking (uint8_t reg, uint8_t* read_data, size_t nbytes_read_data);
void i2c_Write_Write_blocking (uint8_t reg, uint8_t* write_data, size_t nbytes_write_data);
void i2c_Write_Read (uint8_t reg, uint8_t* read_data, size_t nbytes_read_data);


#endif /* SRC_I2C_H_ */
