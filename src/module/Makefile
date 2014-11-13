obj-y := shuttle_syscalls.o

airport-y := shuttle.o
obj-m := airport.o

default:
	$(MAKE) -C /lib/modules/`uname -r`/build/ M=`pwd` modules
clean:
	$(MAKE) -C /lib/modules/`uname -r`/build/ M=`pwd` clean
