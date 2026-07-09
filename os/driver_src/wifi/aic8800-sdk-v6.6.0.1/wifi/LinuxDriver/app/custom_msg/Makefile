ANYKA_FAST_AOV ?= y

ifeq ($(ANYKA_FAST_AOV), y)
LDFLAGS := -Wl,--gc-sections -static
else
LDFLAGS :=
endif

all: default

default:
	arm-anycloud-linux-uclibcgnueabi-gcc -c custom_msg.c
	arm-anycloud-linux-uclibcgnueabi-gcc $(LDFLAGS) -o custom_msg custom_msg.o
clean:
	rm custom_msg custom_msg.o

.PHONY: all clean default