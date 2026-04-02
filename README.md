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

- `launch.json`：Windows 使用 `windows` 字段固定 `openocd` 与 `arm-none-eabi-gdb` 绝对路径；macOS/Linux 通过 `PATH` 自动解析。
- `tasks.json`：Windows 使用 `powershell.exe` + WinGet PATH（配置在 `windows` 平台块内）；macOS/Linux 使用系统默认 shell（zsh/bash），工具链来自 Homebrew 或系统包管理器。

macOS 自检命令：

```bash
which arm-none-eabi-gdb
which openocd
arm-none-eabi-gdb --version
openocd --version
```

## UDS 固件升级

`FIRM/PCB_Relay16CH_CANFD/UDS/` 提供基于 CAN 总线的 UDS (ISO 14229) 固件升级模块：

- CAN ID：物理 `0x7E0`/`0x7E8`，功能 `0x7DF`（标准 11-bit）
- 支持服务：DiagSessionControl / SecurityAccess / RequestDownload / TransferData / RoutineControl / ECUReset
- 兼容 CANoe、PCAN-Explorer 等上位机工具进行刷写
- 独立模块，可移植到其他 STM32 项目

详细集成步骤和 CANoe CAPL 脚本示例请看 `FIRM/PCB_Relay16CH_CANFD/README_CN.md`。

## CI/CD

推送到 `main`/`develop` 或提交 PR 时，GitHub Actions 自动运行固件流水线（仅固件目录有变更时触发）：

| Job | 说明 |
|-----|------|
| **Build** | Debug + Release 双配置编译，报告 Flash/RAM 占用，Release 产物上传 |
| **Static Analysis** | cppcheck 扫描用户代码（warning / performance / portability） |
| **Size Diff** | PR 时对比 base 分支的二进制体积变化 |

工作流配置：`.github/workflows/firmware-ci.yml`

## 详细说明

详细使用指南请看：`FIRM/PCB_Relay16CH_CANFD/README_CN.md`
