#!/bin/bash

# STM32H750 快速烧录脚本

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
DEBUGGER_TYPE="${1:-stlink}"  # stlink 或 jlink

if [ ! -f "$SCRIPT_DIR/build/Relay16CH_H750.elf" ]; then
    echo "错误: ELF文件不存在"
    echo "请先运行: bash build.sh"
    exit 1
fi

if [ "$DEBUGGER_TYPE" = "stlink" ]; then
    echo "使用 ST-Link 烧录..."
    openocd -f interface/stlink.cfg \
            -f target/stm32h7x.cfg \
            -c "program $SCRIPT_DIR/build/Relay16CH_H750.elf verify reset exit"
elif [ "$DEBUGGER_TYPE" = "jlink" ]; then
    echo "使用 J-Link 烧录..."
    openocd -f interface/jlink.cfg \
            -f target/stm32h7x.cfg \
            -c "program $SCRIPT_DIR/build/Relay16CH_H750.elf verify reset exit"
else
    echo "用法: $0 [stlink|jlink]"
    echo "示例: $0 stlink"
    exit 1
fi

echo "烧录完成！"
