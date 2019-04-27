RPI_KERNEL_DIR ?= /lib/modules/`uname -r`/build/


obj-m+=pir.o

all:
  make -C $(RPI_KERNEL_DIR) M=`pwd` modules
  
clean:
  make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean
