/*
 * i2c.c
 *
 *  Created on: 17 Sep 2021
 *      Author: nihalt
 */

#include "i2c.h"

// Include logging for this file
#define INCLUDE_LOG_DEBUG 1
#include "src/log.h"


/**************************************************************************//**
 * GLOBAL Variable Declarations
 *****************************************************************************/
I2C_TransferReturn_TypeDef transferStatus; // A struct that keeps track of the transfer status
I2C_TransferSeq_TypeDef transferSequence; // A struct that stores the transfer sequence that the data/command has to be transfered
uint8_t cmd_data;
uint16_t read_data;
uint8_t received_data[2] = {0}; // An array to store the bits that are being received by the master


/**************************************************************************//**
 * This function initialises the I2C transfer. Sets the appropriate pins and
 * port numbers to perform I2C
 *
 * Refer to the data sheet for the details listed below
 *****************************************************************************/
void i2c_Init()
{
  I2CSPM_Init_TypeDef I2C_Config;
  I2C_Config.port = I2C0,
  I2C_Config.sclPort = SCL_PORT,
  I2C_Config.sclPin = SCL_PIN,
  I2C_Config.sdaPort = SDA_PORT,
  I2C_Config.sdaPin = SDA_PIN,
  I2C_Config.portLocationScl = 14,
  I2C_Config.portLocationSda = 16,
  I2C_Config.i2cRefFreq = 0,
  I2C_Config.i2cMaxFreq = I2C_FREQ_STANDARD_MAX,
  I2C_Config.i2cClhr = i2cClockHLRStandard;

  //  Passing the struct to the initialization function
  I2CSPM_Init(&I2C_Config);

}


/**************************************************************************//**
 * This function sends a command to the bus with the address of the slave
 * and also sends a command that needs to be performed by the slave
 *
 * TrasnferSequence struct is used to make the data in the apt format to send
 * via bus
 *
 * TransferReturn struct is used to initialize the transfer and also store the
 * status of the transfer.
 *****************************************************************************/
void i2c_Write_blocking()
{
  cmd_data = 0xF3; // Measure Temperature, No Hold Master Mode
  transferSequence.flags = I2C_FLAG_WRITE, // Write command
  transferSequence.addr = (Si7021_SLAVE_ADDRESS_TEMP<<1), // Slave address needs to be left shift by one bit
  transferSequence.buf[0].data = &cmd_data, // Passing the pointer that has the command data stored
  transferSequence.buf[0].len = sizeof(cmd_data); // Length of the command data

  // This will initialize the write command on to the bus
  I2C_TransferReturn_TypeDef trans_ret = I2CSPM_Transfer(I2C0,&transferSequence);

  // Checking if the transfer is done or no.
  if(trans_ret != i2cTransferDone)
      {
        LOG_ERROR("I2C Write error"); // If transfer is not done then we will log error message
      }
}


/**************************************************************************//**
 * This function receives the data sent by the slave and stores in a an array
 * this array is then converted into a number based on the number of bits
 * required that is passed
 *
 * TrasnferSequence struct is used to make the data in the apt format to send
 * and receive via bus
 *
 * TransferReturn struct is used to initialize the transfer and also store the
 * status of the transfer.
 *
 * The slave sends out a 14 bit data for the command sent above. the first byte
 * received is the MS Byte and the second in the LS Byte
 *
 * @param:
 *      len is the number of bytes that is expected to be received from the
 *      slave.
 *
 * @return:
 *      returns the temperature in degree celsius
 *****************************************************************************/
uint16_t i2c_Read_blocking(uint8_t len)
{
  uint16_t tempvalue=0; // A variable to store the temperature value
  uint8_t received_data[2] = {0}; // An array to store the bits that are being received by the master
  transferSequence.flags = I2C_FLAG_READ, // Read command
  transferSequence.addr = (Si7021_SLAVE_ADDRESS_TEMP<<1), // Slave address needs to be left shift by one bit
  transferSequence.buf[0].data = received_data, // Passing the array that will store the incoming data
  transferSequence.buf[0].len = sizeof(received_data); // Passing the size of the array

  // We will have to wait for 10.8ms for the 14 bit data to be received byt the master
  timerWaitUs_blocking(10800); // This is the amount of time that the sensor takes to transfer 14 bits of read data to the master

  // Initiating the transfer for the master to receive the data from the bus
  I2C_TransferReturn_TypeDef trans_ret = I2CSPM_Transfer(I2C0, &transferSequence);


  // If the transfer is not done then log an error
  if(trans_ret != i2cTransferDone)
  {
    LOG_ERROR("I2C Read error");
  }
  // If len == 1 then the single byte of data is stored as temperature
  if(len==1)
  {
    tempvalue = received_data[0];
  }
  else // If there are two bytes that are received then we will left shift the first byte of data by 8 bits and then OR with the second byte
  {
    tempvalue = received_data[0];
    tempvalue <<= 8;
    tempvalue |=(received_data[1]);
  }

//  printf("\nThe Temperature is : %f C ", (convertToCelcius(tempvalue))); // For debugging purpose only

//  LOG_INFO("\nThe Temperature is : %f C in the logger", ((convertToCelcius(tempvalue)))); // Logs the temperature

  return tempvalue;
}


/**************************************************************************//**
 * This function sends a command to the bus with the address of the slave
 * and also sends a command that needs to be performed by the slave
 *
 * TrasnferSequence struct is used to make the data in the apt format to send
 * via bus
 *
 * TransferReturn struct is used to initialize the transfer and also store the
 * status of the transfer.
 *****************************************************************************/
void i2c_Write_Read_blocking (uint8_t reg, uint8_t* read_data, size_t nbytes_read_data)
{
  transferSequence.flags = I2C_FLAG_WRITE_READ, // Write command
  transferSequence.addr = (MAX_30101_ADDRESS<<1), // Slave address needs to be left shift by one bit
  transferSequence.buf[0].data = &reg, // Passing the pointer that has the command data stored
  transferSequence.buf[0].len = sizeof(reg); // Length of the command data
  transferSequence.buf[1].data = read_data, // Passing the pointer that has the command data stored
  transferSequence.buf[1].len = nbytes_read_data;

  // This will initialize the write command on to the bus
  I2C_TransferReturn_TypeDef trans_ret = I2CSPM_Transfer(I2C0,&transferSequence);

  // Checking if the transfer is done or no.
  if(trans_ret != i2cTransferDone)
      {
        LOG_ERROR("I2C Write error: %d", trans_ret); // If transfer is not done then we will log error message
        createEventSystemError();
      }
//  for (int i = 0; i < nbytes_read_data; i++)
//    {
//      printf("Reg 0x%02x :: Read %d:0x%02x\t", reg, i+1, *(read_data+i));
//    }
//  printf("\n\n");
}


void i2c_Write_Read (uint8_t reg, uint8_t* read_data, size_t nbytes_read_data)
{
  i2c_Init();

  transferSequence.flags = I2C_FLAG_WRITE_READ, // Write command
  transferSequence.addr = (MAX_30101_ADDRESS<<1), // Slave address needs to be left shift by one bit
  transferSequence.buf[0].data = &reg, // Passing the pointer that has the command data stored
  transferSequence.buf[0].len = sizeof(reg); // Length of the command data
  transferSequence.buf[1].data = read_data, // Passing the pointer that has the command data stored
  transferSequence.buf[1].len = nbytes_read_data;

  NVIC_SetPriority(I2C0_IRQn, 2);

  NVIC_EnableIRQ(I2C0_IRQn);

  // This will initialize the write command on to the bus
  I2C_TransferReturn_TypeDef transferStatus = I2C_TransferInit(I2C0, &transferSequence);

  if ((transferStatus != i2cTransferDone) && (transferStatus != i2cTransferInProgress))
  {
      LOG_ERROR("I2C Write Error code: %d", transferStatus);
//      state_machine_temp (event_Error);
  }



}

/**************************************************************************//**
 * This function sends a command to the bus with the address of the slave
 * and also sends a command that needs to be performed by the slave
 *
 * TrasnferSequence struct is used to make the data in the apt format to send
 * via bus
 *
 * TransferReturn struct is used to initialize the transfer and also store the
 * status of the transfer.
 *****************************************************************************/
void i2c_Write_Write_blocking (uint8_t reg, uint8_t* write_data, size_t nbytes_write_data)
{
  transferSequence.flags = I2C_FLAG_WRITE_WRITE, // Write command
  transferSequence.addr = (MAX_30101_ADDRESS<<1), // Slave address needs to be left shift by one bit
  transferSequence.buf[0].data = &reg, // Passing the pointer that has the command data stored
  transferSequence.buf[0].len = sizeof(reg); // Length of the command data
  transferSequence.buf[1].data = write_data, // Passing the pointer that has the command data stored
  transferSequence.buf[1].len = nbytes_write_data;

  // This will initialize the write command on to the bus
  I2C_TransferReturn_TypeDef trans_ret = I2CSPM_Transfer(I2C0,&transferSequence);

  // Checking if the transfer is done or no.
  if(trans_ret != i2cTransferDone)
      {
        LOG_ERROR("I2C Write error: %d", trans_ret); // If transfer is not done then we will log error message
        createEventSystemError();
      }
//  for (int i =0; i < nbytes_write_data; i++)
//    {
//      printf("Reg 0x%02x :: Write %d:0x%02x\t", reg, i+1, *(write_data+i));
//    }
//  printf("\n");
}

/**************************************************************************//**
 * This function sends a command to the bus with the address of the slave
 * and also sends a command that needs to be performed by the slave
 *
 * TrasnferSequence struct is used to make the data in the apt format to send
 * via bus
 *
 * TransferReturn struct is used to initialize the transfer and also store the
 * status of the transfer.
 *
 * This is interrupt driven function. The interrupt is taken care of at
 * I2C0_IRQHandler.
 *
 *  * @param:
 *      no params
 *
 * @return:
 *      no returns
 *****************************************************************************/
void i2c_Write()
{
  i2c_Init();

  cmd_data = 0xF3; // Measure Temperature, No Hold Master Mode
  transferSequence.flags = I2C_FLAG_WRITE, // Write command
  transferSequence.addr = (Si7021_SLAVE_ADDRESS_TEMP<<1), // Slave address needs to be left shift by one bit
  transferSequence.buf[0].data = &cmd_data, // Passing the pointer that has the command data stored
  transferSequence.buf[0].len = sizeof(cmd_data); // Length of the command data


  NVIC_EnableIRQ(I2C0_IRQn);


  // This will initialize the write command on to the bus
  I2C_TransferReturn_TypeDef transferStatus = I2C_TransferInit(I2C0, &transferSequence);

  if ((transferStatus != i2cTransferDone) && (transferStatus != i2cTransferInProgress))
  {
      LOG_ERROR("I2C Write Error code: %d", transferStatus);
//      state_machine_temp (event_Error);
  }
}

/**************************************************************************//**
 * This function receives the data sent by the slave and stores in a an array
 * this array is then converted into a number based on the number of bits
 * required that is passed
 *
 * TrasnferSequence struct is used to make the data in the apt format to send
 * and receive via bus
 *
 * TransferReturn struct is used to initialize the transfer and also store the
 * status of the transfer.
 *
 * The slave sends out a 14 bit data for the command sent above. the first byte
 * received is the MS Byte and the second in the LS Byte
 *
 * @param:
 *      len is the number of bytes that is expected to be received from the
 *      slave.
 *
 * @return:
 *      returns the temperature in degree celsius
 *****************************************************************************/
void i2c_Read()
{
  i2c_Init();

  transferSequence.flags = I2C_FLAG_READ, // Read command
  transferSequence.addr = (Si7021_SLAVE_ADDRESS_TEMP<<1), // Slave address needs to be left shift by one bit
  transferSequence.buf[0].data = &received_data[0], // Passing the array that will store the incoming data
  transferSequence.buf[0].len = sizeof(received_data); // Passing the size of the array


  NVIC_EnableIRQ(I2C0_IRQn);

  // Initiating the transfer for the master to receive the data from the bus
  transferStatus = I2C_TransferInit(I2C0, &transferSequence);

  if ((transferStatus != i2cTransferDone) && (transferStatus != i2cTransferInProgress))
  {
      LOG_ERROR("I2C Read Error code: %d", transferStatus);
//      state_machine_temp (event_Error);
  }
}


/**************************************************************************//**
 * This is a function that takes in the two bytes that were received and will
 * convert it into a decimal number.
 *
 * @param:
 *      len is the number of bytes that is expected to be received from the
 *      slave.
 *
 * @return:
 *      returns the decimal equivalent of the data received
 *****************************************************************************/
uint16_t concatenatingBytes(uint8_t len)
{
  // DOS: Yikes : you are starting another I2C transfer !!!!, No no no
  //I2C_TransferReturn_TypeDef trans_ret = I2C_TransferInit(I2C0, &transferSequence);


  // If len == 1 then the single byte of data is stored as temperature
  if(len==1)
  {
      read_data = received_data[0];
  }
  else // If there are two bytes that are received then we will left shift the first byte of data by 8 bits and then OR with the second byte
  {
      read_data = received_data[0];
      read_data <<= 8;
      read_data |=(received_data[1]);
  }

//  printf("\nThe Temperature is : %f C ", (convertToCelcius(tempvalue))); // For debugging purpose only

//  LOG_INFO("\nThe Temperature is : %f C in the logger", ((convertToCelcius(tempvalue)))); // Logs the temperature

  return read_data;
}


/**************************************************************************//**
 * This function converts the return value from the sensor and converts it into
 * degrees. This function converts the 16 bit number to a float value and
 * returns a decimal number
 *
 * @param:
 *      val is the raw reading from the sensor (16 bit)
 *      scale is a char
 *        C for Celsius
 *        F for Fahrenheit
 *
 * @return:
 *      returns the temperature in degree celsius (floating point)
 *****************************************************************************/
float convertToDegrees (uint16_t val, char scale)
{
  float float_val = ((float) val);
  switch (scale)
  {
    case('C'):
      float_val = (((175.72*val)/65536)-46.85);
      return float_val;
    break;

    case('F'):
          float_val = (((((175.72*val)/65536)-46.85)*1.8)+32);
          return float_val;
    break;
  }
  return 0.00;
}


/**************************************************************************//**
 * This function simply does the entire I2C stitching for us and gets us a
 * floating point temperature in degrees as per the users entry
 *
 * @param:
 *      scale is a char
 *        C for Celsius
 *        F for Fahrenheit
 *
 * @return:
 *      returns the temperature in degree celsius (floating point)
 *****************************************************************************/
float getTempReadingSi7021(char scale)
{
  i2c_Write_blocking();

  int16_t temp = i2c_Read_blocking(2);

  return convertToDegrees(temp, scale); // Returns the temperature in desired scale
}
