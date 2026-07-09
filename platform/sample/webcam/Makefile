# Project name

PROJ = ak_webcam_sample

LIBS = $(addprefix -l, $(LIB_DEP))

TARGET = ak_webcam_sample

# Funtion
WEBCAM_VIDEO=y
WEBCAM_AUDIO=y

ifneq ($(OS_TYPE), 0)
    TARGET = webcam.a
else
    TARGET = ak_webcam_sample
 	LIB_DEP = plat_common plat_mem plat_thread pthread  plat_isp_sdk plat_log plat_osal mpi_venc plat_dbg app_video plat_vi  plat_ai  plat_ao plat_vqe  akaudiofilter plat_timer 
ifeq ($(CHIP_SERIES), AK3918AV100)
else ifeq ($(CHIP_SERIES), AK3918EV300L)
else ifeq ($(CHIP_SERIES), AK3918AV130)
	LIB_DEP += plat_audio_detect
else
	LIB_DEP += akv_encode 
endif

ifneq ($(CHIP_SERIES), AK37E)
    LIB_DEP += plat_isp_sdk plat_vpss
endif
endif

TOPDIR 	?= $(shell pwd)/../..

# Flags

INCLUDE    =	-I. \
		-I../../include_inner \
		-I../../include \
		-I$(CORE_MODULE_DIR)/include \
		-I$(CORE_MODULE_DIR)/include_inner \

#Locations
INCLUDE += -I$(shell pwd)/../../../os/driver/external/include/
INCLUDE += -I$(shell pwd)/../../../../../os/driver/include/

SRCDIR += .

ifeq ($(WEBCAM_VIDEO),y)
#SRCDIR += video
CFLAGS += -DWEBCAM_VIDEO
endif

ifeq ($(WEBCAM_AUDIO),y)
#SRCDIR += audio
CFLAGS += -DWEBCAM_AUDIO
endif

CSRCS += $(foreach d, $(SRCDIR), $(wildcard $d/*.c))

COBJS += $(patsubst %.c, %.o, $(CSRCS))

LIB_DIR = $(TOPDIR)/lib
#CORE_MODULE_LIB_DIR = $(CORE_MODULE_DIR)/lib

LD_FLAGS = -Xlinker "-(" $(LIBS) -Xlinker "-)" -L$(LIB_DIR) -L$(CORE_MODULE_LIB_DIR) -L$(EXT_LIB_DIR)
#LD_FLAGS = -Xlinker "-(" $(LIBS) -Xlinker "-)" -L$(LIB_DIR) -g -Wl,--gc-sections -static

#build rule
.PHONY: all target install clean

all: $(COBJS)
ifneq ($(OS_TYPE), 0)
	$(AR) -rsv $(TARGET) $(COBJS)
	@cp -r $(TARGET) $(TOPDIR)/lib
else
	$(CC) $(COBJS) $(LD_FLAGS) -o $(TARGET)
	$(STRIP) $(TARGET)
endif

install:
ifeq ($(OS_TYPE), 0)
	@cp -f $(TARGET) $(SAMPLE_INSTALL_DIR)
else
	@cp -f $(TARGET) $(PLATFORM_INSTALL_DIR)/lib
endif

clean : 
	$(RM) $(COBJS) $(TARGET)


# Rules
include $(RULES_MK_FILE)
CFLAGSTMP  = $(shell echo $(CFLAGS) | sed -e 's/c99/gnu99/g')
CFLAGS     := $(CFLAGSTMP)
