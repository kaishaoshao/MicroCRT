# MicroCRT Stdlib 模块函数规范与实现清单

`stdlib.h` 是 MicroCRT 的通用工具、动态内存管理与进程控制核心头文件，定义在 [include/libc/stdlib.h](file:///include/libc/stdlib.h)。

---

## 一、底层板级桩函数 (Target Retargeting Stubs)

### 1. `_sys_exit`
- **函数原型**：`void _sys_exit(int returncode);`
- **作用**：终极系统终止出口，`exit()`、`_Exit()`、`abort()` 清理完高层后调用。
- **裸机实现**：进入低功耗待机死循环（`while(1) { __WFI(); }`）或触发芯片软件系统复位。

### 2. `__user_heap_extend`
- **函数原型**：`size_t __user_heap_extend(int ignore, void **base, size_t requestedsize);`
- **作用**：堆内存扩展（类似 POSIX `sbrk`）。当通用内存分配器（`malloc`）发现当前内存池耗尽时，调用该函数向系统申请新的连续内存块。

---

## 二、通用高级函数列表 (Generic Stdlib Functions)

### 1. 动态内存管理 (Dynamic Memory Allocation)
| 函数名称 | 原型声明 | 描述 |
| :--- | :--- | :--- |
| `malloc` | `void *malloc(size_t size);` | 从堆内存池中分配连续 `size` 字节内存 |
| `calloc` | `void *calloc(size_t num, size_t size);` | 分配 `num * size` 字节内存并清零 |
| `realloc` | `void *realloc(void *ptr, size_t new_size);` | 调整已分配内存块大小并迁移数据 |
| `free` | `void free(void *ptr);` | 释放内存块回内存池，合并相邻空闲块 |
| `aligned_alloc`| `void *aligned_alloc(size_t alignment, size_t size);` | 按照指定字节对齐要求分配堆内存 |

> **内存分配器设计规范**：
> - 嵌入式系统对内存碎片极其敏感，MicroCRT 内存分配器采用**双向链表边界标记法（Boundary Tag Allocation）**或轻量级 **TLSF (Two-Level Segregated Fit)** 架构。
> - 时间复杂度：`malloc` 与 `free` 具备良好的实时性边界（最坏情况时间可预测）。

### 2. 字符串到数值转换 (String Conversion)
| 函数名称 | 原型声明 | 描述 |
| :--- | :--- | :--- |
| `atoi` | `int atoi(const char *str);` | 字符串转换为 `int` |
| `atol` | `long atol(const char *str);` | 字符串转换为 `long` |
| `atoll` | `long long atoll(const char *str);` | 字符串转换为 `long long` |
| `atof` | `double atof(const char *str);` | 字符串转换为 `double` |
| `strtol` | `long strtol(const char *str, char **endptr, int base);` | 支持 2~36 进制的带符号长整数转换 |
| `strtoll` | `long long strtoll(const char *str, char **endptr, int base);` | 64 位宽有符号整数转换 |
| `strtoul` | `unsigned long strtoul(const char *str, char **endptr, int base);` | 无符号长整数转换 |
| `strtoull`| `unsigned long long strtoull(const char *str, char **endptr, int base);` | 64 位宽无符号长整数转换 |
| `strtod` | `double strtod(const char *str, char **endptr);` | 转换浮点数字符串（支持科学计数法与 NaN/Inf） |
| `strtof` | `float strtof(const char *str, char **endptr);` | 转换单精度浮点数字符串 |
| `strtold` | `long double strtold(const char *str, char **endptr);` | 转换高精度浮点数字符串 |

### 3. 算法与排序 (Searching and Sorting)
| 函数名称 | 原型声明 | 描述 |
| :--- | :--- | :--- |
| `qsort` | `void qsort(void *base, size_t nitems, size_t size, int (*compar)(const void *, const void *));` | 快速排序算法（包含三数取中分区与小数组插入排序混合优化） |
| `bsearch` | `void *bsearch(const void *key, const void *base, size_t nitems, size_t size, int (*compar)(const void *, const void *));` | 标准二分查找算法 |

### 4. 整数算术操作 (Integer Arithmetic)
| 函数名称 | 原型声明 | 描述 |
| :--- | :--- | :--- |
| `abs` | `int abs(int n);` | 计算 `int` 绝对值 |
| `labs` | `long labs(long n);` | 计算 `long` 绝对值 |
| `llabs` | `long long llabs(long long n);` | 计算 `long long` 绝对值 |
| `div` | `div_t div(int numer, int denom);` | 同时计算商与余数 |
| `ldiv` | `ldiv_t ldiv(long numer, long denom);` | 32/64 位长整数商与余数 |
| `lldiv` | `lldiv_t lldiv(long long numer, long long denom);`| 64 位长整数商与余数 |

### 5. 伪随机数生成 (Pseudo-Random Generation)
| 函数名称 | 原型声明 | 描述 |
| :--- | :--- | :--- |
| `rand` | `int rand(void);` | 生成 `[0, RAND_MAX]` 范围内的伪随机数（线性同余发生器 LCG） |
| `srand` | `void srand(unsigned int seed);` | 设置伪随机数种子序列 |

### 6. 程序与进程控制 (Process Control)
| 函数名称 | 原型声明 | 描述 |
| :--- | :--- | :--- |
| `exit` | `void exit(int status);` | 执行 atexit 注册的清理函数并安全退出 |
| `_Exit` | `void _Exit(int status);` | 不执行 atexit 函数立即退出 |
| `abort` | `void abort(void);` | 异常终止程序（触发 `SIGABRT`） |
| `atexit` | `int atexit(void (*func)(void));` | 注册退出回调函数链表（标准要求至少支持 32 个） |
| `getenv` | `char *getenv(const char *name);` | 获取环境变量（裸机环境下默认返回 NULL） |
| `system` | `int system(const char *command);` | 执行系统命令（裸机环境下默认返回 -1） |
