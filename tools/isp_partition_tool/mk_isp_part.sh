#!/bin/bash

# 检查是否至少传入一个文件
if [ $# -lt 1 ]; then
    echo "Usage: $0 [compression_type] <isp.conf> [compression_type] <isp.conf> ..."
    echo "Example: $0 lz4 isp1.conf gzip isp2.conf none isp3.conf"
    echo "Compression types: none, gzip, lz4 (others can be added)"
    exit 1
fi

# 初始化变量
files=()
compression_types=()
output_file="total_isp.conf.hdr"
rm -rf $output_file
#temp_dir=$(mktemp -d)
mkdir ./tmp
temp_dir=./tmp
 trap 'rm -rf "$temp_dir"' EXIT  # 确保临时目录在退出时被清理

# 解析参数
i=1
while [ $i -lt $# ]; do
    arg="${!i}"
    # 如果当前参数是压缩类型（非文件名），则下一个参数必须是文件
    if [[ "$arg" == "none" || "$arg" == "gzip" || "$arg" == "lz4" ]]; then
        i=$((i + 1))
        if [ $i -gt $# ]; then
            echo "Error: Missing file after compression type '$arg' '$i' '$#'"
            exit 1
        fi
        file="${!i}"
        if [ ! -f "$file" ]; then
            echo "Error: File '$file' not found"
            exit 1
        fi
        files+=("$file")
        compression_types+=("$arg")
    else
        # 如果不是压缩类型，则认为是文件名，但必须有前一个压缩类型
        if [ $i -eq 0 ]; then
            # 第一个参数就是文件名，没有指定压缩类型，默认为 none
            files+=("$arg")
            compression_types+=("none")
        else
            # 否则报错：文件名不能直接出现，必须有压缩类型前缀
            echo "Error: File '$arg' must be preceded by a compression type (e.g., gzip, lz4, none)"
            exit 1
        fi
    fi
    i=$((i + 1))
done

# 检查是否至少有一个文件
if [ ${#files[@]} -eq 0 ]; then
    echo "Error: No files provided."
    exit 1
fi


# 确保 fix_crc.py 存在
if [ ! -x "fix_one_isp_crc.py" ]; then
    echo "Error: fix_one_isp_crc.py not found or not executable. Please ensure it's in the same directory."
    exit 1
fi

# 为每个文件添加 mkimage header
echo "Processing files..."
offset=0  # 当前偏移量，初始为0
files_info=()  # 存储每个文件的: (temp_file, offset, index)
for i in "${!files[@]}"; do
    file="${files[i]}"
    comp_type="${compression_types[i]}"
    temp_file="$temp_dir/$(basename "$file").img"
    compressed_file="$temp_dir/$(basename "$file").compressed"

    # 根据压缩类型进行压缩（如果需要）
    case "$comp_type" in
        "none")
            # 直接复制原文件（不压缩）
            cp "$file" "$compressed_file"
            ;;
        "gzip")
            # 使用 gzip 压缩
            gzip -c "$file" > "$compressed_file"
            ;;
        "lz4")
            # 使用 lz4c 压缩
            if ! command -v lz4c &> /dev/null; then
                echo "Error: lz4c not found. Please install lz4 (e.g., apt install lz4)"
                exit 1
            fi
            lz4c "$file" "$compressed_file"
            ;;
        *)
            echo "Error: Unsupported compression type '$comp_type'"
            exit 1
            ;;
    esac

    # 检查压缩是否成功
    if [ ! -f "$compressed_file" ]; then
        echo "Error: Compression failed for file '$file' with type '$comp_type'"
        exit 1
    fi

    # 使用 mkimage 添加 header
    mkimage -A arm -O linux -T ramdisk -C none -n "ISP_CONFIG_$i" -d "$compressed_file" "$temp_file"

    if [ $? -ne 0 ]; then
        echo "Error: mkimage failed for file '$file' with compression '$comp_type'"
        exit 1
    fi

    # 用 Python 修改 header 的 CRC 为字节累计和
    python3 fix_one_isp_crc.py "$temp_file"
    if [ $? -ne 0 ]; then
        echo "Error: Failed to modify CRC for file '$file'"
        exit 1
    fi

    # 计算对齐到64字节的大小
    size=$(stat -c%s "$temp_file")
    padding=$(( (64 - (size % 64)) % 64 ))
    if [ $padding -ne 0 ]; then
        # 添加 padding
        dd if=/dev/zero bs=1 count=$padding >> "$temp_file"
    fi

    # 将处理后的文件添加到输出文件
    cat "$temp_file" >> "$output_file"

    # 记录当前文件信息：(temp_file, offset, index)
    files_info+=("$temp_file $offset $i")

    # 更新偏移量（为下一个文件准备）
    offset=$((offset + size + padding))

    echo "File $i: offset=$offset, size=$size, padding=$padding"

    echo `ls -lt $temp_file`
done


# 所有文件处理完后，一次性调用 Python 脚本添加偏移和序号
echo "Updating headers with offsets and indices..."

# 构造8个偏移（十六进制）和8个序号（十六进制）
offsets_hex=()
indices_hex=()

for i in "${!files[@]}"; do
    # 从 files_info 中提取偏移和序号
    temp_file="${files_info[i]}"
    offset_val=$(echo "$temp_file" | awk '{print $2}')
    # index_val=$(echo "$temp_file" | awk '{print $3}')
    # 使能位固定为1
    index_val=1

    # 转为十六进制字符串
    offsets_hex+=("0x$(printf '%x' $offset_val)")
    indices_hex+=("0x$(printf '%x' $index_val)")

done

# 补足到8个
while [ ${#offsets_hex[@]} -lt 8 ]; do
    offsets_hex+=("0x0")
    indices_hex+=("0x0")

    echo "++00"
done

# 转换为字符串，用空格分隔
offset_str="${offsets_hex[*]}"
index_str="${indices_hex[*]}"

# 获取所有 temp_file 路径
file_paths=()
for info in "${files_info[@]}"; do
    file_path=$(echo "$info" | awk '{print $1}')
    file_paths+=("$file_path")
done

echo "offset:"$offset_str""
echo "index:"$index_str""
echo "number of files:"$i""
# 调用 Python 脚本
#python3 add_isp_header.py "$offset_str" "$index_str" "${file_paths[@]}"
#python3 add_isp_pt_header.py ./total_isp.conf.hdr 0x49535048 0x00000000 "$offset_str" "$index_str" 0x01
python3 add_isp_pt_header.py ./total_isp.conf.hdr 0x49535048 $(printf '%s\n' $offset_str) $(printf '%s\n' $index_str) $((i+1))
#python3 add_isp_pt_header.py ./total_isp.conf.hdr 0x49535048 0x00000000 0x00000000 0x00000004 0x00000008 0x0000000C 0x00000010 0x00000014 0x00000018 0x0000001C 0x01 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x01

if [ $? -ne 0 ]; then
    echo "Error: Failed to update headers with offsets and indices"
    exit 1
fi

echo "Success: Generated $output_file with all ISP configs aligned to 64 bytes."
echo "Total files processed: ${#files[@]}"
