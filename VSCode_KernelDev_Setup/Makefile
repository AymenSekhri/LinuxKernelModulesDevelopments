obj-m += Main.o
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
install:
	sudo rmmod Main.ko &> /dev/null
	sudo insmod Main.ko
	dmesg | tail -10 | grep "MyLinuxModule"
