#! /bin/sh

#########################################################################
#MTK
export PATH=$PATH:/opt/buildroot-gcc463/usr/bin
export ARCH=mipsel-linux
export COMPILER=/opt/buildroot-gcc463/usr/bin/mipsel-linux-


#########################################################################
#########################################################################
[ ! -d bin ] && mkdir bin

[ -d tools/fmac_tool ] && cd tools/fmac_tool && ./build.sh && cd - && cp -fv tools/fmac_tool/bin/* bin
[ -d tools/smac_tool ] && cd tools/smac_tool && ./build.sh && cd - && cp -fv tools/smac_tool/bin/* bin
