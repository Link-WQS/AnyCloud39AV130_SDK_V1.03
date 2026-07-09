# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2000-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.

extra-y	= start.o
obj-y	= cpu.o cache.o

ifdef	CONFIG_SPL_BUILD
ifdef	CONFIG_SPL_NO_CPU_SUPPORT_CODE
extra-y	:=
endif
endif

obj-$(CONFIG_ARMADA100) += armada100/
obj-$(if $(filter lpc32xx,$(SOC)),y) += lpc32xx/
obj-$(CONFIG_MX25) += mx25/
obj-$(CONFIG_MX27) += mx27/
obj-$(if $(filter mxs,$(SOC)),y) += mxs/
obj-$(if $(filter spear,$(SOC)),y) += spear/
obj-$(CONFIG_AK37E) += ak37e/
obj-$(CONFIG_AK37D) += ak37d/
obj-$(CONFIG_AK39EV33X) += ak39ev33x/
obj-$(CONFIG_AK3918AV100) += ak3918av100/
obj-$(CONFIG_AK3918AV130) += ak3918av130/
obj-$(CONFIG_AK3918EV300L) += ak3918ev300l/
obj-$(CONFIG_AK39EV200) += ak39ev200/
obj-$(CONFIG_KM01A) += km01a/

# some files can only build in ARM or THUMB2, not THUMB1

ifdef CONFIG_$(SPL_)SYS_THUMB_BUILD
ifndef CONFIG_HAS_THUMB2

CFLAGS_cpu.o := -marm
CFLAGS_cache.o := -marm

endif
endif
