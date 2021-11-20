/*
 * i2c.h
 *
 *  Created on: 17 Sep 2021
 *      Author: nihalt
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


// Function Definitions
void i2c_Init(); // Function to initialize the I2C protocol
void sensorEnable(); // Function to enable the sensor
void i2c_Write_blocking(); // Function to write commands to the slave - Interrupt based
uint16_t i2c_Read_blocking(uint8_t len); // Function to write commands to the slave - Interrupt based
void i2c_Write(); // Function to write commands to the slave - Interrupt based
void i2c_Read(); // Function to read the data sent by the slave - Interrupt based
float convertToDegrees (uint16_t val, char scale); // Function to convert the values into degrees
float getTempReadingSi7021(char scale);
uint16_t concatenatingBytes(uint8_t len); // Concatenates the bytes that were recieved from the I2C Read

#endif /* SRC_I2C_H_ */
