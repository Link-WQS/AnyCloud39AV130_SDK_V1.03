#***************************************************配置说明*******************************************************
# 本project依赖的配置项包括以下五个方面的内容：
# 1、平台中间件的编译规则文件（必须由外部指定，固定为workbench中的rules.mk文件）
# 2、项目类型信息（Sky或者是Cloud，由外部提供，内置默认值）
# 3、芯片类型信息（由外部传入，内置默认值）
# 4、依赖关系（主要包含两个方面，其一是对驱动层的依赖，基二是对atc及三方库的依赖
# 5、平台中间件的安装目录
#******************************************************************************************************************

#**********************************************本部分内容设置外部依赖**********************************************
# 编译规则文件
RULES_MK_FILE=$(shell pwd)/../../rules.mk

# 项目类型信息
OS_TYPE=0

# 芯片类型信息
CHIP_SERIES=AK3918AV130

# core_module安装目录
PLATFORM_INSTALL_DIR=$(shell pwd)/../lib

# 导出变量
export RULES_MK_FILE OS_TYPE CHIP_SERIES PLATFORM_INSTALL_DIR

#******************************************************************************************************************


#******************************************本部分设置不同芯片需编译的模块******************************************
# Select module you needed
MEM                 = y
MEM_EX		        = n
THREAD              = y
LOG                 = y
COMMON              = y
AI                  = y
AO                  = y
VQE                 = y
VI                  = y
VO                  = n
TDE                 = n
ISPSDK              = y
VPSS                = y
AENC                = y
ADEC                = y
VENC                = y
VDEC                = n
OSD                 = y
OSD_EX              = y
MUX                 = y
DEMUX               = y
DBG                 = y
MD                  = y
DRV                 = n
MT                  = n
IR                  = y
PS_IR               = y
VIBE                = y
VIDEO               = y
UUID				= y
SVP               	= n
SVP2               	= y
VAD               	= y
MULTI_CIS           = y
FAST_AOV 		    = y
RTSP_SERVER 		    = y
#******************************************************************************************************************



#******************************************本部分设置项目类别相关的配置项******************************************
SAMPLE_INSTALL_DIR=$(shell pwd)/rootfs/rootfs/usr/bin
EXT_LIB_DIR=`pwd`/../ext_lib
export EXT_LIB_DIR SAMPLE_INSTALL_DIR

#******************************************************************************************************************
# demo and unit_test support
SAMPLE_SUPPORT      = y

# export define
export SAMPLE_SUPPORT
export MEM THREAD LOG COMMON AI AO VQE VI VO TDE ISPSDK VPSS AENC ADEC VENC VDEC OSD OSD_EX MUX DEMUX DBG TIMER MD DRV MT IR PS_IR VIBE MEM_EX VIDEO UUID SVP VAD MULTI_CIS FAST_AOV RTSP_SERVER
