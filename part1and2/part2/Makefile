obj-m := my_date.o

all:
	$(MAKE) -C /lib/modules/`uname -r`/build/ M=`pwd` modules

