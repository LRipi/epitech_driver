obj-m += Epitech_example.o

MAJOR= 240

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

load_module:
	# Clear the kernel log without echo
	sudo dmesg -C
	# Insert the module
	sudo insmod Epitech_example.ko
	# Display the kernel log
	dmesg

unload_module:
	# We put a — in front of the rmmod command to tell make to ignore
	# an error in case the module isn’t loaded.
	-sudo rmmod Epitech_example
	# Clear the kernel log without echo
	sudo dmesg -C

create_nod:
	sudo mknod /dev/Epitech_example c $(MAJOR) 0

build: all load_module create_nod

re: unload_module clean
	rm -rf /dev/Epitech_example
	$(MAKE) build
	gcc test.c -o test