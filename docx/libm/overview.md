# MicroCRT Libm 架构设计与实现规划

MicroCRT 的 `libm` 是轻量级、高性能的浮点数学库，专注于嵌入式微控制器、DSP 以及具备向量扩展（如 ARM NEON、RISC-V Vector）的现代处理器。

---

## 一、三层浮点加速体系 (Three-Tier Math Framework)

`libm` 的架构设计充分利用了 MicroCRT 的 LLVM-style Target 分层机制：

```
+-------------------------------------------------------------+
|                 应用程序层 (User Applications)               |
+-------------------------------------------------------------+
                              │
                              ▼
+-------------------------------------------------------------+
|            Layer 1: 通用高精度算法层 (Generic Libm)          |
|  - CORDIC 旋转算法 (微小 ROM 场景)                          |
|  - Remez 最优多项式逼近 / Chebyshev 逼近 (标准速度与精度)     |
|  - 位操作快速归约 (IEEE-754 Bit-Level Range Reduction)      |
+-------------------------------------------------------------+
                              │
               ┌──────────────┴──────────────┐
               ▼ (优先选择硬件优化)           ▼ (当无硬件浮点时)
+-------------------------------+ +---------------------------+
| Layer 2: 目标指令集加速后端    | | Layer 3: 编译器软浮点层   |
| (Target ISA Acceleration)     | | (Compiler-RT / Soft-FP)   |
| - ARM NEON (math_sinf, cosf)  | | - __aeabi_fadd, _fmul     |
| - ARM VFP / Cortex-M4/M7 FPU  | | - __aeabi_dadd, _dmul     |
| - RISC-V Zve32f / RVV / F/D   | | - __floatunsidf           |
| - x86 SSE2 / AVX2 / FMA       | | - 软整数模拟 IEEE 运算    |
+-------------------------------+ +---------------------------+
```

### 1. 通用数学算法层 (Generic Software Math)
- 纯 ANSI C 实现，不依赖任何特定汇编指令。
- **角度与区间归约（Range Reduction）**：利用 Cody-Waite 算法或 Pay-ne-Hanek 高精度算法，将三角函数输入参数无损失地归约到 $[-\pi/4, \pi/4]$ 区间。
- **核心多项式求值（Polynomial Approximation）**：采用 Horner 规则展开 Remez 最优有理逼近式，保证 1 ULP（Unit in the Last Place）以内的高精度。

### 2. 目标架构与硬件加速层 (Target ISA Backend)
- 当开启 `-DMICROCRT_TARGETS_TO_BUILD=ARM` 且子目标支持 NEON 或 VFP 时，CMake 自动通过 `OVERRIDE` 规则将通用实现无缝替换为硬件向量化实现：
  - 例如 `src/libm/machine/arm/neon/math_sinf.c` 利用 NEON 单指令四路单精度浮点并行计算，吞吐量提升 4~8 倍。
  - RISC-V 目标后端在启用 `rv64gcv` 时调用 Vector 扩展指令优化批量向量运算。

### 3. 编译器软浮点运行时层 (Soft-Float Runtime / Compiler-RT)
- 在无硬件 FPU 的极简 Cortex-M0 或 RV32I 核心上，高层浮点加减乘除会被编译器降阶生成对底层软浮点例程的调用（如 `__aeabi_fadd`, `__aeabi_fmul`, `__aeabi_dadd`）。
- MicroCRT 的 `src/libcrt` 协同提供软浮点基础原语，确保 `libm` 在全场景下的可移植性闭环。

---

## 二、精度与函数命名规范

MicroCRT 严格遵循 ISO C99 / IEEE 754 浮点命名与类型规范：

| 精度类型 | 后缀 | 位宽 (bits) | 尾数精度 (bits) | 典型应用与性能考量 |
| :--- | :--- | :--- | :--- | :--- |
| **单精度 (Single)** | `f` (如 `sinf`) | 32 | 24 (~7位有效数字) | **嵌入式与 DSP 首选**。绝大部分 Cortex-M4/M7 及微控制器具备单周期硬件单精度 FPU。 |
| **双精度 (Double)** | 无后缀 (如 `sin`) | 64 | 53 (~16位有效数字) | 高精度科学计算、航姿解算。部分高端 MCU（Cortex-M7/Cortex-A/RISC-V 64GC）支持硬件双精度。 |
| **扩展精度 (Extended)** | `l` (如 `sinl`) | 80 / 128 | 64 / 113 | 复杂分析与高保真模拟。嵌入式裸机下通常与 double 保持相同位宽或使用软浮点。 |

---

## 三、异常与浮点环境支持 (`fenv.h`)

MicroCRT 提供 IEEE 754 异常状态捕获与舍入模式控制：
- **五种标准异常**：除零异常 (`FE_DIVBYZERO`)、非法操作 (`FE_INVALID`)、溢出 (`FE_OVERFLOW`)、下溢 (`FE_UNDERFLOW`)、不精确 (`FE_INEXACT`)。
- **四种舍入模式**：向最接近数舍入 (`FE_TONEAREST`，默认)、向正无穷 (`FE_UPWARD`)、向负无穷 (`FE_DOWNWARD`)、向零截断 (`FE_TOWARDZERO`)。
