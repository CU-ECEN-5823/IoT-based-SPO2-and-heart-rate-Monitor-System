/*
 * irq.c
 *
 *  Created on: 10 Sep 2021
 *      Author: nihalt
 */

#include "irq.h"
#include "app.h"

#include "scheduler.h"

#include <stdio.h>


// Include logging for this file
#define INCLUDE_LOG_DEBUG 1
#include "src/log.h"

uint8_t cycles=0;
int32_t flags;

//Added for Assignment - 2
/**************************************************************************//**
 * This function receives the data sent by the slave and stores in a an array
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
 *      returns the temperature in degree celcius
 *****************************************************************************/
//void LETIMER0_IRQInit()
//{
//  // LETIMER0 is the register and we are enabling the Underflow Interrupt using LETIMER_IEN_UF bit
//  LETIMER_IntEnable(LETIMER0, LETIMER_IEN_UF);
//
////  // LETIMER0 is the register and we are enabling the Underflow Interrupt using LETIMER_IEN_COMP1 bit
////  LETIMER_IntEnable(LETIMER0, LETIMER_IEN_COMP1);
//  LETIMER_IntDisable(LETIMER0, LETIMER_IEN_COMP1); // By default COMP1 is enabled so the timer used to get switched OFF rather than turning ON
//
//  I2C_IntEnable(I2C0,I2C_IEN_MSTOP);
//}

/**************************************************************************//**
 * This function initiates all the required NVICs
 *
 * @param:
 *      no params
 *
 * @return:
 *      no returns
 *****************************************************************************/
void NVIC_Init(void)
{
  NVIC_ClearPendingIRQ(LETIMER0_IRQn); // We will clear all pending flags in the NVIC for LETIMER
  NVIC_SetPriority(LETIMER0_IRQn, 16);
  NVIC_EnableIRQ(LETIMER0_IRQn); // Enabling the NVIC IRQ

  NVIC_ClearPendingIRQ(GPIO_EVEN_IRQn); // We will clear all pending flags in the NVIC for GPIO Even pins
  NVIC_ClearPendingIRQ(GPIO_ODD_IRQn); // We will clear all pending flags in the NVIC for GPIO Odd pins

  NVIC_SetPriority(GPIO_EVEN_IRQn, 4);
  NVIC_EnableIRQ(GPIO_EVEN_IRQn); // Enabling the NVIC IRQ
  NVIC_SetPriority(GPIO_ODD_IRQn, 8);
  NVIC_EnableIRQ(GPIO_ODD_IRQn); // Enabling the NVIC IRQ
}


//Added for Assignment - 2
/**************************************************************************//**
 * This function handles when an interrupt flag is raised
 * This code will run only when the event occurs and the interrupt flag is
 * raised
 * Once the event occurs and the interrupt is raised we will call a function
 * to create a function to schedule and event in the scheduler
 * @param:
 *      no params
 *
 * @return:
 *      no returns
 *****************************************************************************/
void LETIMER0_IRQHandler()
{
  int32_t flags;
  flags = LETIMER_IntGetEnabled(LETIMER0); // Storing the interrupt enabled register value in another variable so that the bits can be punched down

  LETIMER_IntClear(LETIMER0, flags); // Clearing the interrupt flags


  // Removed for assignment 3 - Start
  // Identifying the source of interrupt and then acting accordingly

//  if (flags & LETIMER_IF_UF) // Underflow Flag
//  {
//      schedulerSetLEDOFF(); // Setting an event to OFF LED0 when underflow occurs
//  }
//  if (flags & LETIMER_IF_COMP1) // Overflow Flag
//  {
//      schedulerSetLEDON(); // Setting an event to ON LED0 when overflow occurs
//  }
  // Removed for assignment 3 - End

//  Added for Assignment - 3
//  The below calls a function to create an event to measure temperature from Si7021
  if (flags & LETIMER_IF_UF) // Underflow Flag
  {
//      LOG_INFO("UF Interrupt");
      cycles++;
      createEventMeasureTempSi7021(); // Function for creating an event to measure temperature
  }

  if (flags & LETIMER_IF_COMP1) // Overflow Flag
  {
//      LOG_INFO("COMP1 Interrupt"); // Be very carefull while using this as they put the CPU to sleep when in EM!=0
//      LOG_INFO("Stop Timmer"); // Be very carefull while using this as they put the CPU to sleep when in EM!=0
      LETIMER_IntDisable(LETIMER0, LETIMER_IEN_COMP1); // Disabling the interrupt for COMP1
      createEventTimerWaitUs_IRQ(); // Function for creating an event to say that the timer is up
  }
}


// Added for assignment 4
/**************************************************************************//**
 * This is a function that returns the total number of milliseconds after the
 * board booted up. For a 32 bit variable we can count up to 24 days before
 * overflow.
 *
 * @param:
 *      len is the number of bytes that is expected to be received from the
 *      slave.
 *
 * @return:
 *      returns the temperature in degree celcius
 *****************************************************************************/
uint32_t letimerMilliseconds()
{
  uint32_t time_lapsed=0;
  uint32_t temp1 = (cycles*LETIMER_PERIOD_MS);
  uint32_t temp2 = ((LETIMER_CompareGet(LETIMER0,0)-LETIMER_CounterGet(LETIMER0))*1000)/CMU_ClockFreqGet(cmuClock_LETIMER0);
//  time_lapsed = (cycles*LETIMER_PERIOD_MS)+(((LETIMER_CompareGet(LETIMER0,0)-LETIMER_CounterGet(LETIMER0))*1000)/CMU_ClockFreqGet(cmuClock_LETIMER0));
  time_lapsed = temp1+temp2;
  return time_lapsed;
}



// Added for assignment 4
/**************************************************************************//**
 * This function handles when an interrupt flag is raised
 *
 * This code will run only when the event occurs and the interrupt flag is
 * raised
 *
 * Once the event occurs and the interrupt is raised we will call a function
 * to create a function to schedule and event in the scheduler
 *
 * @param:
 *      no params
 *
 * @return:
 *      no returns
 *****************************************************************************/
void I2C0_IRQHandler(void)
{




  flags = I2C_IntGetEnabled(I2C0);


//  I2C_TransferReturn_TypeDef trans_ret = I2C_Transfer(I2C0);
//
//  // Checking if the transfer is done or no.
//  if(trans_ret == i2cTransferDone)
//  {
//      NVIC_DisableIRQ(I2C0_IRQn);
//      createEventI2CTransfer();
//  }
//  else if ((trans_ret != i2cTransferDone) && (trans_ret != i2cTransferInProgress))
//  {
//      LOG_ERROR("I2C Error code: %d\n", trans_ret);
//      createEventErrorTemp();
//  }
//  I2C_IntClear(I2C0, flags);

}

void I2C_event(void)
{

  if(flags != 0)
    {
      I2C_TransferReturn_TypeDef trans_ret = I2C_Transfer(I2C0);

    // Checking if the transfer is done or no.
    if(trans_ret == i2cTransferDone)
    {
        NVIC_DisableIRQ(I2C0_IRQn);
        createEventI2CTransfer();
    }
    else if ((trans_ret != i2cTransferDone) && (trans_ret != i2cTransferInProgress))
    {
        LOG_ERROR("I2C Error code: %d\n", trans_ret);
        createEventErrorTemp();
    }
      I2C_IntClear(I2C0, flags);
    }
  flags = 0;
}


/**************************************************************************//**
 * This function handles when the even numbered GPIO pin interrupts are
 * triggered
 *
 * This code will run only when the event occurs and the interrupt flag is
 * raised
 *
 * Once the event occurs and the interrupt is raised we will call a function
 * to create a function to schedule and event in the scheduler
 *
 * @param:
 *      no params
 *
 * @return:
 *      no returns
 *****************************************************************************/
void GPIO_EVEN_IRQHandler(void)
{
  int32_t flags;

  flags = GPIO_IntGetEnabled();

  GPIO_IntClear(flags);

  createEventPB0Pressed();

//  gpioLed1Toggle(); // For Debugging purpose only
}

/**************************************************************************//**
 * This function handles when the odd numbered GPIO pin interrupts are
 * triggered
 *
 * This code will run only when the event occurs and the interrupt flag is
 * raised
 *
 * Once the event occurs and the interrupt is raised we will call a function
 * to create a function to schedule and event in the scheduler
 *
 * @param:
 *      no params
 *
 * @return:
 *      no returns
 *****************************************************************************/
void GPIO_ODD_IRQHandler(void)
{
  int32_t flags;

  flags = GPIO_IntGetEnabled();

  GPIO_IntClear(flags);

  createEventMAX30101Int();

//  createEventPB1Pressed();

//  gpioLed1Toggle(); // For Debugging purpose only
}
