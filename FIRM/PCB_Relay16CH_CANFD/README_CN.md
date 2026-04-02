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
| `UDS/uds_config.h` | UDS 可移植配置 |
| `UDS/uds_tp.h/.c` | ISO-TP 传输层 |
| `UDS/uds_flash.h/.c` | Flash 驱动抽象 |
| `UDS/uds_service.h/.c` | UDS 诊断服务 |
| `UDS/uds.h/.c` | UDS 集成入口 |

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

为保证 Windows 与 macOS 都可直接使用，构建与调试配置采用平台分离策略：

| 配置文件 | Windows | macOS / Linux |
|----------|---------|---------------|
| `launch.json` | `windows` 字段固定 `openocd`、`gdb` 绝对路径 | 通过 `PATH` 自动解析 |
| `tasks.json` | `windows` 平台块：`powershell.exe` + WinGet PATH | 系统默认 shell（zsh/bash），工具链来自 Homebrew 或包管理器 |

macOS 自检命令如下（终端执行）：
```bash
which arm-none-eabi-gdb
which openocd
arm-none-eabi-gdb --version
openocd --version
```

若上述命令能输出路径与版本，则 VSCode `Ctrl+Shift+B` 编译和 `F5` 调试均可直接使用。

---

## UDS 固件升级模块 (CAN 总线 OTA)

### 模块概述

`UDS/` 目录包含完整的 ISO 14229 (UDS) + ISO 15765-2 (ISO-TP) 固件升级模块，独立于业务代码，可移植到其他 STM32 项目。

```
UDS/
├── uds_config.h       ← 可移植配置（CAN ID、Flash 布局、时序参数）
├── uds_tp.h / .c      ← ISO 15765-2 传输层（多帧分段/重组）
├── uds_flash.h / .c   ← Flash 驱动（RAM 缓存 → 整体擦写）
├── uds_service.h / .c ← UDS 诊断服务（10/27/34/36/37/31/11/22/3E）
└── uds.h / .c         ← 集成入口 + CAN 硬件适配
```

### CAN 通信参数

| 参数 | 值 | 说明 |
|------|-----|------|
| 物理请求 ID | `0x7E0` | Tester → ECU |
| 物理响应 ID | `0x7E8` | ECU → Tester |
| 功能请求 ID | `0x7DF` | 广播 |
| ID 类型 | Standard 11-bit | UDS 诊断用标准帧 |
| 帧格式 | Classic CAN (8 bytes) | 兼容所有 CAN 工具 |

### 集成步骤（3 步）

**1. 在 `relays.c` 的 `Relays_Start()` 中初始化 UDS（FDCAN 启动之后）：**

```c
#include "uds.h"

void Relays_Start(void) {
    // ... 现有 FDCAN 初始化代码 ...
    HAL_FDCAN_Start(&hfdcan1);
    HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0);

    uds_init();  // ← 添加这一行

    // ... 其余代码 ...
}
```

**2. 在 CAN 接收回调中路由 UDS 帧（`HAL_FDCAN_RxFifo0Callback`）：**

```c
void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef* hfdcan, uint32_t RxFifo0ITs) {
    uint32_t mid;
    FDCAN_RxHeaderTypeDef FDCAN1_RxHeader;
    HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &FDCAN1_RxHeader, &RxCANDataBuffer[0]);
    mid = FDCAN1_RxHeader.Identifier;

    // ── UDS 帧路由（标准帧 0x7E0 / 0x7DF）──
    if (FDCAN1_RxHeader.IdType == FDCAN_STANDARD_ID) {
        if (mid == 0x7E0 || mid == 0x7DF) {
            uint8_t dlc = FDCAN1_RxHeader.DataLength >> 16;  // 转换为字节数
            uds_can_rx(RxCANDataBuffer, dlc);
        }
    }

    // ── 现有的继电器业务逻辑（扩展帧）──
    if (FDCAN1_RxHeader.IdType == FDCAN_EXTENDED_ID) {
        // ... 现有代码不变 ...
    }

    HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0);
}
```

**3. 在 `main.c` 的主循环中处理 UDS 消息：**

```c
#include "uds.h"

while (1) {
    uds_process();  // ← 添加这一行
}
```

### CANoe 刷写配置

**CAPL 测试脚本示例：**

```capl
variables {
    message 0x7E0 uds_req;
}

// 1. 进入编程会话
on key 'p' {
    uds_req.dlc = 8;
    uds_req.byte(0) = 0x02;  // SF: length=2
    uds_req.byte(1) = 0x10;  // DiagSessionControl
    uds_req.byte(2) = 0x02;  // programmingSession
    output(uds_req);
}

// 2. 请求 Seed
on key 's' {
    uds_req.dlc = 8;
    uds_req.byte(0) = 0x02;
    uds_req.byte(1) = 0x27;  // SecurityAccess
    uds_req.byte(2) = 0x01;  // requestSeed
    output(uds_req);
}

// 3. 发送 Key（收到 seed 后计算: key = seed XOR 0x12345678）
on message 0x7E8 {
    if (this.byte(1) == 0x67 && this.byte(2) == 0x01) {
        dword seed = (this.byte(3) << 24) | (this.byte(4) << 16) |
                     (this.byte(5) << 8) | this.byte(6);
        dword key = seed ^ 0x12345678;
        uds_req.byte(0) = 0x06;
        uds_req.byte(1) = 0x27;
        uds_req.byte(2) = 0x02;  // sendKey
        uds_req.byte(3) = (key >> 24) & 0xFF;
        uds_req.byte(4) = (key >> 16) & 0xFF;
        uds_req.byte(5) = (key >> 8) & 0xFF;
        uds_req.byte(6) = key & 0xFF;
        output(uds_req);
    }
}
```

**CANoe 诊断刷写完整流程：**

```
1. DiagSessionControl (0x10 0x02)     → 进入编程会话
2. SecurityAccess     (0x27 0x01/02)  → 安全认证 (seed/key)
3. RoutineControl     (0x31 0x01 FF00)→ 擦除内存准备
4. RequestDownload    (0x34)          → 开始下载 (地址=0x08000000, 大小=固件字节数)
5. TransferData       (0x36) x N      → 逐块传输固件数据 (每块最大 4102 字节, ISO-TP 多帧)
6. TransferExit       (0x37)          → 传输完成
7. RoutineControl     (0x31 0x01 0202)→ CRC 完整性校验
8. ECUReset           (0x11 0x01)     → 硬复位 → 写入 Flash → 启动新固件
```

### Security Access 算法

当前使用简单算法，便于测试：

```
Key = Seed XOR 0x12345678
```

在 CANoe 的 CAPL/ODX/CDD 中配置相同算法即可。生产环境应替换 `uds_service.c` 中的 `verify_key()` 为 AES 或其他安全算法。

### STM32H750 Flash 特殊说明

STM32H750 仅有 **1 个 128 KB Flash 扇区**，无法局部擦除。刷写策略：

```
TransferData → 数据写入 AXI-SRAM 缓冲区 (0x24010000, 128 KB)
                           ↓
ECUReset → 关中断 → 擦除整个 Flash Bank → 从 RAM 写入 Flash → 复位
```

### 移植到其他项目

1. 复制 `UDS/` 目录到新项目
2. 修改 `uds_config.h` 中的平台参数（CAN ID、Flash 地址、RAM 缓冲区地址）
3. 在 `uds.c` 中适配 `uds_tp_can_send()` 为新项目的 CAN 发送函数
4. 按上述 3 步集成到应用

---

## CI/CD 自动化

推送代码或提交 PR 时，GitHub Actions 自动执行固件 CI 流水线（`.github/workflows/firmware-ci.yml`）：

| Job | 触发条件 | 说明 |
|-----|----------|------|
| **Build** | push / PR | Debug + Release 双配置编译，Flash/RAM 占用报告，Release 产物（ELF/HEX/MAP）上传保留 90 天 |
| **Static Analysis** | push / PR | cppcheck 扫描用户源码（relays.c、base_ID_general_api.c），检查 warning / performance / portability |
| **Size Diff** | 仅 PR | 对比 PR 与 base 分支的 text/data/bss 体积变化，结果显示在 PR Summary |

流水线仅在 `FIRM/PCB_Relay16CH_CANFD/` 目录有文件变更时触发，不影响其他部分的 CI。
