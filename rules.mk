
# Cross Tools and libs
GCC_VERSION = 5.5.0
CROSS_TOOLCHAIN ?= arm-anycloud-linux-uclibcgnueabi
CROSS_PATH      ?= /opt/$(CROSS_TOOLCHAIN)/
CROSS_COMPILE    ?= arm-anycloud-linux-uclibcgnueabi-
ARM_LIBC_PATH	?=$(CROSS_PATH)/$(CROSS_TOOLCHAIN)/sysroot/usr/lib
ARM_LIBGCC_PATH ?=$(CROSS_PATH)/lib/gcc/$(CROSS_TOOLCHAIN)/$(GCC_VERSION)

# Tools
CC           = $(CROSS_COMPILE)gcc
CXX          = $(CROSS_COMPILE)g++
AS           = $(CROSS_COMPILE)as
AR           = $(CROSS_COMPILE)ar
LD           = $(CROSS_COMPILE)ld
RM           = rm -rf
MKDIR        = mkdir
OBJDUMP      = $(CROSS_COMPILE)objdump
OBJCOPY	     = $(CROSS_COMPILE)objcopy
STRIP        = $(CROSS_COMPILE)strip

ifeq ($(CHIP_SERIES), AK37D)
CFLAGS += -D__CHIP_AK37D_SERIES
endif

ifeq ($(CHIP_SERIES), AK39EV33X)
CFLAGS += -D__CHIP_AK39EV33X_SERIES
endif

ifeq ($(CHIP_SERIES), AK37E)
CFLAGS += -D__CHIP_AK37E_SERIES
endif

ifeq ($(CHIP_SERIES), AK3918AV100)
CFLAGS += -D__CHIP_AK3918AV100_SERIES
endif

ifeq ($(CHIP_SERIES), KM01A)
CFLAGS += -D__CHIP_KM01A_SERIES
endif

ifeq ($(CHIP_SERIES), AK3918AV130)
CFLAGS += -D__CHIP_AK3918AV130_SERIES
endif

ifeq ($(CHIP_SERIES), AK3918EV300L)
CFLAGS += -D__CHIP_AK3918EV300L_SERIES
endif

CFLAGS += -Werror -D_GNU_SOURCE -std=c99 -mlittle-endian  -fno-builtin -nostdlib -O2 -mlong-calls $(DEFINE) $(INCLUDE) $(GLB_INCLUDE)
ASFLAGS += -mlittle-endian -x assembler-with-cpp -O2 

export CLIB := $(ARM_LIBC_PATH)/libm.a $(ARM_LIBC_PATH)/libc.a  $(ARM_LIBGCC_PATH)/libgcc.a


# Rules


# --------------------------- s -> o
%.o:%.s
	@echo ---------------------[build $<]----------------------------------
	$(CC) -c $(ASFLAGS) $(CFLAGS) -o $@ $<

# ----------------------------- c -> o
%.o:%.c
	@echo ---------------------[build $<]----------------------------------
	$(CC) -c $(CFLAGS) $(INCLUDE) -o $@ $<

