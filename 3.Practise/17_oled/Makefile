obj-m+=myoled_drv.o
myoled_drv-objs+= oled_drv.o oled.o
KERNEL_DIR:=/home/hewei/6818GEC/kernel
CROSS_COMPILE:=/home/hewei/6818GEC/prebuilts/gcc/linux-x86/arm/arm-eabi-4.8/bin/arm-eabi-
PWD:=$(shell pwd)

default:
	$(MAKE) ARCH=arm CROSS_COMPILE=$(CROSS_COMPILE) -C $(KERNEL_DIR) M=$(PWD) modules 
	
clean:
	rm  *.o *.order .*.cmd  *.mod.c *.symvers .tmp_versions -rf