obj-m += lsm6ds032.o as5048.o
spissd1306-objs := lsm6ds032.o as5048.o

KDIR = /lib/modules/`uname -r`/build

all:
	make -C $(KDIR) M=$(shell pwd) modules

clean:
	make -C $(KDIR) M=$(shell pwd) clean
