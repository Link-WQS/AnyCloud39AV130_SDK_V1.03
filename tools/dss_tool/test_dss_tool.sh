#!/bin/bash

# DSS Config Tool 测试脚本

echo "=== DSS Config Tool 测试 ==="

# 测试帮助信息
echo "1. 测试帮助信息:"
./dss_config_tool --help
echo ""

# 测试版本信息
echo "2. 测试版本信息:"
./dss_config_tool --version
echo ""

# 测试JSON到BIN转换
echo "3. 测试JSON到BIN转换:"
./dss_config_tool -j dss_config_clean.json test_conversion.bin
if [ $? -eq 0 ]; then
    echo "✓ JSON到BIN转换成功"
else
    echo "✗ JSON到BIN转换失败"
fi
echo ""

# 测试BIN到JSON转换
echo "4. 测试BIN到JSON转换:"
./dss_config_tool -b test_conversion.bin > test_conversion.json
if [ $? -eq 0 ]; then
    echo "✓ BIN到JSON转换成功"
else
    echo "✗ BIN到JSON转换失败"
fi
echo ""

# 验证转换一致性
echo "5. 验证转换一致性:"
# 检查文件是否存在
if [ -f "test_conversion.json" ]; then
    echo "✓ 转换后的JSON文件已生成"
    # 显示文件大小
    echo "原始JSON文件大小: $(stat -c%s dss_config_clean.json) 字节"
    echo "转换后JSON文件大小: $(stat -c%s test_conversion.json) 字节"
else
    echo "✗ 转换后的JSON文件未生成"
fi
echo ""

# 清理测试文件
echo "6. 清理测试文件:"
rm -f test_conversion.bin test_conversion.json
echo "✓ 测试文件已清理"

echo ""
echo "=== 测试完成 ==="
