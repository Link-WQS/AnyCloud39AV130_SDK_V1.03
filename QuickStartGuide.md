# AnyCloud39AV130 SDK #

----------

AnyCloud39AV130 SDK是基于嵌入式linux操作系统的软件开发包。

## 说明 ##

以下仅简单描述SDK的快速编译、开发入门、镜像烧录等步骤，更详细的说明请参考PDK包中的《AnyCloud 平台SDK用户开发手册》。

## 依赖 ##

1. 安装gcc交叉工具链，参考PDK包中的《AnyCloud 平台SDK用户开发手册》；

2. SDK的编译依赖以下第三方工具包，如果缺失，可按照以下步骤安装。以Ubuntu系统为例：

```
lib32ncurses5
lib32z1
u-boot-tools
libstdc++6
libncurses5-dev
liblzo2-dev:i386
mtd-utils
liblzma-dev:i386
bison
flex
```

## 快速编译 ##

1. 解压安装SDK包：tar -zxf AnyCloud39AV130_SDK_Vx.xx.tar.gz

2. 进入解压后的SDK根目录：cd ~/AnyCloud39AV130_SDK_Vx.xx

3. 用户根据自己的芯片，配置config.mk

4. 自动编译：./auto_build.sh

   注：默认编译包含内核源码、U-boot源码、中间件示例代码、驱动示例代码、文件系统镜像、应用示例源码。
   可通过编辑配置编译脚本auto_build.sh对编译项进行裁剪。

## 开发入门 ##

1. 编写应用代码

   参考AnyCloud39AV130_SDK/platform/sample目录下的中间件示例代码进行代码及Makefile编写。
   



## 烧录镜像 ##

### 常电版本SDK完成编译后将在AnyCloud39AV130_SDK_Vx.xx/image目录下生成以下镜像文件：
   * uImage：内核镜像文件；
   * u-boot.bin：全功能U-Boot目标镜像文件；
   * *.dtb：与目标开发板匹配的DTB二进制文件；
   * root.sqsh4：Linux标准的根文件系统；
   * usr.sqsh4：挂载到/usr/目录；
   * usr.jffs2：挂载到/etc/config/目录。

### 快启版本SDK完成编译后将在AnyCloud39AV130_SDK_Vx.xx/image目录下生成以下镜像文件：

#### Ramfs文件系统
   * u-boot-spl.bin：全功能U-Boot目标镜像文件；
   * *.dtb：与目标开发板匹配的DTB二进制文件；
   * uImage_fast：linux快启镜像；
   * initfs.img：初始化阶段的临时根文件系统；
   * config.jffs2：配置文件分区；
   * isp.part：ISP conf文件；
   * rootfs.ram：实际的ramfs根文件系统；
   * dss.img：快启参数文件；
   * fastsys_uImage：小核镜像。

#### Romfs文件系统
   * u-boot-spl.bin：全功能U-Boot目标镜像文件；
   * *.dtb：与目标开发板匹配的DTB二进制文件；
   * uImage_fast：linux快启镜像；
   * config.jffs2：配置文件分区；
   * isp.part：ISP conf文件；
   * rootfs.rom：romfs根文件系统；
   * dss.img：快启参数文件；
   * fastsys_uImage：小核镜像。
   