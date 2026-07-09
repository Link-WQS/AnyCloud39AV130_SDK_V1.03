# DSS配置工具

这是一个用于处理DSS（Digital Surveillance System）配置文件的工具集，支持JSON和二进制格式之间的相互转换。

## 项目结构

```
dss_gen/
├── dss_config_tool.c     # 主要的配置工具源代码
├── dss.h                 # DSS数据结构定义头文件
├── Makefile              # 编译配置文件
├── README.md             # 说明文档
├── test_dss_tool.sh      # 自动化测试脚本
├── dss_config_clean.json # 清理过的JSON配置示例
├── dss_config.json       # 详细的JSON配置示例（含中文注释）
├── simple_test.json      # 简单的JSON配置示例
├── minimal_test.json     # 最小化的JSON配置示例
└── realistic_dss_config.json # 实际应用场景的JSON配置示例
```

## 编译

```bash
make
```

## 使用方法

### JSON转二进制
```bash
./dss_config_tool -j input.json output.bin
```

### 二进制转JSON
```bash
./dss_config_tool -b input.bin > output.json
```

### 显示帮助信息
```bash
./dss_config_tool -h
```

### 显示版本信息
```bash
./dss_config_tool -v
```

## JSON配置文件说明

JSON配置文件遵循`ak_qs_dss_info`结构体的定义，包含以下主要部分：

1. `qs_vi_info`: 视频输入设备配置
2. `qs_venc_info`: 视频编码器配置
3. `qs_video_info`: 视频信息配置
4. `isp_info`: ISP信息配置
5. `ircut`: IRCUT硬件参数
6. `led`: LED硬件参数
7. `hw_lumi_sensor`: 硬件光敏传感器参数

## 测试

运行自动化测试：
```bash
./test_dss_tool.sh
```

## 注意事项

1. JSON文件必须严格遵循`ak_qs_dss_info`结构体的定义
2. 所有数组大小必须与头文件中的宏定义保持一致
3. 字符串字段长度不能超过对应的MAX_SIZE定义
4. JSON格式不支持注释，如需保留注释请使用其他格式

## 更新日志

### v1.0.0
- 实现基本的JSON到二进制和二进制到JSON的转换功能
- 添加多个示例配置文件
- 提供自动化测试脚本
- 支持完整的`ak_qs_dss_info`结构体
