# Picolibc `printf` 实现分析

## 1. 分析范围

本报告基于：

```
clib/picolibc/doc/printf.md
clib/picolibc/libc/stdio/vfprintf.c
clib/picolibc/libc/stdio/vfiprintf.c
clib/picolibc/libc/stdio/vffprintf.c
clib/picolibc/libc/stdio/snprintf.c
clib/picolibc/libc/stdio/filestrput.c
clib/picolibc/libc/stdio/fputc.c
clib/picolibc/libc/stdio/fflush.c
clib/picolibc/libc/stdio/stdio_private.h
```

需要先说明一个现状：

```
clib/picolibc
```

当前工作树是脏的，尤其是：

```
clib/picolibc/libc/stdio/vfprintf.c
```

文件尾部存在本地附加实现。因此本报告会区分：

1. 上游 Picolibc 的标准设计
2. 当前工作树里可见的本地偏离

---

## 2. 官方设计目标

Picolibc 官方文档 [clib/picolibc/doc/printf.md](/home/shaokai/Desktop/test/fusa/mculib-fusa/clib/picolibc/doc/printf.md:1) 明确把目标定义为：

```
在嵌入式代码尺寸受限场景下
按功能等级提供多种 printf/scanf 实现
```

它不是靠一个巨型 `vfprintf` 然后“尽量被链接器裁剪”，而是显式提供多种变体：

```
d  double/full
f  float-only
l  long long / no float
i  integer-only
m  minimal
```

选择机制通过：

```
--printf=d/f/l/i/m
```

以及链接期：

```
--defsym=vfprintf=__d_vfprintf
--defsym=vfprintf=__i_vfprintf
...
```

来完成。

这说明 Picolibc 的关键思想是：

```
先把能力等级做成独立符号
再用 specs / linker alias 选择默认实现
```

---

## 3. 入口与变体组织方式

`vfiprintf.c` 和 `vffprintf.c` 非常典型：

```c
#define PRINTF_VARIANT __IO_VARIANT_INTEGER
#define PRINTF_NAME    __i_vfprintf
#include "vfprintf.c"
```

```c
#define PRINTF_VARIANT __IO_VARIANT_FLOAT
#define PRINTF_NAME    __f_vfprintf
#include "vfprintf.c"
```

也就是说 Picolibc 的组织方式是：

```
一个 vfprintf.c 模板
+ 多个极薄包装文件
+ 每个包装文件只改 variant 宏
```

与 emRun 的“emProject 多次编译 `prinops.c`”思路本质接近，但 Picolibc 更干净，完全走普通源码模板复用。

---

## 4. `vfprintf.c` 的核心原理

[clib/picolibc/libc/stdio/vfprintf.c](/home/shaokai/Desktop/test/fusa/mculib-fusa/clib/picolibc/libc/stdio/vfprintf.c:1) 不是简单的单一实现文件，而是一个“按变体宏裁剪功能”的模板。

文件开头会根据 `PRINTF_VARIANT` 推导能力：

```
_NEED_IO_LONG_LONG
_NEED_IO_FLOAT
_NEED_IO_DOUBLE
_NEED_IO_LONG_DOUBLE
_NEED_IO_POS_ARGS
_NEED_IO_C99_FORMATS
_NEED_IO_PERCENT_B
_NEED_IO_SHRINK
```

所以同一个解析器模板，在编译期就已经知道：

1. 是否支持浮点
2. 是否支持 `long long`
3. 是否支持位置参数
4. 是否支持 C99 扩展
5. 是否走 minimal shrink 模式

这意味着 Picolibc 的裁剪边界主要发生在编译期，而不是运行时分支。

---

## 5. 解析器与 formatter

`vfprintf.c` 本身承担的是：

1. 扫描格式串
2. 解析 flags / width / precision / length
3. 选择整数、字符串、浮点、指针等路径
4. 调字符输出回调

和 Arm/emRun 相比，Picolibc 没有把 `flags/width/precision/spec` 拆成很多独立 `.o` 文件，而是把它们保留在一个模板里，由编译宏裁掉不需要的能力。

这是一种不同的权衡：

### 优点

1. 代码集中，维护相对直接
2. 变体逻辑统一
3. 通过宏可强力裁剪

### 缺点

1. formatter 边界不如 Arm 明确
2. 后续想局部替换 spec handler 更难
3. 依赖模板宏，阅读门槛更高

---

## 6. 浮点实现

Picolibc 的浮点支持是清楚分层的。

可见文件包括：

```
vfprintf_float.c
dtoa_engine.c
dtoa_ryu.c
ftoa_engine.c
ftoa_ryu.c
ldtoa_engine.c
dtox_engine.c
ldtox_engine.c
```

这表示它至少分开了：

1. `printf` 浮点格式输出逻辑
2. 十进制转换算法
3. 十六进制浮点转换
4. `float/double/long double` 不同宽度路径

官方文档还说明：

```
io-float-exact=true
```

时会启用精确 round-trip 算法。

所以 Picolibc 在浮点这一块比很多小型 libc 更完整，且可通过变体等级完全关掉。

---

## 7. `snprintf` 路径

Picolibc 的一个关键点是：

```
snprintf 仍然复用 vfprintf
```

但不是把真实 `FILE` 传进去，而是构造一个“字符串 FILE”。

在 [clib/picolibc/libc/stdio/snprintf.c](/home/shaokai/Desktop/test/fusa/mculib-fusa/clib/picolibc/libc/stdio/snprintf.c:35)：

```c
struct __file_str f = FDEV_SETUP_STRING_WRITE(...);
i = vfprintf(&f.file, fmt, ap);
```

而 [clib/picolibc/libc/stdio/filestrput.c](/home/shaokai/Desktop/test/fusa/mculib-fusa/clib/picolibc/libc/stdio/filestrput.c:35) 中的 `__file_str_put` 负责把字符写入内存缓冲区，并在溢出时只停止实际写入，但继续累计总长度。

所以 Picolibc 的设计不是：

```
snprintf 单独一套格式化核心
```

而是：

```
统一 vfprintf core
+ 为 memory 构造 fake FILE
+ 通过 FILE->put 回调写入字符串
```

这是它与 emRun 最大的结构差异之一。

---

## 8. FILE-centered 设计

Picolibc 的 `printf` 更强烈地以 `FILE` 为中心。

`vfprintf` 在 [clib/picolibc/libc/stdio/vfprintf.c](/home/shaokai/Desktop/test/fusa/mculib-fusa/clib/picolibc/libc/stdio/vfprintf.c:1194) 中：

1. `__flockfile(stream)`
2. 检查 `__SWR`
3. 调 `print_core((void *)stream, stdio_out_wrapper, ...)`
4. 由 `stdio_out_wrapper` 调 `stream->put`

这里的关键不是 “输出到某个设备”，而是：

```
输出到一个 FILE 对象
```

而这个 FILE 可以是：

1. 普通缓冲流
2. POSIX FD 包装流
3. `fmemopen` 流
4. `funopen`/`fdevopen` 流
5. `snprintf` 的字符串流

因此 Picolibc 的架构非常适合做标准 libc 风格扩展。

---

## 9. `fputc`、`fflush` 与锁

### 9.1 `fputc`

[clib/picolibc/libc/stdio/fputc.c](/home/shaokai/Desktop/test/fusa/mculib-fusa/clib/picolibc/libc/stdio/fputc.c:35) 表明 `putc` 最终调用：

```
stream->put(c, stream)
```

若失败则置 `__SERR`。

### 9.2 `fflush`

[clib/picolibc/libc/stdio/fflush.c](/home/shaokai/Desktop/test/fusa/mculib-fusa/clib/picolibc/libc/stdio/fflush.c:35) 表明：

```
fflush
  -> __flockfile
  -> stream->flush(stream)
  -> 清 unget 状态
  -> __funlockfile
```

### 9.3 锁模型

`stdio_private.h` 中 `__flockfile/__funlockfile` 封装了递归锁。

这说明 Picolibc 的线程安全边界非常清晰：

```
锁属于 FILE 层
格式化 core 不直接承担锁语义
```

这一点和 Arm 的 locked/unlocked FILE 层，以及你当前想要的设计方向一致。

---

## 10. 变体策略与代码尺寸

Picolibc 的真正强项是“产品级的 printf 等级体系”。

官方五个等级：

```
d full double
f float-only
l no float, keep long long
i integer-only
m minimal
```

这比单纯提供一个 `iprintf` 更系统，因为它把“用户如何选能力”也设计进了工具链接口。

从架构角度看，它解决的不是“怎么写一个 printf”，而是：

```
怎么让用户显式控制 printf 的能力和体积
```

这对嵌入式工程非常重要。

---

## 11. 与 emRun / Arm 的对比

### 11.1 相比 Arm

Picolibc 更偏：

```
模板化单核心
+ 编译期能力裁剪
+ 链接期符号别名选择
```

Arm 更偏：

```
多个较细粒度对象
+ formatter/object 层级更清晰
```

### 11.2 相比 emRun

Picolibc 更偏：

```
完整 stdio/FILE 体系
```

emRun 更偏：

```
轻量 printf core + backend hooks
```

特别是在 `snprintf` 这件事上：

1. emRun：string 直写，不经过 FILE
2. Picolibc：构造 string-FILE，再复用 `vfprintf`

---

## 12. 当前工作树中的本地偏离

当前 [clib/picolibc/libc/stdio/vfprintf.c](/home/shaokai/Desktop/test/fusa/mculib-fusa/clib/picolibc/libc/stdio/vfprintf.c:1180) 尾部有一段额外代码：

1. `stdio_out_wrapper`
2. `print_core`
3. 自定义 `string_out_wrapper`
4. 额外实现的 `vsnprintf` / `snprintf`

这不是上游 Picolibc 的标准组织方式。

它把原本：

```
snprintf -> fake FILE -> vfprintf
```

改成了更接近：

```
snprintf -> print_core + string wrapper
```

这种改法方向上更接近 emRun/Arm 的 sink 思路，但它和目录里现有的 `snprintf.c`、`filestrput.c`、`stdio FILE` 模型是并存的，容易造成架构混杂。

如果后续继续沿这个方向改，建议明确做一个选择：

1. 保留 Picolibc 的 fake FILE 复用路线
2. 或者正式抽出独立 `print_core + sink`

不要两条路径同时长期存在。

---

## 13. 对目标方案的启发

Picolibc 最值得借鉴的是三点：

### 13.1 变体定义要产品化

不要只有“有没有 float”这种隐式宏。

要有明确等级：

```
full
float
long long
integer
minimal
```

### 13.2 锁应留在 FILE 层

这点非常成熟，建议直接继承。

### 13.3 `printf` 选择机制要可控

Picolibc 已经证明：

```
链接别名 / symbol remap
```

是一条非常实用的路。

---

## 14. 结论

Picolibc 的 `printf` 架构本质是：

```
一个模板化 vfprintf 核心
+ 多个编译期变体
+ 多个链接期别名入口
+ 强 FILE-centered 输出模型
+ 完整锁与 flush 语义
+ 通过 fake FILE 复用 snprintf/sprintf
```

它不像 emRun 那样把 sink 抽象做得特别显式，也不像 Arm 那样把 formatter 拆成大量独立对象，但它在“工程可用性”和“能力等级管理”上非常成熟。

如果目标是做一个适合 MCU 的模块化 `printf` 子系统，那么：

1. Arm 适合借鉴 formatter/object 分层
2. emRun 适合借鉴 sink 和 FILE-centered retarget
3. Picolibc 适合借鉴变体等级、FILE 锁和工具链选择机制
