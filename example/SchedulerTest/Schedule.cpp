/* Co-operative Scheduler for DUE/SAM primarily

   Using COMPILE time scheduling table

Version V1.00
Author: Paul Carpenter, PC Services, <sales@pcserviceselectronics.co.uk>
Date    February 2016

Co-operative scheduler library that has many methods and main schedule function
See extras documents for introduction into scheduling and pitfalls on Arduino
platform.

Don't set your minimum time interval for scheduling too small as the following
could cause other tasks to run late.

1/ Serial prints are blocking (stop other tasks running especially if
   you print a LOT more than the buffer can hold), as the print/write will sit
   there waiting for space in the buffer. Time for each character/byte as below

       Baud     Time wait
       2400     4 ms
       9600     1 ms
       115200   86 us

   In your tasks find out how much space available to write send up to that
   amount this time and on next run of the task send more until all done.

2/ LCD print/write is also time consuming due to speed of LCD, each byte takes
   the following amount of time

                    8 bit mode      4 bit mode
        1 byte      120 - 150 us    240 - 300 us
        10 bytes    1.2 - 1.5 ms    2.4 - 3 ms
        20 bytes    2.4 - 3 ms      4.8 - 6 ms

   Even Command bytes like set cursor position take the same amount of time as
   sending 1 character to the LCD.

3/ Functions PulseIn and Delay along with other activities have same result

Functions
---------
Run         Main scheduling loop each call is one loop of checking if time
            to run tasks and executing them in order on the list.
Init        Initialise all tasks in list
Log         Get pointer to copy of task table
getStats    Get pointer to structure of general statistics) and reset maximums
setInterval Set a task's NEW interval and schedule new time from now if not
            already running
getInterval Get a task's interval
getTime     Get a tasks next time to execute
getStatus   Get a particular schedule status word
Start       Start a task (if not already running)
FindID      Get ID of task from task address

Structure of task code.
-----------------------

    Task is actually calling a function that should be defined as

        int function( int ID, int status )

    Tasks are called and passed in parameters are

        int ID      Task ID to recognise this task (have your task save this
                    for helper functions)
        int status  Current status on entry
                        0 initialise task (only from Init function)
                        1 start task
                        2 - 32767 user state for normal running

      This way state machines and switch statements can be used for a task to
      determine what to do on this execution run.

      If interval of scheduling has to change call setInterval during task
      execution

    Return value
      The returned value is the NEW status for that task which tells scheduler
      what to do next time. Values are

         < -1 User error status (stops task execution)
          -1  used as error code for getStatus of Invalid ID requested
           0  Task stopped (if needed to be run will have to be started by Start)
          > 0 Next status
                1 start
                2 - 32767 User status
*/
#include <Arduino.h>
#include "Tasklist.h"

// Points in Rolling average for overdue status
// Recommended values 8 to 32
#define _MAX_AVERAGE 8

unsigned long old_ms;       // last execution time
int running;                // Current task ID being checked or run

// Array of task details
/* Following structures and copy for snapshots for reporting and analysis
   Structures  for task details next run, status etc..
   array for task numbers and copy array for log table stats
*/
struct TaskList taskTable[ _MAX_TASKS ];
#ifndef DISABLE_LOGGING
struct TaskList tasksCopy[ _MAX_TASKS ];
#endif
#ifndef DISABLE_STATS
// Structure for keeping statistics on scheduling
struct Stats stats;
struct Stats statsCopy;

// overdue rolling average variables
unsigned int overdueTotal = 0;
int overdueIdx = 0;
unsigned int overdueAvg[ _MAX_AVERAGE ];
#endif


/* Run - Task scheduling loop
   Checks if Minimum scheduling interval has passed then does ONE pass through
   scheduling table, checking what tasks are due or overdue to run.

   Order of task execution is list of tasks

   When task has completed -
        updates task status with status returned,
        adjusts next run time using interval specified.
            If interval is zero execution time set to zero.
        task run time saved in ms. (may often be zero)

   End of pass logs
      number of tasks run
      pass time
      pass end time
      max pass end time
      overdue time (how late scheduler was called)
      max overdue time
      rolling average overdue (16 point rolling average)

   Then copy tasks table and statistics to copies for User application analysis

   Parameters - NONE

   Returns  int < 0 too early to process
                 0  Processed no tasks to run
                > 0 Number of tasks executed
*/
int Run()
{
int done;
unsigned int overdue;
unsigned long ms;
unsigned long last_us;

// get current time exit if too early
ms = millis( );
overdue = (unsigned int)( ms - old_ms );
if( overdue < MIN_TASK_INTERVAL )
  return -1;

old_ms = ms;

// Do schedule list ONE pass
done = 0;
for( running = 0; running < (int)_MAX_TASKS; running++ )
   {
   if( taskTable[ running ].status > 0 )      // task enabled
     { // check if time to run as in correct interval or overdue
     if( ms - taskTable[ running ].next <= overdue )
       { // run task get new status
       last_us = micros( );
       taskTable[ running ].status = ( *tasks[ running ])( running, taskTable[ running ].status );
       last_us = micros( ) - last_us;
       if( taskTable[ running ].status > 0 )    // process based on new status
         taskTable[ running ].next = ms + taskTable[ running ].interval;
       // save execution time
       taskTable[ running ].last = last_us;
       taskTable[ running ].executed = 1;       // Ran
#ifndef DISABLE_STATS
       if( last_us > stats.maxExec )        // check if above max execution
         {
         stats.maxExec = last_us;           // save max execution time
         stats.maxID = running;             // and task ID
         }
#endif
       done++;
       }
     else
       taskTable[ running ].executed = 0;   // not run
     }
   }
#ifndef DISABLE_STATS
/* End of pass create statistics */
stats.finish = millis( );           // pass end time
stats.start = ms;                   // pass start time
ms = stats.finish -  stats.start;   // get loop time
if( ms > stats.maxLoop )
  stats.maxLoop = ms;               // save longest loop time
stats.qty = done;                   // number of tasks run this pass
if( overdue <= MIN_TASK_INTERVAL )  // Only add to stats if really overdue
  overdue = 0;
else
  overdue -= MIN_TASK_INTERVAL;
stats.overdue = overdue;            // how late scheduler was called
if( overdue > stats.overdueMax )
  stats.overdueMax = overdue;       // Max Overdue call to scheduling
overdueTotal -= overdueAvg[ overdueIdx ];
overdueTotal += overdue;
stats.overdueAvg = overdueTotal / _MAX_AVERAGE;
overdueAvg[ overdueIdx ] = overdue;
if( ++overdueIdx >= _MAX_AVERAGE )
  overdueIdx = 0;
#endif
// Snapshot copy tables and stats for any requests
#ifndef DISABLE_LOGGING
memcpy( tasksCopy, taskTable, sizeof( taskTable ) );
#endif
#ifndef DISABLE_STATS
memcpy( &statsCopy, &stats, sizeof( struct Stats ) );
#endif
return done;
}


/* Init - Initialise all Tasks in scheduling table
   Calls each task with a status of 0 to initialise, each task must initialise
      own status and variables
      set interval time if required
      set next status to 1 or higher to start scheduling or zero to stop for now

   Order of task execution is list of tasks (can have multiple entries only for
   the brave)

   If multiple entries in list the task must sort out first call to initialise
   internal variables and following init calls to just set interval and status
   for THIS task.

   Parameters - NONE

   Returns  int < 0 Error no tasks in list
                > 0 Number of tasks executed
*/
int Init( )
{
unsigned long ms;
unsigned long last_us;

// get current time
ms = millis( );
old_ms = ms;        // Save last executed as now

for( running = 0; running < (int)_MAX_TASKS; running++ )
   {
   last_us = micros( );
   taskTable[ running ].status = (*tasks[ running ])( running, 0 );
   last_us = micros( ) - last_us;
   taskTable[ running ].last = last_us;   // save execution time
   taskTable[ running ].executed = 1;     // Ran
   if( taskTable[ running ].status > 0 )
     taskTable[ running ].next = ms + taskTable[ running ].interval;
   }
return running;
}


/* checkID - Common ID check for valid and not running
    Parameters  int Task ID to check

    Return int  -1  invalid ID
                 0  current task running
                 1  Valid
*/
int checkID( int ID )
{
if( ID < 0 || ID >= (int)_MAX_TASKS )
  return -1;
if( ID == running )
  return 0;
return 1;
}


#ifndef DISABLE_LOGGING
/* Log - Take snapshot of all tasks - task scheduling details
   Copies current tasksTable to tasksCopy and returns pointer to tasksCopy

   Parameters  None

   Return      Pointer to copy array of task structures of type .........
               See Schedule.h for details of structure for accessing

               Array is _MAX_TASKS long so remember to define it
*/
struct TaskList *Log( )
{
return tasksCopy;
}
#endif


#ifndef DISABLE_STATS
/* getStats - Take snapshot of task scheduling stastics
   Copies current stats to statsCopy and returns pointer to statsCopy
   After copying max and some other entries are reset to zero

   Parameters  None

   Return      Pointer to copy array of scheduling statistics of type .........
               See Schedule.h for details of structure for accessing
*/
struct Stats *getStats( )
{
stats.overdueMax = 0;
stats.maxExec = 0;
stats.maxID = 0;
stats.maxLoop = 0;
return &statsCopy;
}
#endif


/* setInterval - set the interval time in ms for a task
   If task running just sets interval as next execution will be set at task end.

   When task NOT running, also sets next execution time to now plus interval.

    Parameters  int Task ID to check
                int interval to set

    Return int  -2 invalid interval
                -1 invalid ID
                 0  task is running
                > 0 task interval and execution time set
*/
int setInterval( int ID, int interval )
{
int i;

if( ( i = checkID( ID ) ) < 0 )
  return i;
if( interval < MIN_TASK_INTERVAL )
  return -2;
taskTable[ ID ].interval = interval;
if( i != 0 )
  {
  taskTable[ ID ].next = millis( ) + interval;
  return 1;
  }
return 0;
}


/* getInterval - get the interval time in ms for a task
    Parameters  int Task ID to check

    Return int  < 0 invalid ID
                >= 0 Valid interval time
*/
int getInterval( int ID )
{
int i;

if( ( i = checkID( ID ) ) < 0 )
  return i;
return taskTable[ ID ].interval;
}


/* getTime - get next execution time in ms of a task
    As value is unsigned long impossible to guarantee error codes
    So if values listed below for errors check current millis() value
    to see if could be real or error.

    Parameters  int Task ID to check

    Return      unsigned long of time (or could be errors)
                0 could be execution time or errro of invalid ID
                1 could be execution time or error of NO interval check
*/
unsigned long getTime( int ID )
{
int i;

if( ( i = checkID( ID ) ) < 0 )
  return 0;
if( taskTable[ ID ].interval <= 0 )
  return 1;
return taskTable[ ID ].next;
}


/* getStatus - get task status even if running task
    Parameters  int Task ID to get status for

    Return int  -1  invalid ID
               any other value Status (including other user errors -ve
*/
int getStatus( int ID )
{
int i;

// Check valid ID, not running and interval
if( ( i = checkID( ID ) ) < 0 )
  return i;
return taskTable[ ID ].status;
}


/* Start - Start a task if not running and has interval set
   Cannot start an already started task

   It is responsibility of the task to stop its task and any associated
   resources (GPIO/TWI/SPI etc).

    Parameters  int Task ID to start

    Return int  -3  Task already started
                -2  No interval on task
                -1  invalid ID
                 0  current task running
                > 0 Valid
*/
int Start( int ID )
{
int i;

// Check valid ID, not running and interval
if( ( i = checkID( ID ) ) <= 0 )
  return i;
if( taskTable[ ID ].interval <= 0 )
  return -2;
if( taskTable[ ID ].status  > 0 )
  return -3;
// Start task
taskTable[ ID ].status = 1;
taskTable[ ID ].next = old_ms + taskTable[ ID ].interval;
return 1;
}


/* FindID - Get ID of task from task address
   If a task is running it is not possible to stop a task executing
   that is for the task or communications to the task from other
   sources to change the task return status to 0

    Parameters  function address

    Return int  < 0 Invalid task address
                >= 0 Valid Task ID
*/
int FindID( int(* const ptr)( int, int ) )
{
int i;

if( ptr == NULL )
  return -1;
for( i = 0; i < (int)_MAX_TASKS; i++ )
   if( tasks[ i ] == ptr )
     break;
if( i == (int)_MAX_TASKS )
  return -2;
return i;
}
