/* Co-operative Scheduler for DUE/SAM primarily

   Using COMPILE time scheduling table

Version V1.00
Author: Paul Carpenter, PC Services, <sales@pcserviceselectronics.co.uk>
Date    February 2016

Co-operative scheduler library that has many methods and main schedule function
See extras documents for introduction into scheduling and pitfalls on Arduino
platform.

Modify this file to add your tasks and customise
See other documentation for details
*/
#ifndef TASKLIST_H
#define TASKLIST_H

// Add includes here for your function declarations to be included in task array
// or direct externs to taks top layer functions of type
//  extern int function( int, int );

/* Array of tasks which are addresses to functions.
   Each function returns int and takes two integer parameters

    e.g.    int func( int TaskId, int Status )

        First parameter is TaskId,
        second is current status of task
            Where   0   task initialise setup default state and interval
                    1   task start
                    > 1 is any status that means running to that task
                    < 0 Invalid never called with negative value

        Return value is new status where
                    < -1 task error status and STOP
                     -1  Reserved for other error status
                     0   Stop task
                     > 1 next status to call task with
*/
int ( * const tasks[])( int, int ) =
                {
                // Insert your task functions names here in order of priority


                };

/* Defines section
   You can change the time at which scheduling is checked, this is the
   time between schedule list checks.

   Default is 10 ms
   Smallest value is 1ms
   Largest value is 32767 ms

   Don't set your minimum time interval for scheduling too small as some
   activities  can take a long time delaying other task execution.
   like
        Print or write to LCD or Serial or other devices
        PulseIn
        Delay

   Setting to values 5 ms and above means

   either   each task can be longer
   or       more short tasks can be run
   or       other tasks can be done in main loop() and hence Arduino
            background tasks

   change the value accordingly */
#define MIN_TASK_INTERVAL 10

/* To remove logging and statistics gathering
     DISABLE_LOGGING deals with saving and accessing snapshots of task history
                     after a pass
     DISABLE_STATS   deals with general statistics
   uncomment out one or both of the following lines */
//#define DISABLE_LOGGING
//#define DISABLE_STATS

/*****************************************************************/
/* Do not edit below here things will break demons will be found */
/*****************************************************************/
// Number of tasks created
#define _MAX_TASKS   (sizeof(tasks) / sizeof( int(* )() ) )

/* Following structures and copy for snapshots for reporting and analysis
  Structures  for task details next run, status etc.. */
struct TaskList {
                unsigned long next;     // next execution time in ms
                unsigned long last;     // last execution time in us
                int status;             // current task status 0 stopped,
                                        // -ve stopped with error,
                                        // 1 start,
                                        // >1 user status (and active)
                int interval;           // interval between starts in ms
                int executed;		    // did run this pass = 1
                };

// Structure for keeping statistics on scheduling
struct Stats    {
                unsigned long start;     // pass start time (ms)
                unsigned long finish;    // pass end time (ms)
                unsigned long maxExec;   // maximum execution time (us)
                int maxID;               // Task with maximum execution time
                unsigned int qty;        // number of tasks run last pass
                unsigned int overdue;    // overdue time (how late scheduler was called)
                unsigned int overdueMax; // largest overdue time
                unsigned int overdueAvg; // Average overdue time
                unsigned int maxLoop;    // Longest schedule loop time
                };
#endif
