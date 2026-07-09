#!/bin/sh

#
# 1. 使用说明
# mkfs.ubifs 相关参数如下
# -F：文件系统可用空间必须在第一次挂载时修复
# -m：页大小，根据NAND Flash手册实际的页大小进行配置
# -e：逻辑块大小=物理块大小-2*页大小=2048*64 - 2048*2 = 126976，根据实际情况进行配置
# -c: 最大逻辑块数量，即分区的分配块数量,此处50MB = 400个块 = 400*2048*64
# -r：指定目录构建文件系统
# -o：生成的镜像
./mkfs.ubifs -F -m 2048 -e 126976 -c 400 -r ./ubi_tmp/ -o tmp-ubifs.img
sleep 1
# ubinize 相关参数如下：
# -o ： 输出到镜像，用于烧录到开发板
# -m ： 页大小，根据nand flash手册实际的页大小进行配置
# -p ： 块大小,根据nand flash手册实际的页大小进行配置
# ubi.cfg 配置文件；
./ubinize -o ubifs.img -m 2048 -p 128KiB ubi.cfg

