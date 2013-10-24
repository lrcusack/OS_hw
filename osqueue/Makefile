obj-m += chardev.o

all:	
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	
device:
	mknod /dev/osqueue c 253 0
	
	
	
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
