/*
 * gpio.h
 *
 *  Modified on: 8 Dec 2021
 *      Author:
 *          Author 1: Nihal T
 *          Author 2: Sudarshan J
 *
 */

#ifndef SRC_GPIO_H_
#define SRC_GPIO_H_
#include <stdbool.h>
#include "em_gpio.h"
#include <string.h>
#include "i2c.h"



// Student Edit: Define these, 0's are placeholder values.
// See the radio board user guide at https://www.silabs.com/documents/login/user-guides/ug279-brd4104a-user-guide.pdf
// and GPIO documentation at https://siliconlabs.github.io/Gecko_SDK_Doc/efm32g/html/group__GPIO.html
// to determine the correct values for these.

#define LED0_port  gpioPortF // The LED is connected to Port F
#define LED0_pin   4 // The LED is connected to Pin 4
#define LED1_port  gpioPortF // The LED is connected to Port F
#define LED1_pin   5 // The LED is connected to Pin 4



// Defining the communication pins for clock and data lines
#define SCL_PORT gpioPortC // Clock line port
#define SCL_PIN 10 // Clock line pin

#define SDA_PORT gpioPortC // Data line port
#define SDA_PIN 11 // Data line pin

// Sensor enable pin for 7021 and LCD
#define SENSOR_ENABLE_PORT gpioPortD // Sensor enable port
#define SENSOR_ENABLE_PIN 15 // Sensor enable pin

// LCD pin
#define DISP_EXTCOMIN_PORT gpioPortD //  enable port
#define DISP_EXTCOMIN_PIN 13 // Sensor enable pin

// Defining the Port and Pin for push Button 0
#define PB0_PORT gpioPortF
#define PB0_PIN 6

// Defining the Port and Pin for push Button 1
#define PB1_PORT gpioPortF
#define PB1_PIN 7

// RED
#define LED_RED_port  gpioPortF // The LED is connected to Port F
#define LED_RED_pin   3 // The LED is connected to Pin 4

// GREEN
#define LED_GREEN_port  gpioPortD // The LED is connected to Port F
#define LED_GREEN_pin   12 // The LED is connected to Pin 4

// BLUE
#define LED_BLUE_port  gpioPortD // The LED is connected to Port F
#define LED_BLUE_pin   10 // The LED is connected to Pin 4

// GREEN
#define MAX_30101_HR_port  gpioPortA // The LED is connected to Port A
#define MAX_30101_HR_pin   3 // The LED is connected to Pin 3

// Function prototypes
void gpioInit();
void gpioLed0SetOn();
void gpioLed0SetOff();
void gpioLed1SetOn();
void gpioLed1SetOff();

void gpioLed1Toggle(); // Done for debugging purpose
void gpioLed0Toggle();

void sensorEnable(); // To turn ON the 7021 temperature sensor
void sensorDisable(); // To turn OFF the 7021 temperature sensor

void gpioSetDisplayExtcomin();

void gpioPB0IntEnable();
void gpioPB1IntEnable();
void gpioPB0IntDisable();
void gpioPB1IntDisable();

void RGB_LED(bool red, bool green, bool blue);

void LED_test_seq();

void gpioMAX30101IntEnable();
void gpioMAX30101IntDisable();


#endif /* SRC_GPIO_H_ */
