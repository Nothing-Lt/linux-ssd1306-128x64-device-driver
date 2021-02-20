obj-m := ssd1306.o
ssd1306-objs := lib_ssd1306.o ssd1306_fonts.o ssd1306_drv.o

KERNEL_MAKEFILE_PLACE = ~/ti-linux-kernel-dev/KERNEL

export ARCH=arm
export CROSS_COMPILE=arm-linux-gnueabihf-

all:
	make -C $(KERNEL_MAKEFILE_PLACE) M=$(PWD) modules

clean:
	make -C $(KERNEL_MAKEFILE_PLACE) M=$(PWD) clean