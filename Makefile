ifneq ($(KERNELRELEASE),)
obj-m += faulty.o
faulty-y := faulty_main.o

ccflags-y := -DDEBUG -Wall

else

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

endif
