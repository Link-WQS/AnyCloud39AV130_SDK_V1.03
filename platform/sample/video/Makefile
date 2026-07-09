# Project name

PROJ = ak_video_sample

LIBS = $(addprefix -l, $(LIB_DEP))

TARGET = ak_video_sample

ifneq ($(OS_TYPE), 0)
    TARGET = libak_video_sample.a
else
    TARGET = ak_video_sample
    LIB_DEP = plat_common  plat_mem plat_thread pthread  plat_log plat_osal mpi_venc plat_dbg app_video plat_vi

ifeq ($(CHIP_SERIES), AK3918AV100)
else ifeq ($(CHIP_SERIES), AK3918EV300L)
else ifeq ($(CHIP_SERIES), KM01A)
else ifeq ($(CHIP_SERIES), AK3918AV130)
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

SRCDIR += .

CSRCS += $(foreach d, $(SRCDIR), $(wildcard $d/*.c))

COBJS += $(patsubst %.c, %.o, $(CSRCS))

LIB_DIR = $(TOPDIR)/lib
#CORE_MODULE_LIB_DIR = $(CORE_MODULE_DIR)/lib

#LD_FLAGS = -Xlinker "-(" $(LIBS) -Xlinker "-)" -L$(LIB_DIR) -L$(CORE_MODULE_LIB_DIR) -L$(EXT_LIB_DIR)
LD_FLAGS = -Xlinker "-(" $(LIBS) -Xlinker "-)" -L$(LIB_DIR)

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
