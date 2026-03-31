#!/bin/bash

# STM32H750 OpenOCD 启动脚本

DEBUGGER_TYPE="${1:-stlink}"  # stlink 或 jlink

if [ "$DEBUGGER_TYPE" = "stlink" ]; then
    echo "启动 OpenOCD - 使用 ST-Link..."
    openocd -f interface/stlink.cfg -f target/stm32h7x.cfg
elif [ "$DEBUGGER_TYPE" = "jlink" ]; then
    echo "启动 OpenOCD - 使用 J-Link..."
    openocd -f interface/jlink.cfg -f target/stm32h7x.cfg
else
    echo "用法: $0 [stlink|jlink]"
    echo "示例: $0 stlink"
    exit 1
fi
