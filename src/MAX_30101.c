/*
 * MAX30101.c
 *
 *  Modified on: 8 Dec 2021
 *      Author:
 *          Server Code: Nihal T
 *
 */

#include "MAX_30101.h"
#include <stdint.h>
#include "timers.h"
#include <stdio.h>

/**************************************************************************//**
 * This function initializes the sensor registers and initiates the sensor to
 * start taking readings
 *
 * @param:
 *      no params
 * @return:
 *      no params
 *****************************************************************************/
void MAX_30101_Init()
{
  // Checking initial values
  uint8_t write = 0xff, read;
  i2c_Write_Read_blocking(0x08, &read, sizeof(read));
  i2c_Write_Read_blocking(0x09, &read, sizeof(read));
  i2c_Write_Read_blocking(0x0C, &read, sizeof(read));
  i2c_Write_Read_blocking(0x11, &read, sizeof(read));
  i2c_Write_Read_blocking(0x00, &read, sizeof(read));


  write = 0x51;
  i2c_Write_Write_blocking(0x08, &write, sizeof(write));
  timerWaitUs_blocking(5000);

  write = 0x02;
  i2c_Write_Write_blocking(0x09, &write, sizeof(write));
  timerWaitUs_blocking(5000);

  write = 0x4F;// Much better
  i2c_Write_Write_blocking(0x0C, &write, sizeof(write));
  timerWaitUs_blocking(5000);

  write = 0x11;// Working
  i2c_Write_Write_blocking(0x11, &write, sizeof(write));
  timerWaitUs_blocking(5000);

  write = 0x80; // For full buffer
  i2c_Write_Write_blocking(0x02, &write, sizeof(write));
  timerWaitUs_blocking(5000);

  write = 0x1F;
  i2c_Write_Write_blocking(0x0A, &write, sizeof(write));
  timerWaitUs_blocking(5000);

  // Checking final values
  i2c_Write_Read_blocking(0x08, &read, sizeof(read));
  i2c_Write_Read_blocking(0x09, &read, sizeof(read));
  i2c_Write_Read_blocking(0x0C, &read, sizeof(read));
  i2c_Write_Read_blocking(0x11, &read, sizeof(read));
}

/**************************************************************************//**
 * This function gets the value that is stored in a particular register in the
 * MAX30101 sensor
 *
 * @param:
 *      reg:              The register that you wish to read the value from
 *      read_data:        Pointer to the data that is read from the sensor
 *      nbytes_read_data: Number of bytes that is read from the sensor with
 *                        read_data as the base pointer
 *
 * @return:
 *      no params
 *****************************************************************************/
void MAX_30101_Get_Reg_Val (uint8_t reg, uint8_t* read_data, size_t nbytes_read_data)
{
  for (int i = 0; i < (int)nbytes_read_data; i++)
    {
      printf("Reg 0x%02x :: Read %d:0x%02x\t", reg, i+1, *(read_data+i));
    }
  printf("\n\n");
}


/**************************************************************************//**
 * This function shuts the sensor down. This functions will retain the register
 * values while just saving power. Upon MAX_30101_PowerUp() the sensor will
 * start taking readings with the same configurations
 *
 * @param:
 *      no params
 *
 * @return:
 *      no params
 *****************************************************************************/
void MAX_30101_ShutDown()
{
  uint8_t write = 0x82; // Shutdown
  i2c_Write_Write_blocking(0x09, &write, sizeof(write));
  timerWaitUs_blocking(5000);
}


/**************************************************************************//**
 * This function powers the sensor up. This functions will retain the register
 * values while just saving power. Upon MAX_30101_PowerUp() the sensor will
 * start taking readings with the same configurations
 *
 * @param:
 *      no params
 *
 * @return:
 *      no params
 *****************************************************************************/
void MAX_30101_PowerUp()
{
  uint8_t write = 0x02; // Power Up
  i2c_Write_Write_blocking(0x09, &write, sizeof(write));
  timerWaitUs_blocking(5000);
}


/**************************************************************************//**
 * This function resets the sensor and sets all the registers back to its
 * default values.
 *
 * @param:
 *      no params
 *
 * @return:
 *      no params
 *****************************************************************************/
void MAX_30101_Reset()
{
  uint8_t write = 0xC2; // Reset
  i2c_Write_Write_blocking(0x09, &write, sizeof(write));
  timerWaitUs_blocking(5000);
}





