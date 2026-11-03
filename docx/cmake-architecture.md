# MicroCRT CMake 构建系统设计与逻辑说明

## 1. 概述与设计理念

MicroCRT 是一个面向嵌入式系统、低资源微控制器、RTOS 以及特定硬件加速平台的轻量化 C 运行时库（包括 `libc`、`libm`、`libcrt` 等组件）。

传统 C 运行库（如 Newlib、Picolibc 或 Musl）在支持多指令集与厂商定制平台时，往往面临以下痛点：
1. **宏开关过度蔓延**：代码中充斥大量平台条件编译宏（如 `#ifdef __arm__`、`#ifdef __riscv`），架构与业务逻辑紧耦合；
2. **缺乏多层级解耦**：难以清晰表达“指令集架构（ISA）”、“CPU 核心子目标（Subtarget）”以及“芯片厂商自定义扩展（Vendor Extensions）”三者之间的层级关系；
3. **外部厂商接入繁琐**：第三方芯片厂商如果自研了独立指令集或特定加速指令，往往需要强行侵入式修改官方构建脚本。

为了解决上述问题，MicroCRT 借鉴了 **LLVM Target 后端的设计范式**，基于 CMake 构建了一套**分层解耦、支持厂商指令集插拔、具备优先级覆盖机制**的现代化构建系统。

---

## 2. 核心架构层级（Three-Layer Target Model）

MicroCRT 构建系统的核心逻辑围绕三层架构展开：

```text
┌─────────────────────────────────────────────────────────────────┐
│                    Layer 1: Target (顶层指令集)                   │
│   (RISCV, ARM, AArch64, X86, LoongArch, Mips, 或完全独立的自研 ISA)│
└────────────────────────────────┬────────────────────────────────┘
                                 │
                                 ▼
┌─────────────────────────────────────────────────────────────────┐
│                  Layer 2: Subtarget (核心子目标)                 │
│      (如 rv32imc, rv64gc, rv64gcv; cortex-m0, cortex-m4, a7)     │
│   - 绑定特定 CPU 架构参数 (-march, -mcpu, -mabi)                  │
│   - 注入特定核心能力宏 (__riscv_xlen, __ARM_ARCH_7EM__ 等)       │
│   - 挂载核心特化的通用汇编实现                                    │
└────────────────────────────────┬────────────────────────────────┘
                                 │
                                 ▼
┌─────────────────────────────────────────────────────────────────┐
│       Layer 3: Vendor Custom Extensions (别家/厂商自定义指令集)    │
│  (如平头哥玄铁 XTheadV/MemPair, Andes CustomDSP, 自定义专用加速器) │
│   - 按需配置独立的 CMake Cache 开关                              │
│   - 注入厂商专用扩展定义与编译器标志                              │
│   - 最高优先级的专用优化汇编替换 (如利用专用向量指令优化的 memcpy)  │
└─────────────────────────────────────────────────────────────────┘
```

此外，系统支持 **Out-of-Tree（树外外部目标）机制**，第三方芯片厂商的代码仓库可以完全独立于 MicroCRT 源码树，通过单一路径参数即可无缝接入构建。

---

## 3. 源码解析与覆盖优先级机制（Source Override Mechanism）

在高性能 C 库中，相同的标准函数（如 `memcpy`、`memset`、`sin`、`printf` 核心部件）在不同层级下存在不同程度的优化版本。

MicroCRT 在 `cmake/microcrt_config.cmake` 和 `cmake/MicroCRTTarget.cmake` 中设计了**基名去重与优先级覆盖机制**：

### 3.1 覆盖优先级原则
同一符号的实现遵循以下严格优先级：
$$\text{Generic C (通用 C)} < \text{Target ISA (.S/.c, 架构级实现)} < \text{Subtarget (.S/.c, 核心级实现)} < \text{Vendor Extension (.S/.c, 厂商专用指令优化)}$$

1. **汇编优于通用 C**：同名 `.S` 汇编源码加入时，自动替换掉已存在的 `.c` 源码；
2. **高层级优于低层级**：当处于更高层级（Subtarget 或 Vendor Extension）时，调用 `OVERRIDE` 参数将无条件替换底层已注册的同名源码；
3. **保持链接符号唯一**：保证在最终生成的静态库（`libc.a`）中，每个标准 C 库函数仅保留针对当前硬件平台最优的一份目标对象，避免多重定义或死代码残留。

---

## 4. CMake 模块与函数接口说明

MicroCRT 的构建基础设施由两个核心 CMake 文件驱动：
- `cmake/microcrt_config.cmake`：负责静态库源码注册、去重与自动覆盖逻辑；
- `cmake/MicroCRTTarget.cmake`：负责指令集 Target、子目标 Subtarget 与厂商自定义扩展的声明、配置与调度。

### 4.1 目标初始化与管理

#### `microcrt_initialize_targets()`
- **功能**：全局初始化指令集构建环境。
- **逻辑**：
  1. 预载已知架构列表 `MICROCRT_ALL_TARGETS`（默认内置 `AArch64;ARM;RISCV;X86;LoongArch;Mips`）；
  2. 扫描 `MICROCRT_EXTRA_TARGET_DIRS` 注册外部自定义架构；
  3. 解析用户配置 `MICROCRT_TARGETS_TO_BUILD`（可传 `all`、`Native` 或架构列表如 `ARM;RISCV`）；
  4. 规范化名称别名（如将 `armv7` 映射为 `ARM`，`rv64gc` 映射为 `RISCV`）；
  5. 计算并生成当前激活的目标列表 `MICROCRT_TARGETS_BUILT`。

#### `microcrt_register_target(TARGET_NAME [DIR <dir>] [DESCRIPTION <desc>])`
- **功能**：注册一个新的指令集架构。
- **适用场景**：动态扩展全新架构或树外第三方架构。

#### `microcrt_add_target(TARGET_NAME [SOURCES ...] [DEFINITIONS ...] [COMPILE_OPTIONS ...] [INCLUDE_DIRECTORIES ...] [OVERRIDE])`
- **功能**：声明并向 `microcrt_c`（或指定库目标）注入该架构的公共属性。
- **自动行为**：注入 `-D__MICROCRT_TARGET_<TARGET>__` 预编译宏。

---

### 4.2 子目标（Subtarget）管理

#### `microcrt_register_subtarget(TARGET_NAME SUBTARGET_NAME [DESCRIPTION <desc>] [DIR <dir>])`
- **功能**：在某指令集下注册可选的具体核心型号或架构 Profile。
- **示例**：
  ```cmake
  microcrt_register_subtarget(RISCV rv64gc DESCRIPTION "64-bit standard general-purpose profile")
  microcrt_register_subtarget(ARM cortex-m4 DESCRIPTION "ARM Cortex-M4 microcontroller")
  ```

#### `microcrt_add_subtarget(TARGET_NAME SUBTARGET_NAME [COMPILE_OPTIONS ...] [DEFINITIONS ...] [SOURCES ...] [INCLUDE_DIRECTORIES ...])`
- **功能**：声明子目标的具体实现与工具链参数。
- **逻辑**：
  - 自动向 CMake Cache 暴露 `MICROCRT_<TARGET>_SUBTARGET` 选项供用户选择；
  - 当子目标与当前选定目标一致（或为 `all`）时激活；
  - 自动注入 `-D__MICROCRT_SUBTARGET_<TARGET>_<SUBTARGET>__`（自动将非法字符转义为合法 C 标识符）；
  - 挂载专有编译参数（如 `-mcpu=cortex-m4 -mthumb`）与子目标特化的汇编/源码。

---

### 4.3 别家/厂商自定义指令集（Vendor Custom Extensions）

#### `microcrt_register_vendor_extension(TARGET_NAME VENDOR_NAME EXTENSION_NAME [DESCRIPTION <desc>] [DEFAULT <ON/OFF>])`
- **功能**：声明厂商专有自定义指令集扩展。
- **逻辑**：
  - 自动在 CMake Cache 中生成配置开关：`MICROCRT_<TARGET>_VENDOR_<VENDOR>_<EXTENSION>`；
  - 允许在构建配置阶段通过命令行自由开关。

#### `microcrt_add_vendor_extension(TARGET_NAME VENDOR_NAME EXTENSION_NAME [DEFINITIONS ...] [COMPILE_OPTIONS ...] [SOURCES ...] [OVERRIDE])`
- **功能**：注入厂商自定义扩展生效时的配置与实现。
- **逻辑**：
  - 当对应的 Cache 开关为 `ON` 时执行；
  - 注入厂商扩展标识宏：`-D__MICROCRT_<TARGET>_VENDOR_<VENDOR>_<EXTENSION>__`；
  - 加入的汇编实现通过 `OVERRIDE` 享有最高替换权，替换同名通用实现。

---

### 4.4 源码注册与覆盖函数

#### `microcrt_sources([TARGET_LIB] [OVERRIDE] source1 source2 ...)`
- **功能**：向目标库注册源文件。
- **逻辑**：
  - 支持提取公共文件名基名（Base name）；
  - 规则 1：当加入 `.S` 且已存在同名 `.c` 时，自动移除 `.c` 并替换为 `.S`；
  - 规则 2：当显式携带 `OVERRIDE` 时，强行移除底层同名文件并替换为当前高优先级版本。

---

### 4.5 配置摘要横幅

#### `microcrt_print_configuration_summary()`
- 在 CMake 配置阶段输出格式化的构建概览，清晰展示当前编译器、所有支持架构、激活的目标、选中的子目标以及启用的厂商自定义指令集扩展。

---

## 5. 项目目录结构规范

在工程的 `src/target/` 目录下，各架构按统一模式组织：

```text
src/target/
├── CMakeLists.txt              # 遍历并加载已激活的 Target 后端
├── AArch64/
│   ├── CMakeLists.txt          # AArch64 顶层、子目标 (A53/N1) 与 SVE 特性
│   └── README.md
├── ARM/
│   ├── CMakeLists.txt          # ARM 顶层、Cortex-M/A 子目标、NEON/Thumb-2 与厂商扩展
│   └── README.md
├── RISCV/
│   ├── CMakeLists.txt          # RISC-V 顶层、RV32/RV64/Vector 子目标与玄铁/安来扩展
│   └── README.md
├── X86/
│   ├── CMakeLists.txt          # X86 顶层、i386/x86_64/haswell 子目标与 AVX
│   └── README.md
└── custom/                     # 树内第三方/自定义指令集参考模板
    ├── CMakeLists.txt
    └── README.md
```

---

## 6. 使用与构建示例

### 6.1 示例 1：默认原生主机模式（开发与测试）
检测当前主机（如 X86-64 或 AArch64），默认启用原生子目标与本地单元测试：
```bash
cmake -B build -DMICROCRT_BUILD_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

### 6.2 示例 2：针对特定 RISC-V 核心配置（含平头哥玄铁自定义指令集）
配置为 64 位 RISC-V，启用带 Vector 的子目标，并打开玄铁专用指令集扩展：
```bash
cmake -B build-riscv \
  -DMICROCRT_TARGETS_TO_BUILD="RISCV" \
  -DMICROCRT_RISCV_SUBTARGET="rv64gcv" \
  -DMICROCRT_RISCV_VENDOR_THEAD_XTHEADV=ON \
  -DMICROCRT_RISCV_VENDOR_THEAD_XTHEADMEMPAIR=ON
cmake --build build-riscv
```
CMake 配置输出：
```text
-- Configuring Target Backend: RISCV (from .../src/target/RISCV)
--   [RISCV] Configured Active Subtarget: rv64gcv
--   [RISCV] Enabled Custom Instruction Set: Thead::XTheadV
--   [RISCV] Enabled Custom Instruction Set: Thead::XTheadMemPair
-- =======================================================
-- MicroCRT Instruction Set & Target Configuration Summary
-- =======================================================
--   Configured Targets:      RISCV
--   Target RISCV:
--     - Subtarget:           rv64gcv
--     - Custom Extensions:   Thead::XTheadV;Thead::XTheadMemPair
-- =======================================================
```

### 6.3 示例 3：挂载树外第三方/完全自研独立架构（Out-of-Tree）
第三方厂商无需修改 MicroCRT 任何核心源码，通过外部路径直接挂载自研架构：
```bash
cmake -B build-custom \
  -DMICROCRT_TARGETS_TO_BUILD="CustomISA" \
  -DMICROCRT_EXTRA_TARGET_DIRS="CustomISA=/opt/vendor/my_custom_target" \
  -DMICROCRT_CUSTOMISA_SUBTARGET="core-v2" \
  -DMICROCRT_CUSTOMISA_VENDOR_VENDOR_CUSTOMMATRIX=ON
cmake --build build-custom
```

### 6.4 示例 4：构建全部支持架构（全量分发 / 语法检查）
```bash
cmake -B build-all -DMICROCRT_TARGETS_TO_BUILD="all"
cmake --build build-all
```

---

## 7. 开发者指南：如何增加新指令集或厂商扩展

### 7.1 步骤一：创建架构目录
在 `src/target/<NewArch>/` 下创建 `CMakeLists.txt`。

### 7.2 步骤二：编写声明与注册逻辑
参照以下标准模板：
```cmake
# 1. 顶层 Target 声明
microcrt_add_target(NewArch
  INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/include"
  SOURCES src/arch_generic.c
)

# 2. 注册并声明 Subtargets
microcrt_register_subtarget(NewArch core_basic DESCRIPTION "Basic embedded core")
microcrt_register_subtarget(NewArch core_dsp   DESCRIPTION "Core with DSP instructions")

microcrt_add_subtarget(NewArch core_dsp
  DEFINITIONS "__NEWARCH_DSP__"
  COMPILE_OPTIONS "-mcpu=core-dsp"
  SOURCES src/dsp_math.S
)

# 3. 注册并声明别家/厂商自定义指令集
microcrt_register_vendor_extension(NewArch VendorName MyExt
  DESCRIPTION "Vendor proprietary extension"
  DEFAULT OFF
)

microcrt_add_vendor_extension(NewArch VendorName MyExt
  DEFINITIONS "__NEWARCH_VENDOR_MYEXT__"
  SOURCES src/vendor_accel.S
  OVERRIDE
)
```

系统会自动将新架构纳入 `MICROCRT_ALL_TARGETS` 并处理构建选项与依赖。
