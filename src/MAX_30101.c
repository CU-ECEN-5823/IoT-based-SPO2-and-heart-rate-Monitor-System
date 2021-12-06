/*
 * MAX_30101.c
 *
 *  Created on: 1 Dec 2021
 *      Author: nihalt
 */

#include "MAX_30101.h"
#include <stdint.h>
#include "timers.h"

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

//  write = 0x0F;
//  write = 0xFF;// Much better

  write = 0x4F;// Much better
  i2c_Write_Write_blocking(0x0C, &write, sizeof(write));
  timerWaitUs_blocking(5000);

  write = 0x11;// Working
  i2c_Write_Write_blocking(0x11, &write, sizeof(write));
  timerWaitUs_blocking(5000);

  write = 0x80; // For full buffer
//  write = 0x60; // For each reading
  i2c_Write_Write_blocking(0x02, &write, sizeof(write));
  timerWaitUs_blocking(5000);

//  write = 0x0F;// Working
  write = 0x1F;
  i2c_Write_Write_blocking(0x0A, &write, sizeof(write));
  timerWaitUs_blocking(5000);

  // Checking final values

  i2c_Write_Read_blocking(0x08, &read, sizeof(read));
  i2c_Write_Read_blocking(0x09, &read, sizeof(read));
  i2c_Write_Read_blocking(0x0C, &read, sizeof(read));
  i2c_Write_Read_blocking(0x11, &read, sizeof(read));

}

void MAX_30101_Get_Reg_Val (uint8_t reg, uint8_t* read_data, size_t nbytes_read_data)
{
  for (int i = 0; i < nbytes_read_data; i++)
    {
      printf("Reg 0x%02x :: Read %d:0x%02x\t", reg, i+1, *(read_data+i));
    }
  printf("\n\n");
}

void MAX_30101_ShutDown()
{
  uint8_t write = 0x82; // Shutdown
  i2c_Write_Write_blocking(0x09, &write, sizeof(write));
  timerWaitUs_blocking(5000);

//  write = 0xC2; // Reset
//  i2c_Write_Write_blocking(0x09, &write, sizeof(write));
//  timerWaitUs_blocking(5000);
}

void MAX_30101_PowerUp()
{
  uint8_t write = 0x02; // Shutdown
  i2c_Write_Write_blocking(0x09, &write, sizeof(write));
  timerWaitUs_blocking(5000);

//  write = 0xC2; // Reset
//  i2c_Write_Write_blocking(0x09, &write, sizeof(write));
//  timerWaitUs_blocking(5000);
}


void MAX_30101_Reset()
{
  uint8_t write = 0xC2; // Reset
  i2c_Write_Write_blocking(0x09, &write, sizeof(write));
  timerWaitUs_blocking(5000);
}





