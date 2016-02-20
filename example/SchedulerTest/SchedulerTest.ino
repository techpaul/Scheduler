/* Scheduler - multi module example

  Created Feb 2016
  by Paul Carpenter

  Originally run on Due R3 based DMX controller

  Needs TWO LEDs (pseudo O/P on LED_PWM brightness inverse of LCD_PWM)
        FOUR switches
        LCD 20 x 4, with PWM driven brightness
        Analog pot on channel 0 (between 3V3 and 0V)
        Serial port (115,200 baud)

  This is an example piece of code that shows scheduler modules being used to
        Read a pot to adjust PWMs for brightness (LED and LCD brightness)
        Flash one LED at 4Hz
        SW_LCD_RIGHT_MID Switch Button press interrupt flash second LED
                at 10Hz for 2 seconds
        Checksum memory space every 10ms
        SW_LCD_RIGHT switch output checksum to LCD
        SW_LCD_LEFT switch button press send log to serial
        SW_LCD_LEFT_MID switch button press send statistics to serial

  After initialisation and displaying menus on LCD, the main schedule loop is run to
  call tasks at predetermined intervals

  The PWM outputs do not need to be connected but are taken from driving two
  PWM outputs from a design example I did, where
     LCD_PWM from 0 to 4095, with high period being when LCD backlight ON
     LED_PWM from 0 to 3968, so 0 was fully on and 3968 was minimum brightness
              As LEDs should never go fully off. Note LOW period is ON time
              3968 is ( ANALOG_MAX - MIN_LED ) where ANALOG_MAX is 4095
              and MIN_LED is 127 for 12 bit PWM (see IO.h)
*/
#include <Arduino.h>
#include <LiquidCrystal.h>
#include "IO.h"
#include "Schedule.h"

// Pointers for statistics printing
struct TaskList *logptr;

int old_pot;        // last pot reading for determining change threshold
int ID10Hz;         // Taks IDs for various tasks for helper functions
int IDLCD;
int IDSwitch;
unsigned long checksum;         // RAM checksum for continuous checksums
unsigned long checksize = 1024; // Number of ints to check

// Interrupt from switches flags to assist switch de-bounce
volatile uint8_t Enable10Hz;
volatile uint8_t EnableStats;
volatile uint8_t EnableLog;
volatile uint8_t EnableCS;

/** Class Constructors **/
// LCD library with the numbers of the interface pins
// For 8 bit interface
LiquidCrystal lcd( RS, RW, ENA, D0, D1, D2, D3, D4, D5, D6, D7 );


/*********** On reset initialise board ************
  Startup
    Initialise GPIOs etc
    Initialise LCD and display menu
    Initialise Task scheduler
    Output to serial the initialisation log
*/
void setup( )
{
initGPIO( );            // Initialise GPIO pins
Serial.begin( 115200 );
lcd.begin( MAX_COL, MAX_ROW );
lcd.clear( );                 // clear the screen
lcd.cursor( );                // Enable Cursor
lcd.blink( );                 // Blinking cursor
// Display menu and other details
lcd.print( "PC Services" );
lcd.setCursor( 0, 1 );
lcd.print( "Scheduler - " );
lcd.print( _MAX_TASKS, DEC );
lcd.print( " tasks" );
lcd.setCursor( 0, 2 );
lcd.print( checksize * sizeof( int ), DEC );
lcd.print( " bytes=" );
lcd.setCursor( 0, 3 );
lcd.print( "Log Stats 10Hz Check" );
Init( );                    // Initialise all tasks
// Send initialisation log to serial
logptr = taskTable;
dumplog( );
logptr = Log( );
}


/**********
  Loop
    Run schedule pass

    Main programme loop Task scheduler Run( ) deals with what runs when as
 that is the task scheduler
*****/
void loop( )
{
Run( );
// Insert any special statistics gathering to memory you need here
// keep it SHORT
}


/******** The tasks to process ******************/

/* Task  - Read a pot and adjust brightness if necessary 10 times a second
   So delay = 100 ms

   Brightness is two inverse PWMs determined by pot LCD is active HIGH for ON
   LED is active LOW for ON, and has minimum brightness threshold
*/
int brightnessCheck( int ID, int status )
{
int val, i;

switch( status )
  {
  case 0: // initialise
          setInterval( ID, 100 );
          status = 1;
          break;
  case 1: // Start does a forced read and set to current pot level
  case 2: // Read Pot and send to PWM O/P
          val = analogRead( BRIGHT_POT );
          if( val > old_pot )               // get difference in reading
            i = val - old_pot;
          else
            i = old_pot - val;
          if( i > 10 || status == 1 )       // more than threshold or start
            {
            analogWrite( LCD_PWM, val );    // LCD backlight dimming
            old_pot = val;
            val = ANALOG_MAX - val;         // Reverse for inverse LED pwm
            if( val > ( ANALOG_MAX - MIN_LED ) )// Min LED brightness
              analogWrite( LED_PWM, val );  // LED Dimming
            else
              analogWrite( LED_PWM, ( ANALOG_MAX - MIN_LED ) );  // LEDs should never be fullky off
            }
          status = 2;
  }
return status;
}


/* Task - Flash LED 1 at 4 Hz  (continuous)
   4 Hz means 8 calls per second so interval = 125 ms */
int LED4hz( int ID, int status )
{
switch( status )
  {
  case 0: // initialise
          setInterval( ID, 125 );
          status = 2;
          break;
  case 1: // Start
  case 2: // write LED ON
          digitalWrite( LED1, 0 );
          status = 3;
          break;
  case 3: // write LED OFF
          digitalWrite( LED1, 1 );
          status = 2;
  }
return status;
}


/* Task  - Flash LED 2 at 10Hz for 2 Seconds on switch press. Initially off
   Triggered by switch SW_LCD_RIGHT_MID
   10 Hz means 20 calls per second so interval = 50 ms
   At end of running re-enable switch flag for next push */
int LED10Hz( int ID, int status )
{
static int count;

switch( status )
  {
  case 0: // initialise
          ID10Hz = ID;
          // Attach Interrupt to switch
          attachInterrupt( SW_LCD_RIGHT_MID, menuRightMid, FALLING );
          setInterval( ID, 50 );
          Enable10Hz = 0;
          status = 0;
          break;
  case 1: // Start
          count = 0;
          status = 2;
          break;
  case 2: // Toggle LED
          count++;
          digitalWrite( LED2, count & 1 );
          if( count < 40 )
            status = 2;
          else
            {
            status = 0;
            Enable10Hz = 0;
            }
  }
return status;
}


/* Task  -  Checksum Area of RAM every 10ms and save result */
int CheckRAM( int ID, int status )
{
unsigned long i;
unsigned int *ptr;

switch( status )
  {
  case 0: // initialise
          setInterval( ID, MIN_TASK_INTERVAL );
          status = 2;
          break;
  case 1: // Start
  case 2: // Perform Checksum
          checksum = 0;
          ptr = (unsigned int *)0x20070000;
          for( i = 0; i < checksize; i++ )
             checksum += *ptr++;
          status = 3;
  }
return status;
}


/* Task - Output last Checksum to LCD
   Triggered by switch SW_LCD_RIGHT
   At end of running re-enable switch flag for next push */
int CheckLCD( int ID, int status )
{
switch( status )
  {
  case 0: // initialise
          IDLCD = ID;
          // Attach Interrupt to switch
          attachInterrupt( SW_LCD_RIGHT, menuRight, FALLING );
          setInterval( ID, MIN_TASK_INTERVAL );
          EnableCS = 0;
          status = 0;
          break;
  case 1: // Start
          lcd.setCursor( 12, 2 );
          lcd.print( "        " );
          status = 2;
          break;
  case 2: // Write last Checksum to LCD then stop
          lcd.setCursor( 12, 2 );
          lcd.print( checksum, HEX );
          EnableCS = 0;
          status = 0;
  }
return status;
}


/* Task - check statistics
   Triggered by switch SW_LCD_LEFT_MID for statistics
   Triggered by switch SW_LCD_LEFT for list
   See if we output statistics
   At end of running re-enable switch flag for next push */
int statisticsCheck( int ID, int status )
{
switch( status )
  {
  case 0: // initialise
          IDSwitch = ID;
          // Attach Interrupts to switches
          attachInterrupt( SW_LCD_LEFT, menuLeft, FALLING );
          attachInterrupt( SW_LCD_LEFT_MID, menuLeftMid, FALLING );
          EnableLog = 0;
          EnableStats = 0;
          setInterval( ID, 100 );
          status = 0;
          break;
  case 1: // Start and recheck if anything to do
          if( EnableLog )
             status = 2;
          else
            if( EnableStats )
              status = 3;
            else
              status = 0;
          break;
  case 2: // output log
          dumplog( );
          EnableLog = 0;
          status = 1;
          break;
  case 3: // output stats
          dumpstats( );
          EnableStats = 0;
          status = 1;
  }
return status;
}


// Serial output of last task scheduler task list copy in readable format
// Called from task Check statistics
void dumplog( )
{
int i;

Serial.print( "\nCurrent time - " );
Serial.println( millis(), DEC );
Serial.println( "ID\tNext\tTook\tStatus\tInter\tRan" );
for( i = 0; i < (int)_MAX_TASKS; i++ )
   {
   Serial.print( i, DEC );
   Serial.write( '\t' );
   Serial.print( logptr[ i ].next, DEC );
   Serial.write( '\t' );
   Serial.print( logptr[ i ].last, DEC );
   Serial.write( '\t' );
   Serial.print( logptr[ i ].status, DEC );
   Serial.write( '\t' );
   Serial.print( logptr[ i ].interval, DEC );
   Serial.write( '\t' );
   Serial.println( logptr[ i ].executed, DEC );
   }
}


// Serial output of last task scheduler Statistics copy in readable format
// Called from task Check statistics
void dumpstats( )
{
struct Stats *statsptr;

statsptr = getStats( );
Serial.print( "\nStatistics\n Finish\t" );
Serial.print( statsptr->finish, DEC );
Serial.print( "\tStart\t" );
Serial.print( statsptr->start, DEC );
Serial.print( "\tDiff = " );
Serial.print( statsptr->finish - statsptr->start, DEC );
Serial.print( "\nTask Run -\t" );
Serial.print( statsptr->qty, DEC );
Serial.print( "\nOverdue by - " );
Serial.print( statsptr->overdue, DEC );
Serial.print( "\tMax -\t" );
Serial.print( statsptr->overdueMax, DEC );
Serial.print( "\tAvg -\t" );
Serial.print( statsptr->overdueAvg, DEC );
Serial.print( "\nMax exec - " );
Serial.print( statsptr->maxExec, DEC );
Serial.print( "\tby - " );
Serial.println( statsptr->maxID, DEC );
Serial.print( "Longest Loop Time " );
Serial.println( statsptr->maxLoop, DEC );
}


/* Interrupt Routines for switches to set flags and start tasks
   Each task must save its appropriate task ID first */
void menuLeft( void )
{
if( !EnableLog )
  {
  EnableLog = 1;
  Start( IDSwitch );
  }
}

void menuLeftMid( void )
{
if( !EnableStats )
  {
  EnableStats = 1;
  Start( IDSwitch );
  }
}

void menuRightMid( void )
{
if( !Enable10Hz )
  {
  Enable10Hz = 1;
  Start( ID10Hz );
  }
}

void menuRight( void )
{
if( !EnableCS )
  {
  EnableCS = 1;
  Start( IDLCD );
  }
}
