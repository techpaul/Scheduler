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

See other documentation for details
*/
#ifndef SCHEDULE_H
#define SCHEDULE_H

#include "Tasklist.h"
extern struct TaskList taskTable[ ];

extern int Run();
extern int Init( );
#ifndef DISABLE_LOGGING
extern struct TaskList *Log( );
#endif
#ifndef DISABLE_STATS
extern struct Stats *getStats( );
#endif
extern int setInterval( int, int );
extern int getInterval( int );
extern unsigned long getTime( int );
extern int getStatus( int );
extern int Start( int );
extern int FindID( int(* const )( int, int ) );
#endif
