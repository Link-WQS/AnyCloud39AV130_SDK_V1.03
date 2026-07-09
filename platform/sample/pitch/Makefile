# Project name

PROJ = ak_pitch_sample

LIB_DEP = plat_ai plat_vqe plat_common plat_log plat_thread pthread plat_mem akaudiofilter m plat_osal plat_dbg plat_audio_effect plat_audio_detect akresample akpitch
LIB_DEP += dl

LIBS = $(addprefix -l,$(LIB_DEP))

ifneq ($(OS_TYPE), 0)
    TARGET = lib$(PROJ).a
else
    TARGET = $(PROJ)
endif

TOPDIR 	?= $(shell pwd)/../..

# Flags

INCLUDE    =	-I. \
		-I../../include \
		-I../include \

#Locations

SRCDIR += .

CSRCS += $(foreach d, $(SRCDIR), $(wildcard $d/*.c))

COBJS += $(patsubst %.c, %.o, $(CSRCS))

LIB_DIR = $(TOPDIR)/lib

ifeq ($(CONFIG_FAST_MODE), no_fast)
    LD_FLAGS = -Xlinker "-(" $(LIBS) -Xlinker "-)" -L$(LIB_DIR)
else ifeq ($(CONFIG_FAST_MODE), fast_aov)
    LD_FLAGS = -Xlinker "-(" $(LIBS) -Xlinker "-)" -L$(LIB_DIR) -static
else ifeq ($(CONFIG_FAST_MODE), aov)
    LD_FLAGS = -Xlinker "-(" $(LIBS) -Xlinker "-)" -L$(LIB_DIR)
else ifeq ($(CONFIG_FAST_MODE), fast_uvc)
    LD_FLAGS = -Xlinker "-(" $(LIBS) -Xlinker "-)" -L$(LIB_DIR) -static
else ifeq ($(CONFIG_FAST_MODE), fast)
    LD_FLAGS = -Xlinker "-(" $(LIBS) -Xlinker "-)" -L$(LIB_DIR)
else 
	LD_FLAGS = -Xlinker "-(" $(LIBS) -Xlinker "-)" -L$(LIB_DIR)
endif

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
