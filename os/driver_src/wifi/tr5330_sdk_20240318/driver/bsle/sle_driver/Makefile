CUR_MK_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
CUR_MK_DIR := $(dir $(CUR_MK_PATH))
CUR_MK_DIR := $(realpath $(CUR_MK_DIR))

include $(CUR_MK_DIR)/$(DIR_MAP_CONFIG_FILE)
include $(WSCFG_KCONFIG_CONFIG)
ifneq ($(KERNELRELEASE),)
$(warning defconfig: ================ARCH=$(WSCFG_ARCH_NAME) CROSS_COMPILE=$(WSCFG_CROSS_COMPILE)=======================)
obj-m := sle_soc.o
sle_soc-objs := sle_host_register.o
sle_soc-objs += sle_socket/sle_chan_kernel.o sle_hcc/sle_hcc_proc.o sle_chba/sle_hci_chba_proc.o sle_chba/sle_tm_chba_proc.o

_INCLUDES += -I$(PLATFORM_CFG_DIR)
_INCLUDES += -I$(PLATFORM_INC_DIR)/oal
_INCLUDES += -I$(PLATFORM_INC_DIR)/oal/linux
_INCLUDES += -I$(PLATFORM_DRV_DIR)/include
_INCLUDES += -I$(PLATFORM_DEVICE_DIR)/romable/include
_INCLUDES += -I$(PLATFORM_HCC_DIR)/cfg
_INCLUDES += -I$(PLATFORM_OSAL_DIR)/include
_INCLUDES += -I$(PLATFORM_HCC_DIR)/comm
_INCLUDES += -I$(PLATFORM_HCC_DIR)/host
_INCLUDES += -I$(PLATFORM_HCC_DIR)
_INCLUDES += -I$(PLATFORM_HCC_DIR)/inc
_INCLUDES += -I$(PLATFORM_LIBC_SEC_DIR)/include
_INCLUDES += -I$(CUR_MK_DIR)/sle_socket
_INCLUDES += -I$(CUR_MK_DIR)/sle_chba
_INCLUDES += -I$(CUR_MK_DIR)/sle_hcc
_INCLUDES += -I$(CUR_MK_DIR)/include
_INCLUDES += -I$(CUR_MK_DIR)

EXTRA_CFLAGS = $(_INCLUDES)
# CleanCode Build Option
EXTRA_CFLAGS += -Werror

EXTRA_CFLAGS += -include $(WSCFG_AUTOCONFIG_H) -include $(PLATFORM_INC_DIR)/oneimage/oneimage.h

else
$(warning defconfig: ================$(WSCFG_KERNEL_DIR)=======================)

STRIP := $(WSCFG_CROSS_COMPILE)strip

modules:
	make -C $(WSCFG_KERNEL_DIR) ARCH=$(WSCFG_ARCH_NAME) CROSS_COMPILE=$(WSCFG_CROSS_COMPILE) M=$(CUR_MK_DIR) modules KBUILD_MODPOST_WARN=1
	$(STRIP) --strip-unneeded sle_soc.ko

clean:
	rm -f *.o *.ko *.mod.c *.o.cmd .*.o.cmd .*.ko.cmd .*.mod.o.cmd *.symvers *.order *.tmp_versions
	rm -f sle_socket/*.o
	rm -f sle_socket/.*.o.cmd
	rm -f sle_chba/*.o
	rm -f sle_chba/.*.o.cmd
	rm -f sle_hcc/*.o
	rm -f sle_hcc/.*.o.cmd

endif