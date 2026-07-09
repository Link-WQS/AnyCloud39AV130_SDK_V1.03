# Project name

PROJ = ak_mem_ex_sample

LIB_DEP =  mpi_venc app_mem_ex plat_vi plat_vpss plat_isp_sdk plat_common plat_mem plat_log plat_thread plat_osal plat_dbg

ifneq ($(OS_TYPE), 0)
    TARGET = lib$(PROJ).a
else
    TARGET = $(PROJ)
    LIB_DEP += pthread
endif

LIBS = $(addprefix -l, $(LIB_DEP))

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

#LD_FLAGS = $(LIBS) -L$(LIB_DIR)
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
