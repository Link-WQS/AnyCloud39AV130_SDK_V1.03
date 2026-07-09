# anycloud39av130 SDK V1.03 版本 更改说明

发布时间: 2026/04

适用于 AK3918AV130L/AK3918AV130N/AK3918AV130A的B版本芯片的SDK V1.03版本。修改点如下：

## 内核

- 快启ramfs添加CONFIG_AK_PM_TIME_TEST配置。
- dts添加saradc的battery-divider参数配置。
- 修复uart1无法收发数据的问题。
- pll3分频系数调整，优化系统时钟配置。
- 38板双目gc3003的mipi通道对调，并修改对应的pwdn配置。
- fastsys模块添加ircut/led状态打印信息。
- 调整核间通讯接口的函数名。
- 新增接口用于快启功能时获取ircut/led当前状态信息结构体地址。
- 快启装载命令存在超过64字节的可能，FILES_MAX增加到96字节。
- 快启qs驱动调用ak_wait_fast_vi_close时提供ae参数的数量。
- 解决initfs使用lz4压缩快启时内核无法启动的问题。

## 驱动

- gc1084驱动的最大帧率修改为30fps。
- 修正夜视模式帧率异常。
- sensor驱动适配SC030IOT。
- 优化quick_start驱动中NPU功能代码。
- 修改gc20c3快启的power_on配置，优化快启时间。
- 更新视频编码库。
- 修改bridge持续缺失load frame时触发硬复位的次数为3次。
- 修复NNE使用某些开源模型时出现崩溃的问题。
- gc20c3 sensor驱动添加强制触发模式软件，支持PWM触发曝光控制。
- quick_start驱动添加修改ARGS分区的代码，支持快启参数动态更新。
- 增加bridge低概率硬复位后因缺少load frame中断而无法编码的容错处理。
- 增加操作rtc前先等待rtc校验完成的逻辑。
- 优化常电rtc校准等待。
- sc2331x sensor驱动新增软件修改sensor地址功能。
- quick_start驱动优化npu检测结果存储方式，改为链表存储。
- 优化时间戳打印。
- 修复aic8800无法连接5G的52/56/60/64信道的问题及AP信道无法开启成功的问题。
- 增加当quick_start驱动读dss.bin失败后，可以从ARGS分区回读并保存为文件的恢复机制。
- 解决gc20c3/gc3003 sensor驱动除零报错的问题。
- 更正quick_start驱动引用的头文件。
- 修改efuse的安全启动等级打印，适配AK3918AV130系列芯片。
- 修复get model head触发dma pool init的问题。
- quick_start驱动加载led驱动时不操作IO，不操作ircut/led/AIN的硬件，避免快启过程中的IO冲突。
- isp驱动添加三目mipi1 switch支持，之前只支持mipi0进行switch。
- quick_start驱动同步软硬光敏的判断逻辑。
- quick_start驱动获取小核ircut/led/daynight信息并提供给用户空间。
- 在mipi_csi_rx_dphy_recover中添加清pp_chx_done。
- quick_start驱动解决从设备节点获取dss参数返回值异常的问题。
- 修正快启enable NPU后会概率性崩溃的问题。
- isp驱动修复awb首帧不生效的问题。
- npu驱动集成支持33模型。
- quick_start驱动去除npu相关变量，优化快启npu逻辑。
- 更新最新编码库支持NumSlices设置，以及移除编码能力判断。
- pcm驱动优化加载时间。
- quick_start驱动调用ak_wait_fast_vi_close返回的结构体包含有效ae数量。
- isp驱动新增vblank调度执行aec_work功能。
- isp驱动加快AE收敛速度。
- isp驱动解决裁剪出现pp overflow的问题。
- 修改多目调试信息的打印级别为debug。
- quick_start驱动解决sc2337p双目采集异常的问题。
- npu驱动新增共享dma内存方案。
- quick_start驱动新增从小核获取多个CIS稳定ae参数的功能。
- mmc驱动支持控制器card busy中断检测逻辑，解决TF卡识别问题及clk/cmd波形异常问题。
- sc3332 sensor驱动修复快启首帧ISP亮度统计异常的问题。
- mmc驱动修复clk/d3检测下坏卡初始化失败后重复枚举的问题。
- quick_start驱动调整ae参数时修正exp/gain参数赋值错误。
- quick_start驱动优化思特微CIS首帧曝光参数，首帧需要满足曝光时间小于rbrows，大核重新计算AE参数。
- isp驱动解决mipi error概率横纹的问题。
- isp驱动解决sc双目快启概率出现MIPI error的问题。
- 修正快启svp逻辑。
- pp驱动解决单目主通道frame模式切换分辨率后第一帧概率图像花屏的问题。
- bridge驱动在handle_init和解绑时将miss_cnt清零。
- npu驱动优化create_chn加载模型参数临时内存释放逻辑。
- npu/nne驱动修复BPP卡死问题，新增chip id校验功能。
- bridge驱动统一接口命名。
- pp驱动解决主通道出现底部白纹问题。
- usb驱动修复wifi断流问题。
- sc2337p驱动修复设置rbrows时的约束条件。

## UBOOT

- 修复小核dss checksum错误的问题。
- 更正ARGS分区(DSS文件)校验失败的log。
- 支持普冉PY25Q128HA和PY25Q64HE双沿模式。
- autoupgrade默认GPIO改为GPIO56。
- 新增autoupgrade功能。
- 支持XTD25W64A norflash。
- 双核快启spl对dss/isp进行校验。
- uboot添加USB设备支持。

## 中间件

- 解决ak_venc_sample和ak_video_sample编码模式不支持CONST_QP和AVBR的问题。
- 修复vad sample在结束时获取帧失败的问题。
- 当驱动获取的最大曝光时间值异常时判定为失败。
- ao模块修复音频输出异常问题。
- 修正venc中间件对于读写锁的错误使用。
- svp2模块集成33模型，支持新的AI推理模型。
- 添加SVP2申请共享内存接口。
- osd颜色表支持4目。
- 修复哭声检测在长时间高CPU负载状态下可能失效的问题。
- 修正fast aov sample部分问题。
- ak_fast_aov_sample添加几个配置修改。
- 提交最新的rtsp sample。
- 打开svp的打印。
- 修改快启svp获取结果写法。
- 修改快启sample的svp逻辑。
- ak_fast_aov_sample支持jpeg抓拍功能。

## 小核(fastsys)

- ae_stable_range等于0时获取isp.conf的值。
- dss/isp.conf校验头与长度信息，确保配置文件完整性。
- sensor驱动修复双目快启概率性采集失败的问题。
- fastae_deamon修复双目保存yuv的功能。
- fastae模块目标亮度改为1档，优化AE收敛效果。
- awb模块优化，调用ak_isp_fast_awb_work接口提升白平衡效果。
- sensor驱动多sensor match支持，增强多摄像头匹配识别能力。
- trap模块新增栈回溯功能，便于调试定位问题。
- sample模块解决硬光敏判断为夜视但第二路未切换到夜视的问题，传vi_dev状态时遗漏led的type，解决强制灯控时参数赋值错误的问题。
- fastae模块软光敏逻辑添加强制灯光/白天夜视支持。
- fastae模块解决led_curstate未配置的问题。
- fastae模块添加硬光敏的逻辑。
- led驱动修复参数传递错误的问题。
- saradc驱动修复AV130的saradc采样问题。
- test模块新增top实现，支持系统监控。
- vi驱动去掉关闭时对0x08地址的操作。
- dss模块同步工具头文件的修改。
- tools模块脚本release时先编译生成库，再编列config编译验证，兼容旧版本的获取config.mk的方法。
- mmu模块二级页表支持。
- i2c/uart模块使用外部配置。
- mmu模块Domain1配置为禁止访问。
- vi驱动修复reboot卡死问题。
- sc3332 sensor驱动快启使用50fps小窗序列。
- fast_ae模块将小窗亮度稳定后的ae参数直接提供给大核。
- sys模块调试兼容两个版本编译的工程。
- fastsys模块优化编译工程，兼容多个配置。
- mmu模块代码段改为写穿模式。
- mmu模块修复采集阻塞的问题。
- gc20c3 sensor驱动最小曝光值设为1。

## 工具相关

- 更新烧录工具。
- 烧录工具支持1-3级别的安全启动。
- 修正USB烧录工具烧录因为默认配置不正确，导致使用默认配置烧录nand flash会异常的问题。
- 修正romfs烧录config的dtb名称为cloudOS.dtb。
- 增加安全启动固件制作工具。

# anycloud39av130 SDK V1.02 版本 更改说明

发布时间: 2026/01

适用于 AK3918AV130L和AK3918AV130N 的 B 版本芯片和AK3918AV130A芯片的SDK V1.02版本。修改点如下：

## 内核

- isp时钟最大支持频率更新为352Mhz。
- dts支持AV130A芯片。
- 支持nand flash。
- 新增快启功能。
- 支持快启ramfs和romfs文件系统。
- 支持双核异构。

## 驱动

- 整理电压复位档位配置。
- venc_adapter新增硬复位接口，解码 MPI 短接无法复位。
- venc模块新增硬复位接口，用于立即对模块进行复位。
- 添加软件绕过 ISP 使用 CopyMargin 功能的 bug 方案。
- 参考 svt 代码，在开启 tsensor 采集前将硬件校准写入。
- tsensor模块采用 cycle 模式替代软件的 8 次均值计算。
- 优化 ak_venc 代码逻辑。
- 修复3MP单目主次通道编码时，子通道使用3个buffer的rtsp测试导致主通道最终无编码的问题。
- 修复venc_adapter动态设置attr有概率OOPS崩溃的问题。
- 修复编码器不编码但一直设置属性导致内存持续减少的问题。
- 删除ak_venc在unbind_isp时reset编码器的down信号量操作。
- 修复停止编码煲机崩溃及通道解绑while循环长时间等待的问题。
- 修复jpeg抓拍因编码缓冲区不足导致通道卡住的问题。
- bridge模块添加 send frame 失败后将当前 buffer 删除的逻辑。
- 添加触发 mipierr 后，bridge 报错drop frame的恢复机制。
- 优化调试打印，添加编码类型打印。
- 双目 1/4 帧调整 timer3 启动的位置。
- 修改 npu 中锁的初始化流程。
- 去掉 wifi 端的丢中断打印提示。
- bridge模块优化调试打印。
- 修复get raw时osd花屏提交引发的get raw失败的问题。
- 升级venc模块编码库。
- 更新USB驱动。
- 修复fifo start计算错误导致海思wifi概率性启动失败的问题。

## UBOOT

- 修复env default -a命令保存后启动失败问题。

## 中间件

- 修复代码osd字符清除有水印的问题。
- 添加aenc适配pcm编码的逻辑。
- aenc模块在open时检查是否已经注册。
- 修复osd死锁及test case未通过的问题。
- 修复OSD编译错误。
- 修复osd字符绘制耗时大的问题。
- 修复打开OSD后主通道帧率不正常的问题。
- 添加av130芯片对画布高度的特殊说明。
- 修复SVP2的内存使用问题。
- 修复SVP2客观测试目标过多时出现段错误的问题。
- AI哭声检测修改probe的模式。
- 修复第一次Set VAD Attr后参数异常的问题。
- 修复VAD Get Attr时type异常的问题。
- 修复VAD Set Attr和Get Attr异常的问题。
- 修复白盒测试哭声检测时的崩溃问题。
- 修复AI CRY无法单独Enable的问题。
- 修复哭声检测无法Get Attr的问题。
- 修改音频采集模块长时间录音出现明显的数据左移的问题。

## 工具相关

- usb烧录工具支持AV100_V2 KM01X芯片类型，支持快启烧录。
- uart烧录工具支持AV100_V2 KM01X芯片类型，支持快启烧录。

# AnyCloud39AV130 SDK V1.01 版本 更改说明

发布时间:2025/10

支持AK3918AV130L和AK3918AV130N 的 B 版本芯片的SDKV1.01版本。修改点如下：

## 内核

- 支持AK3918AV130L和AK3918AV130N 的 B 版本芯片。
- 修复GPIO98 配置为按键报中断申请失败的问题。
- 添加gc20c3的dts支持。
- 解决reboot卡死的问题。
- 修复 WiFi 连接路由器后无法切换至其他路由器的问题。
- 修复“快速启动”内核无法启动的问题。
- 修复rtc节点编译报错的问题。
- 38模块dts增加rtc节点。

## 驱动

- isp支持get raw功能。
- 修复单目主通道完整帧4个buffer丢帧的问题。
- 修复mipi csi monitor持续报错无编码的问题。
- 增加 mipi csi 监控行数不足的统计。
- 优化多目非拼接模式的内存占用。
- 优化编码器逻辑，修复编码器概率性崩溃的问题。
- 新增 send frame 失败后删除当前 buffer 的机制。
- 新增触发 mipierr 后的 drop frame 恢复机制。
- 解决jpeg抓拍时编码缓冲区不足导致通道卡住的问题。
- bridge模块新增debug_flag，实现动态控制调试打印。
- 新增动态时间戳信息打印。  
- 适配 sc2337p 双目传感器新 slave 序列。
- 支持gc20c3。
- 修复gc1084双目丢帧的问题。
- 修复sc2331x驱动，使其兼容sc2331。
- 支持sc3332 CIS 代码编译。
- 新增硬接口用于立即对模块进行复位。
- efuse模块修复DATA1寄存器读bit[8:15]时addr配置错误的问题。
- 修复播放 PCM 时读取 efuse 导致音频中断的问题。
- 按 PG 要求修改安全等级烧写流程。
- 优化电源管理配置。
- 修复 tsensor 温度读数不准确的问题。  
- 修复draw方框宽度异常的问题。
- 更新 NNE 库，升级底层推理引擎，提升性能与稳定性。
- 重构usb驱动，解决专项测试遇到的问题。
- 解决双边沿中断触发异常的问题，修复按键中断误触发或漏触发的问题。
- 新增限制 MTU 最小值的功能，防止配置过低导致网络异常。

## UBOOT

- 修复 SFC 读写非 256 字节对齐数据时出现中断超时的问题。
- 修复clk检测配置未设置的问题。
- 修复env default -a命令保存后启动失败问题。

## 中间件

- 支持480×288分辨率的人形/人脸检测和320×192分辨率的手势检测模型。
- 修复语音活动检测（VAD）接口设置与获取时的异常问题。
- 修复白盒测试中哭声检测崩溃的问题。
- 支持AV130 哭声检测事件标志。
- 音频编解码库的MP3、AAC、AMR修改为动态注册模式。
- 修复SVP2的内存使用问题。
- 解决vqe和effect之间的重定义的问题。
- 修复哭声检测和音量检测超时相关的逻辑判断。
- 配置音频编解码库打印等级。
