# STM32H750 编译、下载、调试快速指南

## ✅ 当前环境（已验证成功编译）

| 项目 | 版本/工具 | 状态 |
|------|---------|------|
| **MCU** | STM32H750VBTx | ✓ |
| **工具链** | osx-cross ARM GCC 10.3 | ✓ |
| **CMake** | 4.0.2+ | ✓ |
| **Ninja** | 1.12.1+ | ✓ |
| **OpenOCD** | 0.12.0 | ✓ |
| **编译状态** | ✓ 成功 | **可烧录** |

## 🚀 快速开始

### 第一步：安装工具链（首次运行）

**macOS（推荐使用osx-cross以获得完整工具链）**
```bash
# 1. 添加osx-cross工具库
brew tap osx-cross/arm

# 2. 安装完整的ARM工具链（包含newlib）
brew install arm-gcc-bin@10

# 3. 安装构建工具
brew install cmake ninja openocd

# 4. 验证安装
arm-none-eabi-gcc --version
```

**Linux (Ubuntu/Debian)**
```bash
sudo apt-get update
sudo apt-get install gcc-arm-none-eabi gdb-arm-none-eabi binutils-arm-none-eabi
sudo apt-get install libnewlib-arm-none-eabi libstdc++-arm-none-eabi-newlib
sudo apt-get install cmake ninja-build openocd
```

### 第二步：编译项目

**选项 A: VSCode 中编译（推荐）**
```
按 Ctrl+Shift+B 或选择 Terminal → Run Build Task → CMake: Build Debug
```

**选项 B: 命令行编译**
```bash
cd /Users/weiqiao/Projects/VSCode/CICDTest/FIRM/PCB_Relay16CH_CANFD
bash build.sh
```

**编译结果**
```
✓ ELF文件: build/Relay16CH_H750.elf (2.0 MB)
✓ HEX文件: build/Relay16CH_H750.hex (70 KB) - 可直接烧录
✓ 大小: text: 25.4 KB, data: 24 B, bss: 3.5 KB
```

### 第三步：连接调试器

- **硬件连接**：将ST-Link或J-Link连接到PC和STM32H750开发板
- **连接方式**：
  - VCC → 3.3V
  - GND → GND  
  - SWDIO → PA13
  - SWCLK → PA14

### 第四步：选择调试方式

---

## 🔄 三种工作流

### 工作流 1️⃣: VSCode一键调试（最简单）

```
1. F5 (或 Debug 菜单中选择 "Debug with ST-Link (OpenOCD)")
2. 自动编译
3. 自动启动OpenOCD
4. 在main()处停止，可开始调试
```

**快捷键**：
- F10: 单步
- F11: 进入函数  
- F5/F6: 继续/暂停
- Ctrl+F2: 停止

---

### 工作流 2️⃣: 命令行编译+调试

**终端1: 启动OpenOCD**
```bash
bash openocd.sh stlink
# 或
bash openocd.sh jlink
```

**终端2: 编译**
```bash
bash build.sh
```

**终端3: 烧录**
```bash
bash flash.sh stlink
# 或
bash flash.sh jlink
```

---

### 工作流 3️⃣: 纯命令行GDB调试

**终端1: OpenOCD服务器**
```bash
openocd -f interface/stlink.cfg -f target/stm32h7x.cfg
```

**终端2: GDB调试**
```bash
arm-none-eabi-gdb build/Debug/Relay16CH_H750.elf

(gdb) target extended-remote localhost:3333
(gdb) monitor reset halt
(gdb) break main
(gdb) continue
(gdb) step
(gdb) print i
(gdb) quit
```

---

## 📋 常用 VSCode 任务

| 任务 | 快捷键 | 说明 |
|------|--------|------|
| **CMake: Build Debug** | Ctrl+Shift+B | 编译Debug版本 |
| **Flash Binary (ST-Link)** | - | 烧录到芯片 |
| **Flash Binary (J-Link)** | - | 烧录到芯片 |
| **Generate Hex file** | - | 生成HEX文件 |
| **Show Binary Size** | - | 查看二进制大小 |

**使用方法**：`Ctrl+Shift+P` → "Tasks: Run Task" → 选择任务

---

## 🐛 调试基础

### VSCode Debug 面板

1. **左侧 Variables**: 查看变量的值
2. **Watch**: 监控特定变量  
3. **Call Stack**: 查看函数调用栈
4. **DEBUG CONSOLE**: 执行GDB命令

### 常用操作

```
断点：点击行号左侧 或 F9
监控变量：右键选择 "Add to Watch"
计算表达式：Debug Console输入 print 命令
改变变量：Debug Console输入 set 命令
```

---

## ❌ 故障排查

### ✅ 问题1: "arm-none-eabi-gcc not found" → 已解决
```bash
# 正确做法：使用osx-cross/arm-gcc-bin@10
brew tap osx-cross/arm
brew install arm-gcc-bin@10

# 验证
which arm-none-eabi-gcc
arm-none-eabi-gcc --version
```

### ✅ 问题2: OpenOCD 找不到调试器 → 已解决
```bash
# 检查USB连接
ls /dev/tty.usbmodem*  # macOS

# 重试连接（可能需要sudo）
sudo openocd -f interface/stlink.cfg
```

### ✅ 问题3: CMake配置失败 → 已解决
```bash
# 清除缓存并重新配置
rm -rf build
cmake --preset Debug -B build
cmake --build build --config Debug
```

### ✅ 问题4: 编译错误 - 缺失头文件 → 已解决
**原因**: macOS Homebrew默认工具链缺少newlib

**解决方案**: 已在README的第一步中指示使用完整工具链

### ⚠️ 构建路径注意
- **编译输出**: `build/Relay16CH_H750.elf` (不是 `build/Debug/`)
- **HEX文件**: `build/Relay16CH_H750.hex`
- **映射文件**: `build/Relay16CH_H750.map`

---

## 📝 文件说明

| 文件 | 用途 |
|------|------|
| `.vscode/tasks.json` | VSCode编译任务配置 |
| `.vscode/launch.json` | VSCode调试配置 |
| `build.sh` | 自动编译脚本 |
| `flash.sh` | 一键烧录脚本 |
| `openocd.sh` | 启动OpenOCD脚本 |
| `openocd_stlink.cfg` | ST-Link配置文件 |
| `openocd_jlink.cfg` | J-Link配置文件 |
| `COMPILE_GUIDE.md` | 详细编译指南 |

---

## ✅ 编译成功验证

**当前项目已成功编译！** 🎉

编译输出统计：
```
text    data     bss     dec      hex
25392    24      3520    28936    7108

✓ ELF: build/Relay16CH_H750.elf (2.0 MB)
✓ HEX: build/Relay16CH_H750.hex (70 KB)

内存使用：
  FLASH: 20.8 KB / 128 KB (15.90%)
  DTCMRAM: 3.4 KB / 128 KB (2.59%)
```

---

## 📚 有用的参考

- [STM32H750 数据手册](https://www.st.com/resource/en/datasheet/stm32h750vb.pdf)
- [OpenOCD 官方文档](http://openocd.org/)
- [ARM GDB 教程](https://sourceware.org/gdb/documentation/)
- [CMake 官方文档](https://cmake.org/)

---

## 💡 提示

1. **保持OpenOCD运行**: 调试时OpenOCD必须一直运行
2. **查看构建日志**: 编译失败时检查Output窗口
3. **使用Release模式**: 最终烧录时可切换到Release以减小二进制大小
4. **定期更新工具**: `brew upgrade arm-gcc-bin openocd` 保持最新

---

**需要帮助？查看 `COMPILE_GUIDE.md` 获取更多详细信息。**

---

## Windows 与 macOS 兼容说明

为保证 Windows 与 macOS 都可直接使用，调试配置采用平台分离策略：

1. **Windows**: `launch.json` 使用 `windows` 专用字段固定 `openocd` 与 `arm-none-eabi-gdb` 的绝对路径，避免 VSCode 找不到可执行文件。
2. **macOS**: 不使用固定路径，仍通过 `PATH` 自动解析 `openocd` 与 `arm-none-eabi-gdb`，保持原使用方式不变。

macOS 自检命令如下（终端执行）：
```bash
which arm-none-eabi-gdb
which openocd
arm-none-eabi-gdb --version
openocd --version
```

若上述命令能输出路径与版本，则 VSCode 调试可直接使用。
