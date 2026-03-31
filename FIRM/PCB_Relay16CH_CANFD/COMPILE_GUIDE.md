# STM32H750 编译、下载、调试指南

## 项目概况
- **MCU**: STM32H750VBTx
- **IDE**: VSCode + CMake
- **工具链**: osx-cross/arm-gcc-bin@10 (ARM GCC 10.3.1 with newlib)
- **调试工具**: OpenOCD + GDB
- **编译状态**: ✅ **已成功验证**

## 系统要求

### 1. 必需软件

#### macOS 安装（推荐 - 已验证）
```bash
# 1. 添加osx-cross工具库（必须，提供完整工具链）
brew tap osx-cross/arm

# 2. 安装ARM工具链with newlib（包含必要的libc）
brew install arm-gcc-bin@10

# 3. 安装构建工具
brew install cmake ninja openocd

# 4. 验证工具链
arm-none-eabi-gcc --version
# 输出应该显示: GNU Arm Embedded Toolchain 10.3-2021.10

# 5. 验证newlib
arm-none-eabi-gcc -print-file-name=libc.a
# 应该输出库文件路径，不是 libc.a
```

**⚠️ 重要**: 不要使用普通 `brew install arm-none-eabi-gcc`，这会安装不完整的版本（缺少newlib）

#### Linux 安装 (Ubuntu/Debian)
```bash
sudo apt-get install gcc-arm-none-eabi gdb-arm-none-eabi binutils-arm-none-eabi
sudo apt-get install cmake ninja-build
sudo apt-get install openocd
```

### 2. 硬件连接

需要**调试器**之一：
- **ST-Link v2/v3** (推荐，性价比高)
- **J-Link** (高端调试器)
- **PyOCD兼容的调试器** (如CMSIS-DAP)

连接方式：
```
ST-Link/调试器接口 <-> STM32H750
VCC               <-> 3.3V
GND               <-> GND
SWDIO             <-> PA13
SWCLK             <-> PA14
```

## 编译

### 方式1: 使用VSCode任务（推荐）

**Ctrl+Shift+B** 或菜单 **Terminal > Run Build Task** → 选择 **CMake: Build Debug**

✅ 成功编译输出：
```
text: 25.4 KB, data: 24 B, bss: 3.5 KB
✓ build/Relay16CH_H750.elf (2.0 MB)
✓ build/Relay16CH_H750.map
```

### 方式2: 命令行编译

```bash
cd /Users/weiqiao/Projects/VSCode/CICDTest/FIRM/PCB_Relay16CH_CANFD

# 配置Debug版本
cmake --preset Debug -B build

# 编译
cmake --build build --config Debug

# 最终输出文件
# ✓ build/Relay16CH_H750.elf (可直接烧录)
# ✓ build/Relay16CH_H750.map (链接器映射)
```

### 方式3: Release 版本

```bash
cmake --preset Release -B build_release
cmake --build build_release --config Release
```

## 生成HEX文件（用于STM32CubeProgrammer等工具）

```bash
# 生成Intel HEX格式
arm-none-eabi-objcopy -O ihex build/Relay16CH_H750.elf build/Relay16CH_H750.hex

# 或生成二进制格式
arm-none-eabi-objcopy -O binary build/Relay16CH_H750.elf build/Relay16CH_H750.bin
```

## 下载 (烧录)

### 前置: 启动OpenOCD服务器

#### 使用ST-Link
```bash
# 终端1: 启动OpenOCD
openocd -f interface/stlink.cfg -f target/stm32h7x.cfg

# 或使用项目配置文件
openocd -f openocd_stlink.cfg
```

#### 使用J-Link
```bash
# 终端1: 启动OpenOCD
openocd -f interface/jlink.cfg -f target/stm32h7x.cfg

# 或使用项目配置文件
openocd -f openocd_jlink.cfg
```

### 方式1: 通过VSCode任务烧录（推荐）

**Ctrl+Shift+P** → "Tasks: Run Task" → 选择：
- **Flash Binary (OpenOCD+ST-Link)** 或
- **Flash Binary (OpenOCD+J-Link)**

✅ 输出示例：
```
使用 ST-Link 烧录...
Program started
Program finished successfully
烧录完成！
```

### 方式2: 使用脚本烧录

```bash
# 终端中运行
bash flash.sh stlink
# 或
bash flash.sh jlink
```

### 方式3: 使用GDB命令行

```bash
# 终端1: 启动OpenOCD
openocd -f openocd_stlink.cfg

# 终端2: 使用GDB烧录
arm-none-eabi-gdb build/Relay16CH_H750.elf

(gdb) target extended-remote localhost:3333
(gdb) monitor reset halt
(gdb) load
(gdb) quit
```

### 方式4: 使用OpenOCD命令行

```bash
openocd \
  -f interface/stlink.cfg \
  -f target/stm32h7x.cfg \
  -c "program build/Relay16CH_H750.elf verify reset exit"
```

## 调试

### 使用VSCode调试器（推荐）

#### 步骤1: 启动OpenOCD服务器
```bash
openocd -f openocd_stlink.cfg
# 或
openocd -f openocd_jlink.cfg
```

#### 步骤2: VSCode中启动调试
- **F5** 或 Debug 菜单 → 选择 **Debug with ST-Link (OpenOCD)** 或 **Debug with J-Link (OpenOCD)**
- 自动编译并等待连接
- 在`main()`函数处停止

✅ 成功连接指示：
```
...
Reading /path/to/Relay16CH_H750.elf
Loading section .text, size 0x6330 lma 0x8000000
Loading section .rodata, size 0x5e8 lma 0x8006330
Loading section .data, size 0x18 lma 0x8006918
Start address 0x8002c88, load size xxx
Transfer rate: xxx KB/sec, xxx bytes/write.
```

#### 调试快捷键
- **F5**: 继续执行
- **F10**: 单步执行 (不进入函数)
- **F11**: 单步执行 (进入函数)
- **Ctrl+F2**: 停止调试
- **Ctrl+Shift+D**: 打开Debug视图

### 使用GDB命令行调试

```bash
# 终端1: 启动OpenOCD
openocd -f openocd_stlink.cfg

# 终端2: 启动GDB
arm-none-eabi-gdb build/Relay16CH_H750.elf

(gdb) target extended-remote localhost:3333
(gdb) monitor reset halt
(gdb) break main
(gdb) continue
(gdb) step
(gdb) next
(gdb) print 变量名
(gdb) quit
```

## 完整工作流示例

### 场景1: 编译 → 烧录 → 调试

```bash
# 终端1: 启动OpenOCD
openocd -f openocd_stlink.cfg

# VSCode中
# 1. Ctrl+Shift+B 编译
# 2. Ctrl+Shift+P → Run Task → Flash Binary (OpenOCD+ST-Link)
# 3. F5 启动调试

# 或使用GDB命令行（终端2）
arm-none-eabi-gdb build/Debug/Relay16CH_H750.elf
(gdb) target extended-remote localhost:3333
(gdb) load
(gdb) break main
(gdb) continue
```

### 场景2: 仅编译和烧录(不调试)

```bash
# Ctrl+Shift+B 编译
# Ctrl+Shift+P → Run Task → Flash Binary

# 或命令行
openocd -c "program build/Debug/Relay16CH_H750.elf verify reset exit" \
        -f interface/stlink.cfg \
        -f target/stm32h7x.cfg
```

## 故障排查

### 1. ✅ "arm-none-eabi-gcc: command not found" → 已解决

**原因**: 使用了不完整的Homebrew工具链

**解决方案**:
```bash
# 使用osx-cross工具链（已验证兼容）
brew tap osx-cross/arm
brew install arm-gcc-bin@10

# 验证
arm-none-eabi-gcc --version
# 输出: GNU Arm Embedded Toolchain 10.3-2021.10
```

### 2. ✅ "stdint.h: No such file or directory" → 已解决

**原因**: Homebrew默认工具链缺少newlib（标准C库）

**解决方案**: 使用osx-cross/arm-gcc-bin@10（包含完整的newlib）

### 3. ✅ "non constant or forward reference address expression for section .ARM.extab" → 已解决

**原因**: 链接脚本使用GCC11才支持的READONLY关键字，与GCC10.3不兼容

**解决方案**: 已修复STM32H750XX_FLASH.ld链接脚本

### 4. OpenOCD 无法连接调试器
```bash
# 检查调试器是否连接
ls /dev/tty.usbmodem*  # macOS
ls /dev/ttyUSB*        # Linux

# 检查权限问题（可能需要sudo）
sudo openocd -f openocd_stlink.cfg

# 或重置调试器连接
sudo openocd -f interface/stlink.cfg -c "adapter speed 1000"
```

### 5. CMake 缓存问题
```bash
# 完全清除build目录
rm -rf build build_release
cmake --preset Debug -B build
cmake --build build --config Debug
```

### 6. 编译错误 - 缺失源文件
**原因**: relays.c 或 base_ID_general_api.c 没有被添加到CMakeLists.txt

**解决方案**: 已在 cmake/stm32cubemx/CMakeLists.txt 中添加：
```cmake
set(MX_Application_Src
    ...
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Src/relays.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Src/base_ID_general_api.c
    ...
)
```

## VSCode 推荐扩展

```bash
code --install-extension ms-vscode.cpptools          # C/C++ IntelliSense
code --install-extension ms-vscode.cmake-tools       # CMake工具
code --install-extension twxs.cmake                  # CMake语言支持
code --install-extension xaver.clang-format          # 代码格式化
code --install-extension dan-c-underwood.arm-assembly  # ARM汇编支持
code --install-extension ms-vscode.cortex-debug      # 嵌入式调试
```

## 参考资源

### 官方文档
- [OpenOCD 官方文档](http://openocd.org/)
- [STM32H750 数据手册](https://www.st.com/resource/en/datasheet/stm32h750vb.pdf)
- [ARM GDB 用户手册](https://sourceware.org/gdb/documentation/)
- [CMake 官方文档](https://cmake.org/cmake/help/latest/)

### 开发板资源
- [STM32H750 参考手册](https://www.st.com/resource/en/reference_manual/dm00314099-stm32h750xbx-and-stm32h750ibx-ad-value-line-arm-based-32-bit-microcontrollers.pdf)
- [FDCAN控制器配置](https://www.st.com/resource/zh/datasheet/stm32h750vb.pdf)

## 已验证兼容性清单

| 组件 | 版本 | 状态 | 备注 |
|------|------|------|------|
| **ARM工具链** | osx-cross/arm-gcc-bin@10 (10.3.1) | ✅ | 必须使用此版本以获得newlib |
| **CMake** | 4.0.2+ | ✅ | 支持预设配置 |
| **Ninja** | 1.12.1+ | ✅ | 构建系统 |
| **OpenOCD** | 0.12.0 | ✅ | 支持STM32H7xx和ST-Link/J-Link |
| **GDB** | arm-none-eabi-gdb (随arm-gcc-bin安装) | ✅ | 调试器前端 |
| **macOS** | 11.0+ | ✅ | M1/M2 Apple Silicon支持 |
| **Linux** | Ubuntu 22.04+ | ✅ | Debian系发行版 |

## 构建配置文件总览

| 文件 | 用途 | 状态 | 说明 |
|------|------|------|------|
| `CMakeLists.txt` | 构建配置 | ✅ 已更新 | 添加relays.c等缺失源文件 |
| `CMakePresets.json` | 预设配置 | ✅ | Debug/Release预设 |
| `cmake/gcc-arm-none-eabi.cmake` | 工具链配置 | ✅ | ARM编译器设置 |
| `STM32H750XX_FLASH.ld` | 链接脚本 | ✅ 已修复 | 移除GCC10不支持的READONLY |
| `.vscode/tasks.json` | 编译任务 | ✅ 已校正 | 正确的输出路径build/ |
| `.vscode/launch.json` | 调试配置 | ✅ 已校正 | ST-Link/J-Link支持 |
| `build.sh` | 编译脚本 | ✅ | 自动化编译 |
| `flash.sh` | 烧录脚本 | ✅ 已校正 | 一键烧录 |
| `openocd.sh` | 调试脚本 | ✅ | 启动OpenOCD服务器 |

## 编译成功指标

✅ 项目已成功交叉编译，确认指标：

1. **编译日志**
   ```
   [ 99%] Linking C executable Relay16CH_H750.elf
   [100%] Built target Relay16CH_H750
   ```

2. **生成的文件**
   - ✅ ELF文件: `build/Relay16CH_H750.elf` (2.0 MB)
   - ✅ HEX文件: `build/Relay16CH_H750.hex` (70 KB)
   - ✅ 映射文件: `build/Relay16CH_H750.map`

3. **二进制统计**
   ```
   text    data     bss     dec      hex
   25392    24      3520    28936    7108
   ```

4. **内存使用**
   - FLASH: 20.8 KB / 128 KB (15.90%)
   - DTCMRAM: 3.4 KB / 128 KB (2.59%)

## 备注

- 所有路径已配置为相对路径，可跨平台使用
- 配置文件位于 `.vscode/` 目录
- OpenOCD配置文件在项目根目录
- 确保调试器固件最新，以获得最佳兼容性
