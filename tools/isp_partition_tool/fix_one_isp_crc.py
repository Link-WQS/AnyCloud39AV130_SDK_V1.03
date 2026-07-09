#!/usr/bin/env python3

import sys
import struct

def calculate_byte_sum(data):
    """计算字节累计和（8位无符号整数累加）"""
    return sum(data) & 0xFFFFFFFF  # 取低32位，避免溢出

def modify_header_crc(file_path):
    """修改文件头的 CRC 字段：先写入 dcrc（偏移24），再写入 hcrc（偏移4），hcrc 包含 8~63 字节"""
    with open(file_path, 'rb') as f:
        data = f.read()

    if len(data) < 64:  # mkimage header 至少64字节
        print(f"Error: File {file_path} too small to have header.")
        return False

    # 1. 计算 dcrc：从偏移 64 开始到文件末尾（数据部分）
    data_part = data[64:]  # 数据部分
    dcrc = calculate_byte_sum(data_part)

    # 2. 先写入 dcrc 到偏移 24（4字节）
    new_data = data[:24] + struct.pack('I', dcrc) + data[28:]

    # 3. 计算 hcrc：从偏移 8 到 63（header 的 8~63 字节）
    header_part = new_data[8:64]  # 56 字节，注意：dcrc 已写入，所以用 new_data
    hcrc = calculate_byte_sum(header_part)

    # 4. 写入 hcrc 到偏移 4（4字节）
    final_data = new_data[:4] + struct.pack('I', hcrc) + new_data[8:]

    # 5. 重新写入文件
    with open(file_path, 'wb') as f:
        f.write(final_data)

    return True

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python fix_one_isp_crc.py <file>")
        sys.exit(1)

    file_path = sys.argv[1]
    if modify_header_crc(file_path):
        print(f"Successfully modified CRC for {file_path}")
    else:
        sys.exit(1)