******************* Airport Shuttle Simulation **********************
************ README File for COP4610 project 2 part3 ****************

Authors: Cory Agami and Brandon Hillan
Date: 10/29/2014

*******************************Summary*******************************

*NOTE: part1and2 folder contains module / syscall practice for parts 1 and 2. 
       Main project code for module, simulation, and system calls is in src.
       python_shuttle contains an early python implementation of the shuttle.

The following C code implements the following system calls in a kernel module

int start_shuttle(void) 

A shuttle begins at Terminal C with no passenger onboard and no passenger 
waiting at all terminals, and with a shuttle status of either PARKED or MOVING.  
The call returns 1 if the service has already been started and 0 if it was 
started successfully.

int issue_request(char passenger_type, int initial_terminal, 
int destination_terminal)

This call queues a passenger with the given parameters.  The shuttle will then 
be able to pick up this passenger if the shuttle is currently at terminal 
initial_terminal, and the sum of all current passengers’ seats requirements, 
plus the passenger_type’s requirements, is at most 25.  Once they have been 
loaded, they may be unloaded once the shuttle is at destination_terminal.  This 
function returns 1 if the request is not valid (invalid argument range) and 0 
otherwise.

int stop_shuttle(void) 

This call deactivates the shuttle service. before the shuttle can be considered 
closed, it unloads all of its current passengers.  The call returns 1 if the 
shuttle is already deactivated or is in the process of unloading all its 
passengers for deactivation. The call returns 0 otherwise.

In addition to the above system calls, a proc file called terminal is created.
When /proc/terminal is read, it gives the following information: 

Status (OFFLINE / DEACTIVATING / MOVING / PARKED)
	OFFLINE - start_shuttle should be called to change the state
	DEACTIVATING - stop_shuttle has been called and the shuttle is unloading 
					passengers so that it can move to the OFFLINE state
	MOVING - the terminal is currently in route to a new destination
	PARKED - the terminal is currently loading or unloading passengers
Current seats used / available
Current number of passengers and their type
Current location [1-5] 
Destination location [1-5] 
Running total of passengers unloaded thus far and their types.
A summary of who is in queue at the five terminals, and delivered totals for 
each terminal.


****************************Instructions******************************

To compile, type "Make" in the module directory, and "insmod airport.ko" to 
insert the module. The proc file is now created and the shuttle is in OFFLINE
mode and is awaiting instructions from a userspace program.

*NOTE: it is still necessary to include the headers for the new syscalls
in the syscalls_32/64.tbl file as well as the syscalls.h file of the linux
kernel. Also you need to add the path to the shuttle syscalls/syscalls to your
Linux kernel makefile and "make; make install; reboot" before you will be able
to run this simulation.

Sample /proc/terminal output:

cat /proc/terminal

Status:		MOVING
Seats:		17 used 8 available
Passengers:	16 (4 adult with luggage, 6 adult without luggage, 6 children)
Location:	3
Destination:	2
Delivered:	72  (21 adult with luggage, 18 adult without luggage, 33 children)
-----
Terminal 1:1 adult with luggage, 2 adult without luggage, 0 children in queue.  
18 passengers delivered so far.
Terminal 2:3 adult with luggage, 1 adult without luggage, 4 children in queue.  
12 passengers delivered so far.
Terminal 3:1 adult with luggage, 1 adult without luggage, 2 children in queue.  
24 passengers delivered so far.
Terminal 4:1 adult with luggage, 2 adult without luggage, 1 children in queue.  
3 passengers delivered so far.
Terminal 5:0 adult with luggage, 0 adult without luggage, 3 children in queue.  
15 passengers delivered so far.


*************************Source Code Files****************************

For the module:
	shuttle_syscalls.c	-	for asmlinkage / syscall STUBS
	shuttle.c			-	shuttle module
	Makefile
	
For the userspace program:
	userspace.c			-	simple userspace program to test the syscalls with
							prompts and return code reporting
	timetest.c			-	basic program to test throughput of shuttle
	Makefile

*****************Known Errors And Missing Features********************

With more time we would have improved the scheduling algorithm to increase
the throughput of people through the shuttle. As of now it travels on a simple
1 - 2 - 3 - 4 - 5 - 4 - 3 - 2 - 1 pattern.
