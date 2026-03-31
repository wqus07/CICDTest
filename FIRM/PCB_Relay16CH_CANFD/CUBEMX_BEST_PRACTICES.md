# CubeMX 与用户代码混合项目最佳实践

## 问题背景

在转换Keil工程到CMake时，遇到的常见问题：
- **现象**: CubeMX生成的CMakeLists.txt不包含 `relays.c`
- **原因**: 这是用户自定义文件，不是CubeMX生成的
- **结果**: 需要手动管理用户文件

---

## 📋 项目结构分析

### CubeMX 管理的文件（自动生成）
```
Src/
├── main.c                      ← CubeMX生成
├── gpio.c                      ← CubeMX生成
├── crc.c                       ← CubeMX生成
├── fdcan.c                     ← CubeMX生成
├── tim.c                       ← CubeMX生成
├── stm32h7xx_it.c             ← CubeMX生成
├── stm32h7xx_hal_msp.c        ← CubeMX生成
└── system_stm32h7xx.c         ← CubeMX生成
```

### 用户自定义文件（需手动维护）
```
Src/
├── relays.c                    ← 用户文件
├── base_ID_general_api.c       ← 用户文件
├── sysmem.c                    ← Newlib标准（可选）
└── syscalls.c                  ← Newlib标准（可选）
```

---

## ✅ 最佳实践方案

### 方案：在根 CMakeLists.txt 中管理用户文件

**为什么这样做？**
1. ✅ CubeMX重新生成代码时不会覆盖用户文件配置
2. ✅ 清晰分离CubeMX管理的代码和用户代码
3. ✅ 易于维护和查看所有用户文件

**实现方式：**

**CMakeLists.txt（根目录）**
```cmake
# ...

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Add user-defined sources (will NOT be overwritten by CubeMX)
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    Src/relays.c
    Src/base_ID_general_api.c
)

# ...
```

**cmake/stm32cubemx/CMakeLists.txt（CubeMX生成）**
```cmake
# STM32CubeMX generated application sources
set(MX_Application_Src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Src/main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Src/gpio.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Src/crc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Src/fdcan.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Src/tim.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Src/stm32h7xx_it.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Src/stm32h7xx_hal_msp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Src/sysmem.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Src/syscalls.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../startup_stm32h750xx.s
)
# 注意：不包含 relays.c 和 base_ID_general_api.c
```

---

## 🔄 CubeMX 工作流程

### 当 CubeMX 重新生成代码时

1. **CubeMX 会重写以下文件**
   - `cmake/stm32cubemx/CMakeLists.txt`
   - `Src/main.c`
   - `Src/gpio.c`
   - 其他CubeMX生成的文件

2. **CubeMX 不会修改**
   - 根目录的 `CMakeLists.txt` ✅
   - 用户自定义的 `Src/relays.c` ✅
   - 用户自定义的 `Src/base_ID_general_api.c` ✅

3. **手动操作**
   - 检查 `cmake/stm32cubemx/CMakeLists.txt` 是否有新增文件需要配置
   - 保持根 `CMakeLists.txt` 中的用户文件列表

---

## 📝 添加新的用户文件时

当需要添加新的用户自定义源文件时：

```cmake
# CMakeLists.txt（根目录）

target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # 用户自定义源文件
    Src/relays.c
    Src/base_ID_general_api.c
    Src/my_new_module.c              # ← 新增用户文件
    Src/another_user_code.c          # ← 新增用户文件
)
```

---

## ⚠️ 常见错误

### ❌ 错误1：在 cmake/stm32cubemx/CMakeLists.txt 中添加用户文件

```cmake
# ❌ 不要这样做
set(MX_Application_Src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Src/main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Src/relays.c    # ❌ 错误
    ...
)
```

**问题**: 下次 CubeMX 重新生成时，手动添加的行会被覆盖。

### ❌ 错误2：在 CubeMX 中不正确配置项目

**问题**: 如果 CubeMX 配置指向错误的输出目录，会导致文件冲突或丢失。

**检查点**:
- CubeMX 的 Target Toolchain 是否设置为 CMake
- CubeMX 的输出路径是否正确
- 是否启用了代码保护（防止 CubeMX 覆盖用户代码）

---

## 🎯 当前项目配置

### 根 CMakeLists.txt
✅ 正确管理用户文件：
```cmake
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    Src/relays.c
    Src/base_ID_general_api.c
)
```

### cmake/stm32cubemx/CMakeLists.txt
✅ 只包含 CubeMX 生成的文件，不包含用户文件

### 编译验证
✅ 成功编译，所有依赖项都能正确解析

---

## 📚 文件管理规则总结

| 来源 | 文件位置 | 管理方式 | 会被CubeMX覆盖 |
|------|---------|---------|----------------|
| **CubeMX生成** | `Src/main.c` 等 | cmake/stm32cubemx/CMakeLists.txt | ✅ 会 |
| **用户自定义** | `Src/relays.c` 等 | 根CMakeLists.txt | ❌ 不会 |
| **第三方库** | `Libs/xxx.c` | 根CMakeLists.txt | ❌ 不会 |
| **中间件** | `Drivers/xxx.c` | 根CMakeLists.txt | 根据配置 |

---

## 🔗 相关文件

- 当前项目根 CMakeLists.txt: 已配置用户文件
- CubeMX 配置: Relay16CH_H750.ioc
- 项目元数据: .mxproject

---

**总结**: 这不是 CubeMX 配置问题，而是项目结构的正常现象。通过在根 CMakeLists.txt 中管理用户文件，可以确保每次 CubeMX 重新生成代码时都能保持用户代码的完整性。
