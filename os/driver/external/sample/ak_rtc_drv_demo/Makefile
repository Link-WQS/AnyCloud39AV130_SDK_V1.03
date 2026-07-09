####################################################################################################
#	@Copyright (C) 	Anyka 2019-12-16
#	@brief					Create Linux4.4.x linux driver test demo of module for AnyCloudv500
#	@Version				V1.0.0 
#####################################GCC Compile Flags #############################################
DEBUG := n
ifeq ($(DEBUG), y)
	CFLAGS := -g -O0
	CFLAGS += -DAK_MEM_DEBUG=1
	CXXFLAGS := -g -O0
else
	CFLAGS := -fno-strict-aliasing -Os
	CXXFLAGS := -fno-strict-aliasing -Os
endif

CFLAGS += -Wall -Werror -D_GNU_SOURCE -std=c99 -fms-extensions

#####################################GCC Objects ###################################################
#定义当前的工作目录
CUR_DIR=$(shell pwd)  
SRC_DIR=$(CUR_DIR)

#链接共享库库
#LOCAL_SHARED_LIB:= akaudiofilter

#链接静态库
#LOCAL_STATIC_LIB:= plat_ai plat_thread

#定义链接库的路径
#LOCAL_LDFLAGS := -L$(PLAT_LIB_DIR)
#LOCAL_LDFLAGS += $(addprefix $(PLAT_LIB_DIR)/,$(patsubst %,lib%.a,$(LOCAL_STATIC_LIB)))

#添加链接库路径到LDFLAGS
#LDFLAGS := $(LOCAL_LDFLAGS)
#LDFLAGS += $(addprefix -l,$(patsubst lib%,%,$(LOCAL_SHARED_LIB)))

AUTOCONF := $(KSRC)/include/config/auto.conf
FEXIST := $(shell if [ -f $(AUTOCONF) ]; then echo "exist"; else echo "noexist"; fi;)
ifeq ("$(FEXIST)", "exist")
SOURCES = $(AUTOCONF)
include $(SOURCES)
endif

ifeq ($(CONFIG_SYS_FAST_LAUNCH), y)
	STD_LDFLAGS = -lrt -lpthread -ldl -Wl,--gc-sections -static
else
#添加标准库参数到LDFLAGS
	STD_LDFLAGS = -lrt -lpthread -ldl -Wl,-rpath=/mnt -rdynamic
endif
LDFLAGS += $(STD_LDFLAGS)

#定义当前的工作目录头文件路径
#INC_DIR :=$(CUR_DIR)/include
INC_DIR := $(CUR_DIR)/
LOCAL_INCLUDES := $(INC_DIR)
CFLAGS += $(addprefix -I,$(LOCAL_INCLUDES))

#找到当前模块目录下的所有.c的源码文件
SRCS		:=	${wildcard *.c}

#将当前的所有源码文件，替换成不带目录路径的.o目标文件
OBJS		:=	${patsubst %.c, %.o, ${SRCS}}

#定义编译输出的模块镜像名称
TARGET := ak_rtc_drv_demo

#定义编译镜像的输出路径
BUILDPATH  := ./drvdemobd
WORKDIR:=$(shell pwd)
INSTALL_BIN_DIR ?= $(WORKDIR)/drvdemobd

#####################################Platform Related Compile Toolchain#############################
#定义工具链compile toolchain
CROSS_COMPILE ?= arm-anycloud-linux-uclibcgnueabi-
CC :=      $(CROSS_COMPILE)gcc
AS :=      $(CROSS_COMPILE)as
AR :=      $(CROSS_COMPILE)ar
#LD :=     $(CROSS_COMPILE)ld
LD :=      $(CC)
NM :=      $(CROSS_COMPILE)nm
OBJCOPY := $(CROSS_COMPILE)objcopy
OBJDUMP := $(CROSS_COMPILE)objdump
STRIP :=  $(CROSS_COMPILE)strip
CAT=     cat
RM :=     rm -rf
CP := 		cp -f
MKDIR=   mkdir
MV =     mv

#定义编译输出镜像的名称可以调整
#USER_MODULE_NAME ?= 
#ifneq ($(USER_MODULE_NAME),)
#MODULE_NAME := $(USER_MODULE_NAME)
#endif

#####################################Make Compile Module ###########################################
#定义编译链接过程
all:$(TARGET)  $(BUILDPATH)
	@echo "make all first step!"
	
$(TARGET):$(OBJS)
	@echo "make modules!"
	@echo $(OBJS)
	$(LD) $^ $(LDFLAGS) -o $@
	$(STRIP) $@

#定义c代码编译输出.o代码		
%.o : %.c
	$(CC) $(CFLAGS) -c $^ -o $@
	@echo ""

$(BUILDPATH):
	$(MKDIR) -p $(BUILDPATH)

	#cp ./ak_rtc_drv_demo ~/H322D_1/workbench/workspace/rootfs/utils/etc/config -rf

#.PHONY: all modules install clean test
.PHONY: all $(TARGET) install clean test

install:
	$(CP) $(TARGET) $(INSTALL_BIN_DIR)
	@echo ""
	
clean:
	$(RM) *.o *.bak
	$(RM) $(TARGET)
	$(RM) $(BUILDPATH)

test:
	@echo $(CUR_DIR)
	@echo $(SRCS)
	@echo $(OBJS)






