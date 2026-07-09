NET_DRIVER_DIR := wifi/LinuxDriver/aic8800_netdrv/
CUSTOM_MSG_DIR := wifi/LinuxDriver/app/custom_msg/
FASYNC_DEMO_DIR := wifi/LinuxDriver/app/fasync_demo/

KSRC ?= $(abspath ../../../kbd)
KSRC := $(abspath $(KSRC))

.PHONY: all clean $(NET_DRIVER_DIR) $(CUSTOM_MSG_DIR) $(FASYNC_DEMO_DIR)

all: $(NET_DRIVER_DIR) $(CUSTOM_MSG_DIR) $(FASYNC_DEMO_DIR)

$(NET_DRIVER_DIR):
	$(MAKE) -C $(NET_DRIVER_DIR) KSRC=$(KSRC)

$(CUSTOM_MSG_DIR):
	$(MAKE) -C $(CUSTOM_MSG_DIR)

$(FASYNC_DEMO_DIR):
	$(MAKE) -C $(FASYNC_DEMO_DIR)

clean:
	$(MAKE) -C $(NET_DRIVER_DIR) clean
	$(MAKE) -C $(CUSTOM_MSG_DIR) clean
	$(MAKE) -C $(FASYNC_DEMO_DIR) clean

