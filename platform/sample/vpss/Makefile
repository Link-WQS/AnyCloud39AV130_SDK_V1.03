dir_y := effect
dir_y += md
#dir_y += od
dir_y += mask
ifneq ($(CHIP_SERIES), AK39EV200)
dir_y += qr
dir_y += 3dnr_buf
endif
ifeq ($(OS_TYPE), 0)
dir_y += sps
dir_y += sps2
endif

all:
	@for i in $(dir_y); \
	do \
		$(MAKE) -C $$i; \
		if [ $$? -ne 0 ];then exit 1; fi \
	done

install:
	@for i in $(dir_y); \
	do \
		$(MAKE) -C $$i install; \
	done

clean:
	@for i in $(dir_y); \
	do \
		$(MAKE) -C $$i clean; \
	done

.PHONY: all install clean

