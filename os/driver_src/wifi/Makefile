dir_y :=

AUTOCONF := $(KSRC)/include/config/auto.conf
FEXIST := $(shell if [ -f $(AUTOCONF) ]; then echo "exist"; else echo "noexist"; fi;)
ifeq ("$(FEXIST)", "exist")
SOURCES = $(AUTOCONF)
include $(SOURCES)
endif

ifeq ($(CONFIG_SYS_FAST_LAUNCH),y)
dir_y += aic8800-sdk-v6.6.0.1
else
#dir_y += rtl8188FU_linux_v5.7.4_33085.20190419
dir_y += rtl8188FU_linux_v5.15.3-6-g1a2e952f9.20230217
dir_y += rtl8188EUS_linux_v5.7.6.1_35670.20191106
#dir_y += rtl8189FS_linux_v5.7.9_35795.20191128
dir_y += rtl8189FS_linux_v5.15.12-7-g4c86cb3ae.20231020
#dir_y += atbm603X_wifi_Sv1997_Lv13216_20211201
dir_y += AtbmWifi_Driver_SVN20816_LMAC14449_20220606
dir_y += ATBM_WIFI_SVN3236_AsmLite19040_ARESM18826_Meru19052_20231208
dir_y += L.SMAC.19Q3.2053.01
#dir_y += rtl8192FU_rtl8725AU_WiFi_linux_v5.12.4-4-ga7451b386.20210111_COEX202001015-2005
#dir_y += ATBM_WIFI_SVN2936_AsmLite17993_ARESM17948_20230814
dir_y += ATBM_WIFI_SVN3352_AsmLite19040_ARESM18826_Meru19624_20240226
dir_y += AtbmWifi_ble_Driver_SVN3047_LMAC18260_20230922
dir_y += aic8800-sdk-v6.6.0.1
endif
INSTALL := 

# for L.SMAC.19Q3.2053.01  ssv6155p
KBUILD_TOP := $(shell pwd)/L.SMAC.19Q3.2053.01
SSV_DRV_PATH := $(shell pwd)/L.SMAC.19Q3.2053.01
# end for L.SMAC.19Q3.2053.01  ssv6155p 
export KBUILD_TOP
export SSV_DRV_PATH

export KSRC
export INSTALL

AUTOCONF := $(KSRC)/include/config/auto.conf
FEXIST := $(shell if [ -f $(AUTOCONF) ]; then echo "exist"; else echo "noexist"; fi;)
ifeq ("$(FEXIST)", "exist")
SOURCES = $(AUTOCONF)
include $(SOURCES)
endif 


all:
	@for i in $(dir_y); \
	do \
		$(MAKE) -C $$i; \
		if [ $$? -ne 0 ];then exit 1; fi \
	done

strip:
	@for i in $(dir_y); \
	do \
		$(MAKE) -C $$i strip; \
	done
clean:
	@for i in $(dir_y); \
	do \
		$(MAKE) -C $$i clean; \
	done

install:
	@rm -rf $(INSTALL)
	@mkdir $(INSTALL)
	@find . -name *.ko | xargs -i cp {} $(INSTALL)

help:
	@echo "Compile :"
	@echo "    --KSRC : Kernel Build Directory(Only Support Absolute Path)"
	@echo "    --examples : "
	@echo "           make KSRC=/home/test/linux/bd"
	@echo "Install :"
	@echo "    --INSTALL : Modules Intall Directory(Only Support Absolute Path)"
	@echo "    --examples : "
	@echo "           make install INSTALL=/home/test/linux/modules"

.PHONY: all clean help
