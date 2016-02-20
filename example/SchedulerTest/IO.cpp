/* Scheduler example IO assignments

  Constants for pin numbers and similar like LCD Rows and Column
  Pins that are input or output

  Only operate on pins and interfaces not dealt with elsewhere like
    LCD (pins defined in IO.h but initialised elsewhere)
    LEDs on SPI and pwm pin
    Serial ports

  Header IO.h defines ALL pins

  Created Feb 2016
  Paul Carpenter
*/
// Get project Pin constants
#include <Arduino.h>
#include "IO.h"


// Array of pins to set to Input Pullup
const uint8_t inputPins[]  = { SW_LCD_LEFT, SW_LCD_LEFT_MID,
                                SW_LCD_RIGHT_MID, SW_LCD_RIGHT };
const uint8_t outputPins[]  = { LED1, LED2 };


void initGPIO( )
{
int pins;

// Set GPIOs to default mode and levels
for( pins = 0; pins < (signed char)sizeof( inputPins ); pins++ )
   pinMode( inputPins[ pins ], INPUT_PULLUP );
// outputs to OFF
for( pins = 0; pins < (signed char)sizeof( outputPins ); pins++ )
   {
   pinMode( outputPins[ pins ], OUTPUT );
   digitalWrite( outputPins[ pins ], 0 );
   }

//pinMode( LCD_PWM, OUTPUT );
analogWriteResolution( ANALOG_RES );
analogReadResolution( ANALOG_RES );
analogWrite( LCD_PWM, 0 );
}
