# Project name

PROJ = ak_svp2_sample

LIBS = $(addprefix -l, $(LIB_DEP))


#LIB_DEP += dl

ifneq ($(OS_TYPE), 0)
    TARGET = lib$(PROJ).a
else
    TARGET = ak_svp2_sample
    LIB_DEP = plat_common plat_log plat_mem plat_thread plat_vi plat_osal plat_vpss mpi_svp2 plat_dbg ak_nne plat_npu ak_mrd ak_object_filter 
    LIB_DEP += pthread
    
    ifeq ($(ISPSDK), y)
       LIB_DEP += plat_isp_sdk
    endif

endif



#LIBS = $(addprefix -l, $(LIB_DEP))

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
