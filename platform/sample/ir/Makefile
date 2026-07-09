# Project name

PROJ = ak_ir_sample

LIB_DEP = plat_common plat_mem plat_log plat_thread pthread plat_osal plat_dbg

LIBS = $(addprefix -l,$(LIB_DEP))

TARGET = ak_ir_sample

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

#LD_FLAGS = -Xlinker "-(" $(LIBS) -Xlinker "-)" -L$(LIB_DIR) -L$(CORE_MODULE_LIB_DIR)
LD_FLAGS = -Xlinker "-(" $(LIBS) -Xlinker "-)" -L$(LIB_DIR)

#build rule
.PHONY: all target install clean

all: $(COBJS)
	$(CC) $(COBJS) $(LD_FLAGS) -o $(TARGET)
	$(STRIP) $(TARGET)

install:
	@cp -f $(TARGET) $(SAMPLE_INSTALL_DIR)

clean : 
	$(RM) $(COBJS) $(TARGET)


# Rules
include $(RULES_MK_FILE)
