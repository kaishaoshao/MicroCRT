# MicroCRT Libc 总体设计与实现规划

MicroCRT 的 `libc` 遵循轻量级、模块化、分层解耦的设计原则，专为嵌入式与裸机环境量身定制。`libc` 架构严格区分**通用标准 C 库函数**与**目标硬件底层适配桩函数**。

---

## 一、双层架构模型 (Two-Tier Architecture)

```
+-------------------------------------------------------------+
|                 应用程序层 (User Applications)               |
+-------------------------------------------------------------+
                              │
                              ▼
+-------------------------------------------------------------+
|               通用 C 库函数层 (Generic Portable Libc)        |
|  - stdio:  printf, sprintf, snprintf, scanf, sscanf, puts   |
|  - stdlib: malloc, free, atoi, strtol, qsort, bsearch       |
|  - string: memcpy, memset, strcpy, strcmp, strlen, strstr   |
|  - ctype:  isdigit, isalpha, tolower, toupper               |
|  - time:   difftime, gmtime, localtime, strftime            |
+-------------------------------------------------------------+
                              │
                              ▼ (仅在需要系统交互时调用)
+-------------------------------------------------------------+
|           目标平台底层桩函数 (Target Retargeting Stubs)       |
|  - I/O 出口:  _sys_write (串口/控制台输出), _sys_read (输入) |
|  - 流生命周期: _sys_open, _sys_close, _sys_istty, _sys_seek |
|  - 内存扩展:  __user_initial_stackheap, __user_heap_extend |
|  - 异常控制:  _sys_exit, __assert_fail, __rt_raise          |
+-------------------------------------------------------------+
                              │
                              ▼
+-------------------------------------------------------------+
|         硬件平台 / 板级支持包 (BSP / Hardware / QEMU)        |
+-------------------------------------------------------------+
```

1. **上层通用函数（Generic Portable Layer）**：
   - 包含绝大部分标准 C 算法与格式化、字符分析逻辑。
   - 100% 纯 C 代码，与目标 CPU 指令集和电路板硬件外设完全解耦。
   - 提供弱符号（Weak Symbol）机制，允许特定架构（如 ARM Cortex-M/NEON 或 RISC-V）通过汇编提供指令级加速（如 `memcpy.S`、`strlen.S`）。

2. **下层硬件适配桩（Target Retargeting Stubs）**：
   - 标准 C 规范只定义行为，无法预知实际硬件使用的是 UART1 还是 UART2，亦或板载内存从 `0x20000000` 开始还是 `0x80000000`。
   - MicroCRT 通过精简的 `_sys_*` 接口打通硬件与标准库，用户或板级驱动仅需实现少数几个核心桩函数，即可点亮完整的 `printf`、`malloc`、`exit` 等高阶能力。

---

## 二、头文件与功能模块映射表

| 头文件路径 | 模块类型 | 核心通用功能 (General) | 底层依赖桩函数 (Target Stubs) |
| :--- | :--- | :--- | :--- |
| `include/libc/stdio.h` | 标准 I/O | 格式化打印 (`printf`、`sprintf`)、流操作、缓冲机制 | `_sys_write`, `_sys_read`, `_sys_open`, `_sys_close`, `_sys_istty` |
| `include/libc/stdlib.h` | 通用工具与内存 | 字符串数值转换 (`strtol`, `atoi`)、排序搜索 (`qsort`, `bsearch`)、内存块管理 (`malloc`, `free`) | `__user_heap_extend`, `__user_initial_stackheap`, `_sys_exit` |
| `include/libc/string.h` | 内存与字符串 | 内存操作 (`memcpy`, `memset`)、字符串处理 (`strcpy`, `strcmp`, `strstr`) | 无强依赖 (架构可选汇编加速) |
| `include/libc/ctype.h` | 字符分类转换 | 字符类别检查 (`isdigit`, `isalpha`)、大小写转换 (`tolower`) | 无依赖 (纯查表/位运算) |
| `include/libc/errno.h` | 错误码定义 | 线程级或全局 `errno`、标准错误号 (`EINVAL`, `ENOMEM`, `ERANGE` 等) | 裸机静态空间 / RTOS TLS |
| `include/libc/assert.h` | 断言机制 | `assert()` 宏 | `__assert_fail` -> `_sys_write` / `_sys_exit` |
| `include/libc/time.h` | 时间与日历 | 日历换算 (`gmtime`, `mktime`, `strftime`) | `clock()`, `time()` 需硬件定时器支持 |
| `include/libc/stddef.h` | 基础标准类型 | `size_t`, `ptrdiff_t`, `NULL`, `offsetof` | 无依赖 (编译器与架构对齐) |
| `include/libc/stdint.h` | 固定宽度整数 | `int8_t` ~ `uint64_t`, `intptr_t`, 极值宏 | 无依赖 |
| `include/libc/stdbool.h` | 布尔类型 | `bool`, `true`, `false` | 无依赖 |

---

## 三、各模块实现与测试推进路线

1. **Phase 1: 基础工具与静态操作 (已完成/完善)**
   - `string.h`: 纯 C 与硬件指令优化实现已完成基本布局。
   - `ctype.h`: 基于 256 字节紧凑位图查表法，实现 O(1) 字符类型识别。
   - 基础基础类型支持: `stddef.h`, `stdint.h`, `stdbool.h`, `errno.h`。

2. **Phase 2: 模块化 Printf 与标准 I/O (stdio 演进)**
   - 格式化输出引擎：解耦整数转换（`_printf_int_dec`, `_printf_int_base`）、文本转换与浮点转换。
   - 输出通道解耦：`sprintf` 写入内存 buffer，`printf`/`fprintf` 接入 `_sys_write`。
   - 流缓冲机制：按需引入无缓冲 (`_IONBF`) 与行缓冲 (`_IOLBF`)。

3. **Phase 3: 内存分配器与进程控制 (stdlib 演进)**
   - 嵌入式轻量级内存池分配器（支持首次适应 First-fit 或 TLSF 算法）。
   - 数值与字符串双向转换算法（`strtol`, `strtoll`, `strtod`）。
   - 快速排序 (`qsort`) 与二分查找 (`bsearch`)。
