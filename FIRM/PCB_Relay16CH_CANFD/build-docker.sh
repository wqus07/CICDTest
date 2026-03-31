#!/bin/bash

# Docker: 构建编译镜像
docker build -t stm32h750-build:latest .

# Docker: 运行编译
docker run --rm -v $(pwd):/workspace stm32h750-build:latest bash -c "
    cd /workspace && \
    cmake --preset Debug -B build && \
    cmake --build build --config Debug
"

echo "编译完成！"
