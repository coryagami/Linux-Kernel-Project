Cory Agami
Brandon Hillan(?)

Here we have our parts 1 and 2 for project 2.

In part 1 you have a c file that calls exactly 42 system calls, which is 20 
over the 22 that an empty main calls. Simply running the makefile here will
compile it and run strace on it, with results output in the log file.

In part 2 you have our my_date kernel module, which created a currentdate
file in /proc/ and when read, outputs the current kernel time in Y-m-d H:M:S
format. currentdate is created with the module and terminated with the module.
We used current_kernel_time() and time_to_tm() to get and parse the time data,
respectively. Also used %02 in the printf statemet to make sure the double-
digit outputs like H, M, S were prefaced with leading 0's if below 10, to
match date +"%Y-%m-%d %H:%M:%S".
