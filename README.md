# Arduino Simple Co-operative Scheduler

## Scheduler originally tested on DUE R3 hardware and V1.6.7 Arduino IDE

Simple Co-operative scheduling module to add to sketch, for *COMPILE* time
task list creation. This is because all tasks are known at compile time and new
tasks cannot be loaded whilst running, tasks can be stopped and started so
management of tasks is simpler.

All tasks can run as state machines using task status from task scheduler.

Has ability to read list of tasks and last execution status along with
statistics to aid debugging. Statistics and list include last execution time
in us as well as details on overdue scheduling or longest ever task run time.

Many methods to start and change frequency and other aspects of task management.
Tasks can stop themselves and change frequency, other helper functions, tasks
and interrupts.

Extra folder contains documentation also comments in code to assist.

Only file that needs editing is Tasklist.h to give the list of top level 
functions for each task and if you want faster or slower task loop time (the
time between each processing pass of the task list).

## Example

Example folder contains an example performing multiple tasks and some on demand
so they stay stopped until started on demand.

Uses hardware of

    2 x LEDs (wired from GPIO to ground via resistor for 1 = ON
    20 x 4 LCD (PWM available to drive backlight brightness
    Analogue pot to determine brightness for backlight (High for ON)
    LED PWM output for driving optional LED Brightness but LOW for ON
    FOUR switches to make requests to start tasks
    Serial port (115,200 baud) Serial0 or Programming Port
    
See file IO.h for pin assignments    
    
Main tasks
1. Continuous read pot to set brightness    
2. Continuous checksum and area of RAM
3. Continuous flash One LED at 4Hz
4. On demand (from a switch) flash a second LED at 10Hz for 2 seconds
5. On demand (from a switch) display last checksum on LCD
6. On demand (from a switch) send copy of last pass task list details to serial
7. On demand (from a switch) send copy of last pass statistics details to serial

## Installation

Three files to add to sketch ONLY one to edit to match your sketch.

    Schedule.cpp
    Schedule.h
    Tasklist.h
    
    
### Author

Paul Carpenter<br>
P C Services<br>
<sales@pcserviceselectronics.co.uk><br>
<http://www.pcserviceselectronics.co.uk><br>

February 2016
