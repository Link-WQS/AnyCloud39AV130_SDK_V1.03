# Project name

PROJ = ak_rtsp_server_sample

LIBS = $(addprefix -l, $(LIB_DEP))


ifneq ($(OS_TYPE), 0)
    TARGET = libak_rtsp_server_sample.a
    LIB_DEP = plat_common  plat_mem plat_thread pthread  plat_log plat_osal mpi_venc akv_encode plat_dbg
else
    TARGET = ak_rtsp_server_sample
ifeq ($(CHIP_SERIES), AK3918AV100)
	LIB_DEP = plat_common  plat_mem plat_thread pthread  plat_log plat_osal mpi_venc plat_dbg rtsp_server app_its app_video plat_timer
else ifeq ($(CHIP_SERIES), KM01A)
	LIB_DEP = plat_common  plat_mem plat_thread pthread  plat_log plat_osal mpi_venc plat_dbg plat_isp_sdk plat_vpss rtsp_server plat_audio_detect plat_audio_effect rtsp_server app_its app_video plat_timer
else ifeq ($(CHIP_SERIES), AK3918EV300L)
	LIB_DEP = plat_common  plat_mem plat_thread pthread  plat_log plat_osal mpi_venc plat_dbg rtsp_server app_its app_video plat_timer
else ifeq ($(CHIP_SERIES), AK3918AV130)
	LIB_DEP = plat_common  plat_mem plat_thread pthread  plat_log plat_osal mpi_venc plat_dbg plat_audio_detect rtsp_server app_its app_video plat_timer
else
	LIB_DEP = plat_common  plat_mem plat_thread pthread  plat_log plat_osal mpi_venc akv_encode plat_dbg rtsp_server app_its app_video plat_timer
endif
    
    ifeq ($(VI), y)
    	LIB_DEP += plat_vi
    endif

    ifeq ($(ISPSDK), y)
        LIB_DEP += plat_isp_sdk
    endif

    ifeq ($(VPSS), y)
    	LIB_DEP += plat_vpss
    endif

    ifeq ($(AENC), y)
    	LIB_DEP += mpi_aenc plat_ai plat_vqe akaudiofilter akaudiocodec akmp3 akaac akamr
    endif
endif



#LIBS = $(addprefix -l, $(LIB_DEP))
LIBS += -L./lib/
TOPDIR  ?= $(shell pwd)/../..

# Flags

INCLUDE    =	-I. \
		-I../../include_inner \
		-I../../include \
		-I../include \
        -I../../ext_lib/include \
        -I../../dep_lib/include \
		-I$(EXT_LIB_DIR)/include \

#Locations

SRCDIR += .

CSRCS += $(foreach d, $(SRCDIR), $(wildcard $d/*.c))

COBJS += $(patsubst %.c, %.o, $(CSRCS))

LIB_DIR = $(TOPDIR)/lib

#LD_FLAGS = $(LIBS) -L$(LIB_DIR)

LD_FLAGS = -Xlinker "-(" $(LIBS) -Xlinker "-)" -L$(LIB_DIR)
ifeq ($(OS_TYPE), 0)
LD_FLAGS += -L$(EXT_LIB_DIR)
endif

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

ifndef RULES_MK_FILE
RULES_MK_FILE=$(TOPDIR)/../../rules.mk
endif


# Rules
include $(RULES_MK_FILE)
