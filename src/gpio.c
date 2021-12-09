/*
 * gpio.h
 *
 *  Modified on: 8 Dec 2021
 *      Author:
 *          Author 1: Nihal T
 *          Author 2: Sudarshan J
 *
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
  GPIO_PinModeSet(DISP_EXTCOMIN_PORT, DISP_EXTCOMIN_PIN, gpioModePushPullAlternate, false);

  GPIO_DriveStrengthSet(LED_RED_port, gpioDriveStrengthStrongAlternateStrong);
  GPIO_PinModeSet(LED_RED_port, LED_RED_pin, gpioModePushPull, false); // Assignment 2: Initializing the LED0 to Push Pull Mode


  GPIO_DriveStrengthSet(LED_GREEN_port, gpioDriveStrengthStrongAlternateStrong);
  GPIO_PinModeSet(LED_GREEN_port, LED_GREEN_pin, gpioModePushPull, false); // Assignment 2: Initializing the LED0 to Push Pull Mode

  GPIO_DriveStrengthSet(LED_BLUE_port, gpioDriveStrengthStrongAlternateStrong);
  GPIO_PinModeSet(LED_BLUE_port, LED_BLUE_pin, gpioModePushPull, false); // Assignment 2: Initializing the LED0 to Push Pull Mode

//  CMU_ClockEnable(cmuClock_GPIO, true);

  GPIO_PinModeSet(PB0_PORT, PB0_PIN, gpioModeInputPullFilter, true);
  GPIO_PinModeSet(PB1_PORT, PB1_PIN, gpioModeInputPullFilter, true);

  GPIO_PinModeSet(MAX_30101_HR_port, MAX_30101_HR_pin, gpioModeInputPullFilter, true);

  gpioPB0IntEnable();
  gpioPB1IntEnable();
  gpioMAX30101IntEnable();


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
 * GPIO pin set to toggle LED0
 *****************************************************************************/
void gpioLed0Toggle()
{
  GPIO_PinOutToggle(LED0_port,LED0_pin);
}

/**************************************************************************//**
 * Sets the color of RGB LEDs
 *****************************************************************************/
void RGB_LED(bool red, bool green, bool blue)
{
  if (red)
  {
      GPIO_PinOutSet (LED_RED_port, LED_RED_pin);
  }
  else
  {
      GPIO_PinOutClear (LED_RED_port, LED_RED_pin);
  }

  if (green)
  {
      GPIO_PinOutSet (LED_GREEN_port, LED_GREEN_pin);
  }
  else
  {
      GPIO_PinOutClear (LED_GREEN_port, LED_GREEN_pin);
  }

  if (blue)
  {
      GPIO_PinOutSet (LED_BLUE_port, LED_BLUE_pin);
  }
  else
  {
      GPIO_PinOutClear (LED_BLUE_port, LED_BLUE_pin);
  }
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
 * GPIO pin set to toggle the pin to provide a frequency
 *****************************************************************************/
void gpioSetDisplayExtcomin()
{
  GPIO_PinOutToggle(DISP_EXTCOMIN_PORT,DISP_EXTCOMIN_PIN);
}

/**************************************************************************//**
 * Function to enable the Push Button 0 interrupt
 *****************************************************************************/
void gpioPB0IntEnable()
{
  GPIO_ExtIntConfig (PB0_PORT, PB0_PIN, PB0_PIN, true, true, true);
}

/**************************************************************************//**
 * Function to enable the Push Button 1 interrupt
 *****************************************************************************/
void gpioPB1IntEnable()
{
  GPIO_ExtIntConfig (PB1_PORT, PB1_PIN, PB1_PIN, true, true, true);
}

/**************************************************************************//**
 * Function to enable the Push Button 0 interrupt
 *****************************************************************************/
void gpioPB0IntDisable()
{
  GPIO_ExtIntConfig (PB0_PORT, PB0_PIN, PB0_PIN, true, true, false);
}

/**************************************************************************//**
 * Function to enable the Push Button 1 interrupt
 *****************************************************************************/
void gpioPB1IntDisable()
{
  GPIO_ExtIntConfig (PB1_PORT, PB1_PIN, PB1_PIN, true, true, false);
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

/**************************************************************************//**
 * Function to enable the Push Button 1 interrupt
 *****************************************************************************/
void gpioMAX30101IntEnable()
{
  GPIO_ExtIntConfig (MAX_30101_HR_port, MAX_30101_HR_pin, MAX_30101_HR_pin, false, true, true);
}

/**************************************************************************//**
 * Function to enable the Push Button 0 interrupt
 *****************************************************************************/
void gpioMAX30101IntDisable()
{
  GPIO_ExtIntConfig (MAX_30101_HR_port, MAX_30101_HR_pin, MAX_30101_HR_pin, false, true, false);
}

