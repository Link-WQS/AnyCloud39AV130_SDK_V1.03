ANYKA_FAST_AOV ?= y

ifeq ($(ANYKA_FAST_AOV), y)
LDFLAGS := -Wl,--gc-sections -static
else
LDFLAGS :=
endif

all: default

default:
	arm-anycloud-linux-uclibcgnueabi-gcc -c fasync_user.c
	arm-anycloud-linux-uclibcgnueabi-gcc $(LDFLAGS) -o fasync_user fasync_user.o
clean:
	rm -rf fasync_user fasync_user.o

.PHONY: all default clean