/*
  gpio.c
 
   Created on: Dec 12, 2018
       Author: Dan Walkes
   Updated by Dave Sluiter Dec 31, 2020. Minor edits with #defines.

   March 17
   Dave Sluiter: Use this file to define functions that set up or control GPIOs.

 */




#include "gpio.h"



/**************************************************************************//**
 * This function initialises the GPIO Pins. Sets modes to the appropriate pins
 * and port numbers
 *
 * Refer to the data sheet for the details listed below
 *****************************************************************************/
void gpioInit()
{

//  // Student Edit:
//
//	// GPIO_DriveStrengthSet(LED0_port, gpioDriveStrengthStrongAlternateStrong);
//	//GPIO_DriveStrengthSet(LED0_port, gpioDriveStrengthWeakAlternateWeak);
	GPIO_PinModeSet(LED0_port, LED0_pin, gpioModePushPull, false); // Assignment 2: Initializing the LED0 to Push Pull Mode
//
//	// GPIO_DriveStrengthSet(LED1_port, gpioDriveStrengthStrongAlternateStrong);
//	//GPIO_DriveStrengthSet(LED1_port, gpioDriveStrengthWeakAlternateWeak);
	 GPIO_PinModeSet(LED1_port, LED1_pin, gpioModePushPullAlternate, false);


// Setting the GPIO modes for I2C with 7021  temperature sensor
//  GPIO_PinModeSet(SCL_PORT, SCL_PIN, gpioModePushPullAlternate, false); // Taken care by I2CPSM
//  GPIO_PinModeSet(SDA_PORT, SDA_PIN, gpioModePushPullAlternate, false); // Taken care by I2CPSM
  GPIO_PinModeSet(SENSOR_ENABLE_PORT, SENSOR_ENABLE_PIN, gpioModePushPull, false);

  GPIO_DriveStrengthSet(LED_RED_port, gpioDriveStrengthStrongAlternateStrong);
  GPIO_PinModeSet(LED_RED_port, LED_RED_pin, gpioModePushPull, false);

  GPIO_DriveStrengthSet(LED_GREEN_port, gpioDriveStrengthStrongAlternateStrong);
  GPIO_PinModeSet(LED_GREEN_port, LED_GREEN_pin, gpioModePushPull, false);

  GPIO_DriveStrengthSet(LED_BLUE_port, gpioDriveStrengthStrongAlternateStrong);
  GPIO_PinModeSet(LED_BLUE_port, LED_BLUE_pin, gpioModePushPull, false);

} // gpioInit()

/**************************************************************************//**
 * GPIO pin set to turn ON LED0
 *****************************************************************************/
void gpioLed0SetOn()
{
	GPIO_PinOutSet(LED0_port,LED0_pin);
}

/**************************************************************************//**
 * GPIO pin set to turn OFF LED0
 *****************************************************************************/
void gpioLed0SetOff()
{
	GPIO_PinOutClear(LED0_port,LED0_pin);
}

/**************************************************************************//**
 * GPIO pin set to turn ON LED1
 *****************************************************************************/
void gpioLed1SetOn()
{
	GPIO_PinOutSet(LED1_port,LED1_pin);
}


/**************************************************************************//**
 * GPIO pin set to turn OFF LED1
 *****************************************************************************/
void gpioLed1SetOff()
{
	GPIO_PinOutClear(LED1_port,LED1_pin);
}

/**************************************************************************//**
 * GPIO pin set to toggle LED1
 *****************************************************************************/
void gpioLed1Toggle()
{
  GPIO_PinOutToggle(LED1_port,LED1_pin);
}

/**************************************************************************//**
 * GPIO pin set to turn ON the sensor 7021 for temperature measurement
 *****************************************************************************/
void sensorEnable()
{
  GPIO_PinOutSet(SENSOR_ENABLE_PORT,SENSOR_ENABLE_PIN);
}

/**************************************************************************//**
 * GPIO pin cleared to turn OFF the sensor 7021 for temperature measurement
 *****************************************************************************/
void sensorDisable()
{
  GPIO_PinOutClear(SENSOR_ENABLE_PORT,SENSOR_ENABLE_PIN);
}

/**************************************************************************//**
 * Test Sequence to check all RGB colours
 *****************************************************************************/
void LED_test_seq()
{
  GPIO_PinOutSet(LED_RED_port,LED_RED_pin);
  timerWaitUs_blocking(1000000);
  GPIO_PinOutClear(LED_RED_port,LED_RED_pin);

  GPIO_PinOutSet(LED_GREEN_port,LED_GREEN_pin);
  timerWaitUs_blocking(1000000);
  GPIO_PinOutClear(LED_GREEN_port,LED_GREEN_pin);

  GPIO_PinOutSet(LED_BLUE_port,LED_BLUE_pin);
  timerWaitUs_blocking(1000000);
  GPIO_PinOutClear(LED_BLUE_port,LED_BLUE_pin);
}
