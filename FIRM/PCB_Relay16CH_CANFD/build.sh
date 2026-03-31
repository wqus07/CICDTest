#!/bin/bash

# STM32H750 快速开始脚本

set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

echo "========================================"
echo "STM32H750 编译工具链检查"
echo "========================================"

# 检查必要工具
tools=("arm-none-eabi-gcc" "arm-none-eabi-gdb" "arm-none-eabi-objcopy" "cmake" "ninja" "openocd")

for tool in "${tools[@]}"; do
    if command -v "$tool" &> /dev/null; then
        echo "✓ $tool 已安装"
    else
        echo "✗ $tool 未安装"
        exit 1
    fi
done

echo ""
echo "========================================"
echo "开始编译..."
echo "========================================"

# 清理旧build
rm -rf "$SCRIPT_DIR/build"

# 配置和编译
cmake --preset Debug -B "$SCRIPT_DIR/build"
cmake --build "$SCRIPT_DIR/build" --config Debug

echo ""
echo "========================================"
echo "编译完成！"
echo "========================================"
echo ""
echo "输出文件:"
echo "  ELF: $SCRIPT_DIR/build/Debug/Relay16CH_H750.elf"
echo ""
echo "可用命令:"
echo "  1. 查看二进制大小:"
echo "     arm-none-eabi-size $SCRIPT_DIR/build/Debug/Relay16CH_H750.elf"
echo ""
echo "  2. 生成HEX文件:"
echo "     arm-none-eabi-objcopy -O ihex $SCRIPT_DIR/build/Debug/Relay16CH_H750.elf $SCRIPT_DIR/build/Debug/Relay16CH_H750.hex"
echo ""
echo "  3. 烧录(需要OpenOCD服务器运行):"
echo "     arm-none-eabi-gdb $SCRIPT_DIR/build/Debug/Relay16CH_H750.elf"
echo "     (gdb) target extended-remote localhost:3333"
echo "     (gdb) load"
echo "     (gdb) quit"
echo ""
