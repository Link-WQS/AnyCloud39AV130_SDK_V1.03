ifndef KSRC
    KSRC := $(shell pwd)/../../kbd
endif
AUTOCONF := $(KSRC)/include/config/auto.conf
FEXIST := $(shell if [ -f $(AUTOCONF) ]; then echo "exist"; else echo "noexist"; fi;)
ifeq ("$(FEXIST)", "exist")
SOURCES = $(AUTOCONF)
include $(SOURCES)
endif

dir_y :=
ifeq ($(CONFIG_MACH_KM01A),y)
dir_y += sc200ai
dir_y += sc2336
dir_y += sc2336p
dir_y += sc2337p
dir_y += sc431hai
else ifeq ($(CONFIG_MACH_AK3918AV130),y)
dir_y += gc2063
dir_y += gc3003
dir_y += sc2337p
dir_y += sc2331x
dir_y += gc1084
dir_y += sc3332
dir_y += gc20c3
#dir_y += sc3336_sc1B5AK_sc2B3AK
#dir_y += mis20c1
dir_y += cv2005
dir_y += sc1346
#dir_y += f26a
#dir_y += gc0308
#dir_y += sc3332p
dir_y += sc030iot
else
dir_y += c2399
dir_y += c3390
dir_y += c4390
dir_y += c2599
dir_y += f22_f23_f28_f28p_f35_f37_f37p
dir_y += f53
dir_y += f38p
dir_y += h63_h63p
dir_y += mis2006
dir_y += mis2008
dir_y += mis4001
dir_y += mis5001
dir_y += sc200ai
dir_y += sc231hai
dir_y += sc2145
dir_y += sc2231
dir_y += sc2239
dir_y += sc223a
dir_y += sc223ap
dir_y += sc2332
dir_y += sc2331
dir_y += sc2331x
dir_y += sc2335
dir_y += sc2336
dir_y += sc2336p
dir_y += sc2336x
dir_y += sc3235
dir_y += sc3332
dir_y += sc3335
dir_y += sc3336
dir_y += sc3336p
dir_y += sc4236
dir_y += sc4336
dir_y += sc4336p
dir_y += sc401ai
dir_y += sc501ai
dir_y += sc5238
dir_y += sc5335p
dir_y += sc5336
dir_y += sc5336p
dir_y += sc1346
dir_y += sc132gs
dir_y += sc531ai
dir_y += sc431hai
dir_y += sc301iot
dir_y += os02g10
dir_y += ov02b10
dir_y += pr2000
dir_y += ps5268
dir_y += q03
dir_y += q03p
dir_y += ar0230
dir_y += gc2063
dir_y += gc4653
dir_y += gc4023
dir_y += gc0308
dir_y += gc032a
dir_y += gc2083
dir_y += gc1084
dir_y += gc3003
dir_y += tp9950
dir_y += k06
dir_y += k306p
dir_y += cv5001
dir_y += cv4002
dir_y += cv2003
dir_y += bf3238
endif

INSTALL :=
#KSRC := $(shell pwd)/../bd
export KSRC
export INSTALL

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
