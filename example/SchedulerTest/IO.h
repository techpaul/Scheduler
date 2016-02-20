/* Scheduler example IO assignments

  Constants for pin numbers and similar like LCD Rows and Column
  Pins that are input or output

  Only operate on pins and interfaces not dealt with elsewhere like
    LCD (pins defined here but initialised elsewhere)
    Serial ports

  This header IO.h defines ALL pins

  Created Jan 2016
  Paul Carpenter
*/
#ifndef IO_h
#define IO_h

// Constants for GPIO Pins
// Inputs with Pullup
#define SW_LCD_LEFT     68
#define SW_LCD_LEFT_MID 69
#define SW_LCD_RIGHT_MID 66
#define SW_LCD_RIGHT    67

// Outputs
#define LED_PWM     12
#define LCD_PWM     11
#define LED1        10
#define LED2        9

// Analog inputs
#define BRIGHT_POT  0

// Analog resolution in bits and max
#define ANALOG_RES  12
#define ANALOG_MAX  4095
#define MIN_LED     127

/* LCD pins definitions (constants) to match your LCD */
#define RS  38
#define ENA 40
#define RW  41
#define D7  47
#define D6  46
#define D5  49
#define D4  48
#define D3  44
#define D2  45
#define D1  42
#define D0  43

/* LCD Constants to match your display
   Columns in display */
#define MAX_COL 20
/* Rows in display */
#define MAX_ROW 4

// functions
void initGPIO( );
#endif
