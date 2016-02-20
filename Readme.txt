Simple Scheduler Examples and template

Paul Carpenter - P C Services
<mailto:sales@pcserviceselectronics.co.uk>

February 2016

Simple Co-operative scheduler with run to completion tasks.

Originally tested on R3 Due hardware using Arduino 1.6.7 and Due board 1.6.6 

Task list defined at COMPILATION time by simple edits to Tasklist.h rest of 
scheduler setup occurs when initialised and each task is initialised.

Directories

	extra 	additional documentation of -
			  IntorductionToScheduling.pdf
			  Instructions.pdf

	example
        SchedulerTest
            Example of scheduler usage and statistics gathering
            Full sketch needs

			  2 x LEDs (that are from GPIO pin to ground - so 1 = ON)
			  4 x switches (menu switches for new tasks)
              1 x Analog pot on Channel 0
              2 x PWM OUTPUT
			  1 x LCD (20 x 4) in 8 bit mode
			  1 x Serial Port 115,200 baud, (Serial 0 or Programming Port)

	template  Files that need to be copied to your sketch folder

Assumptions modified LCD code is used for improved LCD performanace, if yours 
is slow (more than 2.67 ms to write line of 20 characters) see github pull 
request 4550 for better performing LCD library.

https://github.com/arduino/Arduino/pull/4550
