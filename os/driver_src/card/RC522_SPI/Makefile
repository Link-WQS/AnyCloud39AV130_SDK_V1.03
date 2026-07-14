########################################################################################################
# Build RC522 as an out-of-tree kernel module so it can be loaded manually with insmod/modprobe.
########################################################################################################

EXTRA_CFLAGS += $(USER_EXTRA_CFLAGS)
EXTRA_CFLAGS += -O1
EXTRA_CFLAGS += -Wall
EXTRA_CFLAGS += -Wno-unused-variable
EXTRA_CFLAGS += -Wno-unused-value
EXTRA_CFLAGS += -Wno-unused-label
EXTRA_CFLAGS += -Wno-unused-parameter
EXTRA_CFLAGS += -Wno-unused-function
EXTRA_CFLAGS += -Wno-unused
EXTRA_CFLAGS += -Wno-error=date-time
EXTRA_CFLAGS += -Wno-date-time
EXTRA_CFLAGS += -fno-common

EXTRA_LDFLAGS += --strip-debug

CUR_DIR := $(shell pwd)
SRC_DIR := $(CUR_DIR)

MODULE_NAME := spi_rc522
OBJS := RC522_SPI.o

CONFIG_PLATFORM_ARM_ANYKA = y

ifeq ($(CONFIG_PLATFORM_ARM_ANYKA), y)
EXTRA_CFLAGS += -DCONFIG_LITTLE_ENDIAN
ARCH := arm
CROSS_COMPILE ?= arm-anycloud-linux-uclibcgnueabi-
ifndef KSRC
    KSRC := $(shell pwd)/../../../kbd
endif
endif

USER_MODULE_NAME ?= spi_rc522
ifneq ($(USER_MODULE_NAME),)
MODULE_NAME := $(USER_MODULE_NAME)
endif

ifneq ($(KERNELRELEASE),)

$(MODULE_NAME)-objs := $(OBJS)
obj-m := $(MODULE_NAME).o

else

all: modules strip

modules:
	$(MAKE) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KSRC) M=$(shell pwd) modules

strip:
	@if [ -f $(MODULE_NAME).ko ]; then $(CROSS_COMPILE)strip $(MODULE_NAME).ko --strip-unneeded; fi;

.PHONY: all modules clean

clean:
	rm -rf *.o* *.ko* *.mod.c *.mod.* *.bak *.order *.symvers
	rm -rf .*.o.cmd .*.ko.cmd .*.mod.o.cmd
	rm -rf .tmp_versions

endif
