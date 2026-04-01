# STM32H750 编译、下载、调试快速指南

## 当前状态

项目在 Windows 与 macOS 已验证可编译与调试。VSCode 已提供 CMake 任务与 Cortex-Debug 调试配置。

## 快速开始

### 1. 安装工具链

**Windows**

- 安装 Arm GNU Toolchain，并确保 `arm-none-eabi-gcc`、`arm-none-eabi-gdb` 在 `PATH` 中
- 安装 `ninja` 与 `openocd`
  - `winget install --id Ninja-build.Ninja --exact`
  - `winget install --id xpack-dev-tools.openocd-xpack --exact`

**macOS（推荐使用 osx-cross）**

```bash
brew tap osx-cross/arm
brew install arm-gcc-bin@10
brew install cmake ninja openocd
```

**Linux (Ubuntu/Debian)**

```bash
sudo apt-get update
sudo apt-get install gcc-arm-none-eabi gdb-arm-none-eabi binutils-arm-none-eabi
sudo apt-get install libnewlib-arm-none-eabi libstdc++-arm-none-eabi-newlib
sudo apt-get install cmake ninja-build openocd
```

### 2. 编译

**VSCode（推荐）**

- `Ctrl+Shift+B` → `CMake: Build Debug`

**命令行**

```bash
cmake --preset Debug -B build
cmake --build build --config Debug
```

### 3. 调试

- 连接 ST-Link 或 J-Link
- VSCode 选择 `Debug STM32H750 (ST-Link)` 或 `Debug STM32H750 (J-Link)`，按 `F5`

## Windows 与 macOS 兼容说明

- Windows 调试在 `launch.json` 中使用 `windows` 专用字段固定 `openocd` 与 `arm-none-eabi-gdb` 路径。
- macOS 保持使用 `PATH` 自动解析，原使用方式不变。

macOS 自检命令：

```bash
which arm-none-eabi-gdb
which openocd
arm-none-eabi-gdb --version
openocd --version
```

## 详细说明

详细使用指南请看：`FIRM/PCB_Relay16CH_CANFD/README_CN.md`
