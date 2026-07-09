#!/usr/bin/env python3
import sys
import struct
import os

def calculate_crc(data):
    """计算字节累加和（CRC）"""
    return sum(data) & 0xFFFFFFFF  # 保持32位无符号

def main():
    print(f"Number of command-line arguments: {len(sys.argv)}")
    if len(sys.argv) < 20:
        print("Usage: python add_isp_header.py <input_file> <magic>  <isps_offset_0> ... <isps_offset_7> <valid_index_0> ... <valid_index_7> <valid_num>")
        print("Example: python add_isp_header.py isp.conf 0x12345678 0x00000000 0x00000004 0x00000008 0x0000000C 0x00000010 0x00000014 0x00000018 0x0000001C 0x01 0x01 0x01 0x01 0x01 0x01 0x01 0x01 0x08")
        sys.exit(1)

    # 解析参数
    input_file = sys.argv[1]
    magic = int(sys.argv[2], 16)  # 0x12345678
    # dcrc = int(sys.argv[3], 16)   # 传入的dcrc（实际会重新计算，但可传入用于校验）
    isps_offset = [int(sys.argv[3 + i], 16) for i in range(8)]
    valid_index = [int(sys.argv[11 + i], 16) for i in range(8)]
    valid_num = int(sys.argv[19], 16)

    # 读取原始文件内容
    try:
        with open(input_file, 'rb') as f:
            data = f.read()
    except FileNotFoundError:
        print(f"Error: File '{input_file}' not found.")
        sys.exit(1)

    # 计算实际 dcrc（字节累加和）
    actual_dcrc = calculate_crc(data)

    # 构造头信息（64字节）
    header = bytearray(64)

    # 填充 magic
    struct.pack_into('<I', header, 0, magic)

    # 保留 hcrc 位置（稍后计算）
    # struct.pack_into('<I', header, 4, 0)  # 暂时设为0，稍后填充

    # 填充 dcrc
    struct.pack_into('<I', header, 8, actual_dcrc)

    # 填充 isps_offset[8] (每个4字节)
    for i in range(8):
        struct.pack_into('<I', header, 12 + i * 4, isps_offset[i])

    # 填充 valid_index[8] (每个1字节)
    for i in range(8):
        header[44 + i] = valid_index[i]

    # 填充 valid_num
    header[52] = valid_num

    # 填充 reserved[11]（全0）
    for i in range(11):
        header[53 + i] = 0

    # 计算 hcrc：从 dcrc 开始到 reserved[10]（即从偏移8到偏移63）
    hcrc_data = header[8:64]  # 从 dcrc 开始，到末尾
    hcrc = calculate_crc(hcrc_data)

    # 填充 hcrc
    struct.pack_into('<I', header, 4, hcrc)

    # 输出结果文件（原文件名 + .pthdr）
    output_file = input_file + ".pthdr"
    with open(output_file, 'wb') as f:
        f.write(header)
        f.write(data)

    print(f"Header added successfully. Output file: {output_file}")

if __name__ == "__main__":
    main()
