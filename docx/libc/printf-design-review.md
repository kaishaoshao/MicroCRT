# printf 设计复盘与后续修改方案

## 1. 文档目的

这份文档不再只回答“现在代码怎么做”，而是集中回答五件事：

1. 当前仓库里的 `printf` 流程到底长什么样
2. 这套方案后续准备怎么发布和落地
3. 具体修改路线应该怎么排
4. Arm 方法和 emRun 方法各自有什么缺点
5. 为什么我们前面多次改方案，以及现在为什么收敛到当前设计

它是对 [printf-plan.md](/home/shaokai/Desktop/test/fusa/mculib-fusa/docs/printf-plan.md:1) 的补充，不替代那份定稿文档。

---

## 2. 当前流程图

### 2.1 默认 `printf` 主链

```text
+-------------------------------+
| public API                    |
| printf / fprintf / snprintf   |
+-------------------------------+
               |
               v
+-------------------------------+
| wrapper                       |
| va_start / stream setup       |
| FILE bridge or memory bridge  |
+-------------------------------+
               |
               v
+-----------------------------------------------+
| __printf_core_default(out, fmt, ap)           |
|                                               |
|   parse loop                                  |
|   + __printf_flags                            |
|   + __printf_wp                               |
|   + __printf_ss                               |
|   + default profile / family toggle gating    |
+-----------------------------------------------+
               |
               v
+-----------------------------------------------+
| dispatch                                       |
|                                               |
|  %c   -> _printf_c / __printf_char            |
|  %s   -> _printf_s / __printf_string          |
|  %d%i -> _printf_d/_printf_i + int_dec        |
|  %u   -> _printf_u + int_dec                  |
|  %xX  -> _printf_x + int_hex                  |
|  %o   -> _printf_o + int_oct                  |
|  %p   -> _printf_p + int_hex                  |
|  %n   -> _printf_n                            |
|  %feg -> _printf_f/_printf_e/_printf_g        |
|  %a   -> _printf_a + fp_hex                   |
+-----------------------------------------------+
               |
               v
+-------------------------------+
| __printf_out                  |
| putc / write / ctx / flags    |
+-------------------------------+
               |
               v
+-------------------------------+
| stream/backend layer          |
| buffered write / fflush       |
| semihost / RTT / newlib / mem |
+-------------------------------+
```

### 2.2 `sprintf` / `snprintf` 路径

```text
sprintf / snprintf
        |
        v
memory-backed FILE bridge
        |
        v
__printf_out_init_file(...)
        |
        v
__printf_core_default(...)
        |
        v
memory buffer update + trailing '\0'
```

结论：

* 当前 `sprintf/snprintf` 不是独立 formatter
* 它们是通过 memory-backed bridge 复用同一个 core
* 这条路径要保留，不建议再拆回“单独一套 string formatter”

### 2.3 `iprintf` / `printf_full` 路径

```text
iprintf / fiprintf / siprintf
        |
        v
__printf_core(..., __printf_caps_int())

printf_full / fprintf_full / snprintf_full
        |
        v
__printf_core(..., __printf_caps_float_long_long())
```

结论：

* 当前已经存在“默认版 / 整数版 / full 版”的入口分层
* 默认版现在是单独的 `__printf_core_default`
* 非默认变体目前仍走 `__printf_core + caps`

---

## 2A. 旧 `src/libc/stdio` 实现的真实结构

这一节只描述刚恢复出来的旧实现事实，不夹带目标设计。

### 2A.1 外层 wrapper 很薄

旧实现的外层入口基本都是薄封装：

* `printf.c`
  `printf -> vfprintf(stdout, ...)`
* `fprintf.c`
  `fprintf -> vfprintf(stream, ...)`
* `sprintf.c`
  `sprintf/vsprintf -> 构造 string-backed FILE -> vfprintf`
* `snprintf.c`
  `snprintf/vsnprintf -> 构造 bounded string-backed FILE -> vfprintf`

这说明两件事：

1. 旧实现已经天然满足“`sprintf/snprintf` 复用同一 core”
2. 旧实现的问题不在 wrapper 层，而在 `vfprintf` 本体过于集中

### 2A.2 `sprintf/snprintf` 不是独立 formatter

旧实现里：

```text
sprintf/snprintf
    -> FDEV_SETUP_STRING_WRITE(...)
    -> 构造 __file_str
    -> vfprintf(&f.file, fmt, ap)
    -> 手动补 '\0'
```

也就是说，字符串输出路径本质上已经是：

```text
memory-backed FILE bridge
    + shared vfprintf core
```

这一点和我们现在文档里的目标设计是一致的，不应该推翻。

### 2A.3 `FILE` / stream / flush 在旧实现里的位置

旧实现里有两个关键文件：

* `iob.c`
  定义 `stdin/stdout/stderr`
* `fileops.c`
  定义：
  * `__file_str_put`
  * `__file_str_put_alloc`
  * `fflush`
  * `putc/fputc/putchar`
  * `fwrite/fputs/puts`

现状特点是：

1. `FILE` 输出和字符串输出已经统一到 `stream->put`
2. `fflush` 已经处于 stream 层，而不是 specifier 层
3. `stdout/stderr` 当前直接对接 `_write`

这也说明两点：

* “core 不应直接依赖设备写出”这个方向是对的
* 旧实现虽然简陋，但 bridge/backend 分层并不是从零开始

### 2A.4 `vfprintf.c` 是一个单大 core

旧实现的关键问题在这里。

`vfprintf.c` 同时承担了：

1. variant 选择
   * 当时是 `PRINTF_VARIANT`
   * 当时是 `PRINTF_NAME`

2. 功能宏展开
   * 当时主要是 `_NEED_IO_LONG_LONG`
   * 当时主要是 `_NEED_IO_FLOAT`
   * 当时主要是 `_NEED_IO_DOUBLE`
   * 当时主要是 `_NEED_IO_WCHAR`
   * 当时主要是 `_NEED_IO_POS_ARGS`

3. parser
   * flags
   * width / precision
   * size-spec
   * positional args

4. 主循环
   * 普通字符扫描
   * `%` conversion 驱动

5. specifier 分发
   * float path
   * char path
   * string path
   * `%n`
   * integer path

6. 输出细节
   * `my_putc`
   * `stream_len`
   * lock/unlock

也就是说，旧 `vfprintf.c` 同时是：

```text
wrapper + parser + dispatch + formatter + output glue
```

这正是它难以演进的根本原因。

### 2A.5 旧实现已经有“分块”，但不是对象边界

旧 `vfprintf.c` 末尾不是直接把所有逻辑都写成一个函数体，而是：

```text
#include "vfprintf_float.c"
#include "vfprintf_char.c"
#include "vfprintf_str.c"
#include "vfprintf_n.c"
#include "vfprintf_int.c"
```

这说明旧实现已经有“按 conversion 类型拆块”的意识。

但这个拆法的本质仍然是：

* 预处理级源码拼接
* 不是对象级边界
* 不是可单独链接裁剪的 `_printf_d/_printf_f/...`

所以它只解决了“源码读起来别太长”，没有真正解决：

* 可配置能力分层
* 可解释对象边界
* 可控 codesize 裁剪

### 2A.6 旧整数路径的真实状态

`vfprintf_int.c` 里已经基本包含了：

* `%d/%i`
* `%u`
* `%o`
* `%x/%X`
* `%p`

并且共享：

* `arg_to_signed`
* `arg_to_unsigned`
* `__ultoa_invert`

这说明旧代码里的整数族本来就是“共享整数格式化骨架”的。

对我们来说，这部分最适合被抽成：

```text
_printf_d / _printf_i / _printf_u
    -> _printf_int_dec

_printf_x
    -> _printf_int_hex

_printf_o
    -> _printf_int_oct

_printf_p
    -> _printf_hex_ptr
```

也就是说，旧整数路径并不是要重写思路，而是要把它从“大 include 块”抽成明确 entry/helper 边界。

### 2A.7 旧字符 / 字符串 / `%n` 路径的真实状态

* `vfprintf_char.c`
  `%c`，并带宽字符分支
* `vfprintf_str.c`
  `%s`，并带宽字符 / 多字节转换分支
* `vfprintf_n.c`
  `%n` 写回当前输出长度

这三块的特点是：

1. 已经是按 specifier 类型拆块
2. 但仍然直接依赖 `vfprintf.c` 里的局部变量和标签
3. 还不是独立 entry 函数

所以后续第一刀不是“重写语义”，而是：

* 先把局部变量依赖显式化
* 再把它们变成真正的 `_printf_c/_printf_s/_printf_n`

### 2A.8 旧浮点路径的真实状态

`vfprintf_float.c` 目前是：

* `%f/%e/%g`
* `%a`
* `double/long double`
* `inf/nan`
* 宽度/精度/对齐/符号处理

都放在一整块里。

它已经体现出两个重要事实：

1. 浮点本来就是一个大成本能力块
2. `%f/%e/%g` 与 `%a` 虽然都在浮点路径里，但共享度并不完全一样

所以后续拆分上，最自然的是：

```text
_printf_f / _printf_e / _printf_g
    -> _printf_fp_dec

_printf_a
    -> _printf_fp_hex
```

### 2A.9 旧构建方式的真实问题

旧 `src/libc/stdio/CMakeLists.txt` 只编：

* `printf.c`
* `fprintf.c`
* `sprintf.c`
* `snprintf.c`
* `vfprintf.c`
* `fileops.c`
* `iob.c`

而像：

* `vfprintf_int.c`
* `vfprintf_float.c`
* `vfprintf_char.c`
* `vfprintf_str.c`
* `vfprintf_n.c`

并不是独立对象，而是被 `vfprintf.c` 直接 `#include` 进去。

这正是旧代码离 Arm 风格最远的一点：

```text
源码拆块存在
对象边界基本不存在
```

### 2A.10 旧实现对我们现在设计的直接启发

旧实现最值得保留的东西有四个：

1. wrapper 层已经足够薄
2. `sprintf/snprintf` 已经复用 shared core
3. `FILE` / string 输出已经有统一的 `put` 抽象
4. conversion 类型已经有初步源码分块

旧实现最需要改变的东西也有四个：

1. `vfprintf.c` 过度集中
2. parser / dispatch / output glue 全缠在一起
3. conversion 分块还是 `#include` 级别，不是对象级
4. variant / capability / public API 语义没有清晰分层

### 2A.11 第一刀应该怎么切

基于旧实现现状，第一刀最合理的切法不是直接大面积重写 specifier 语义，而是：

1. 先把 `vfprintf.c` 的主循环和 parser 驱动抽成单模板 core 骨架
2. 保留旧的 conversion 语义作为临时来源
3. 逐步把 `vfprintf_char/str/n/int/float.c` 从“被 include 的代码块”改成真正的 entry/helper 文件

换句话说，正确顺序应该是：

```text
先收 core 边界
再收 entry/helper 边界
最后再做对象化和 capability 细化
```

而不是一上来就同时改三层。

---

## 3. 当前架构的真实状态

当前仓库不是纯 Arm 形式，也不是纯 emRun 形式，而是一个过渡态：

```text
外层入口组织更接近 emRun/SEGGER 的能力分档
内部文件拆分和 handler 边界更接近 Arm 的对象化方向
输出层和 FILE/flush 边界更接近 picolibc/emRun
```

更准确地说：

* API 层：已经有 `printf / iprintf / printf_full / wprintf`
* core 层：已经有共享 parser 和共享 formatter 路径
* specifier 层：已经拆出 `_printf_d/_printf_i/_printf_x/...`
* backend 层：已经有 `printf_stdio_bridge`、stream backend、`fflush`
* 配置层：已经有 `PROFILE + family toggle`

因此现在不能再把它理解成“还没开始设计”。
现在的问题不是“有没有框架”，而是“这个过渡框架要怎样稳定、怎样继续演进到最终目标”。

### 3.1 截至 2026-05-25 的已落地结构

这一小节只描述已经落到代码里的事实，用来和后面“建议收敛成什么样”区分开。

当前默认主链已经固定为：

```text
printf
  -> vprintf
  -> vfprintf
  -> __printf_vformat_stream
  -> __printf_core_default
```

`fprintf` 也已经统一到：

```text
fprintf
  -> vfprintf
  -> __printf_vformat_stream
  -> __printf_core_default
```

当前 core 模板已经稳定为：

```text
printf_core.inc
```

其中现状是：

* `printf_core.inc`
  已经把 entry/local state、`__printf_out_begin(out)`、parse loop、dispatch、tail、ret/fail/secure error 收在一个模板里
* `_printf_parser.c`
  已经承接：
  `__printf_parse_flag_char`
  `__printf_parse_number_char`
  `__printf_parse_dynamic_field`
  `__printf_parse_precision_marker`
  `__printf_parse_size_modifier`
  `__printf_parse_positional_char`
* `_printf_conversion_dispatch.c`
  统一承接 conversion classifier，核心模板里直接调
  `__printf_dispatch_conversion(...)`

当前装配层也已经比文档前半段描述的旧过渡态再收敛了一步：

```text
printf_core_impl.inc
  -> printf_config.h
  -> parser/text helpers
  -> integer helpers
  -> float helpers
  -> conversion dispatcher
```

这意味着当前代码的真实状态已经是：

* 配置集中到 `printf_config.h`
* parser/text/helper 直接装配一组
* integer/helper/entry 直接装配一组
* float/helper/entry 直接装配一组
* final dispatch 单独一组

同时，宏入口也已经比旧过渡态更集中：

* profile 选择正在以 `PRINTF_CORE_PROFILE` / `PRINTF_CORE_SYMBOL` 为主
* capability 选择正在以 `PRINTF_CAP_*` 为主
* 旧 `PRINTF_VARIANT` / `PRINTF_NAME` / `_NEED_IO_*` 仍有兼容映射，但不应再继续扩散到新代码

也就是说，当前仓库虽然仍然不是对象级独立裁剪结构，但已经不再是“一个 support 文件把所有 `_printf_*` 平铺到底”的最初过渡形态。

结合 `arm-armlib-printf-analysis.md` 的对象层证据，当前代码和 Arm 的真正差异已经可以说得很具体：

1. 我们已经有“多 core 壳 + shared helper”这半边。
2. 我们还没有 Arm 那种“per-specifier 薄 entry object”这半边。
3. 当前代码已经把 integer/float family 从 consolidated dispatcher 继续拆到了 per-specifier entry。

所以如果目标是“模仿 Arm 的对象组织”，下一步的正确方向是：

```text
_printf_d.c
_printf_u.c
_printf_o.c
_printf_x.c
_printf_p.c

_printf_f.c
_printf_e.c
_printf_g.c
_printf_a.c
```

而 shared helper 继续保持：

```text
_printf_char_core.c
_printf_string_core.c
_printf_int_dec.c
_printf_int_base.c
_printf_pointer.c
_printf_fp_dec.c
_printf_fp_hex.c
```

这一步的意义不是“看起来更像 Arm”，而是把对象边界进一步压成：

```text
core shell
  -> specifier entry
      -> shared helper
```

这样一来：

* `%x` 不再跟 `%d/%u/%o/%p` 绑在一个 family-dispatch 对象里
* `%a` 不再跟 `%f/%e/%g` 绑在一个 family-dispatch 对象里
* 后续如果做更细粒度裁剪或链接选择，边界会更接近 Arm 已验证的对象层模式

---

## 4. 发布流程

这里的“发布流程”不是指发版本号，而是指这套 `printf` 方案怎样逐步进入可交付状态。

### 4.1 发布阶段

#### 阶段 A：当前过渡版定型

目标：

* 默认 `printf` 不再默认等于 full
* `iprintf`、`printf_full`、`wprintf` 入口语义固定
* 默认 `printf` 支持通过 `PROFILE + family toggle` 调整
* smoke test 能覆盖默认行为和一个裁剪场景

发布标准：

* `printf-smoke` 通过
* `printf-default-config` 通过
* 文档写清默认支持矩阵

#### 阶段 B：内部对象边界继续做细

目标：

* 继续压实 `_printf_d/_printf_i/_printf_x/_printf_p/_printf_f/...` 的调用边界
* 避免默认 core 对禁用 family 建立不必要引用
* 梳理 long/long long/float/wchar 的对象依赖关系

发布标准：

* 能稳定做 codesize 对比
* 默认 profile 切换能带来可解释的对象引用差异
* 文档补齐“对象边界表”

#### 阶段 C：准备 Arm 风格收敛

目标：

* 让 current object layout 更接近 Arm 目标
* 把 specifier entry 与 shared helper 的边界做实
* 为未来链接器配合预留稳定符号和边界

发布标准：

* `_printf_d.o -> _printf_int_dec`
* `_printf_x.o -> _printf_int_hex`
* `_printf_f.o -> _printf_fp_dec`
* `_printf_a.o -> _printf_fp_hex`

也就是说，不一定现在就拥有 Arm 的链接裁剪能力，但对象边界必须先长成 Arm 喜欢的样子。

#### 阶段 D：链接器增强版

目标：

* 真正把裁剪重点从编译期开关继续下沉到对象级引用
* 做到用户调用集合不同，最终拉入对象集合也不同

这一阶段现在不立刻做，但前面的设计必须为它服务。

### 4.2 每次修改的提交顺序

后续每一轮改动都建议按这个顺序出：

1. 先改文档，明确要收敛哪一层边界
2. 再改配置或对象边界，不同时改太多维度
3. 再补测试，至少覆盖一个正向场景和一个裁剪场景
4. 最后再做 codesize 对比

原因很简单：

* 不先写清边界，后面每次都会重新争论
* 不先补测试，裁剪行为很容易把 `va_list` 或 fallback 语义搞错
* 不先稳定行为，codesize 数据没有解释意义

---

## 5. 具体修改方案

### 5.1 当前建议拍板

当前建议正式拍板成下面这套：

#### 对外能力模型

* `printf/fprintf/sprintf/snprintf/vfprintf/vprintf`
  走默认版
* `iprintf/fiprintf/siprintf/vfiprintf`
  走整数版
* `printf_full/fprintf_full/sprintf_full/snprintf_full/vfprintf_full`
  走 full 版
* `wprintf/fwprintf/swprintf/vfwprintf/vswprintf`
  走 wide 版

#### 默认版配置模型

第一层：

* `MCULIB_PRINTF_DEFAULT_PROFILE`

第二层：

* `MCULIB_PRINTF_DEFAULT_ENABLE_*`

当前不对外暴露第三层“每个 specifier 一个独立开关”。

#### 内部继续演进的模型

* parser 继续集中
* dispatch 继续集中
* specifier entry 继续分文件
* shared helper 继续独立
* backend/fflush 继续留在 stream 层

### 5.2 接下来最值得做的修改

#### 修改 1：补默认支持矩阵文档

需要补一张明确表格，列出：

* 每个 profile 默认支持什么长度
* 每个 family toggle 对哪些 specifier 生效
* 禁用后 fallback 语义是什么

这是为了让配置不再靠猜。

#### 修改 2：补更多裁剪测试

当前只有“禁 `%c`”这类最小验证还不够。
后续建议补：

* 禁 `%s`
* 禁 `%p`
* 禁 `%n`
* 禁 `%f`
* `LONG` profile 禁 `%lld`

这样能尽快把默认版裁剪路径压稳。

#### 修改 3：继续压实对象边界

优先看这些链路：

* `_printf_d -> _printf_int_dec`
* `_printf_i -> _printf_int_dec`
* `_printf_x -> _printf_int_hex`
* `_printf_o -> _printf_int_oct`
* `_printf_p -> _printf_int_hex`
* `_printf_f/_printf_e/_printf_g -> _printf_fp_dec`
* `_printf_a -> _printf_fp_hex`

如果这些边界还夹杂不必要交叉依赖，后面做 Arm 风格收敛会很痛苦。

#### 修改 4：补 codesize 观测点

后续每个 profile 至少要有一组固定对比：

* `INT`
* `LONG`
* `LONG_LONG`
* `FLOAT_LONG`
* `FLOAT_LONG_LONG`

这样讨论裁剪才有客观依据。

---

## 6. Arm 方法的缺点

这里说的不是 Arm 方案“没价值”，而是说它直接照搬到当前仓库会遇到的问题。

### 6.1 过度依赖对象级和链接器级裁剪

Arm 方法最强的点，恰恰也是它当前最难直接照搬的点：

* 它假设对象颗粒度很细
* 它假设链接器和收集规则能很好地配合
* 它假设 conversion entry / helper / core variant 的边界已经被长期打磨过

而我们当前环境还没有把这三件事全部准备好。

直接照搬的后果是：

* 架构会先变复杂
* 但 codesize 未必立刻变小
* 还会把测试和调试难度显著抬高

### 6.2 当前仓库还没有等价的多 core 变体基础

Arm 的 `__printf` 不是“一个 core 全包”，而是本来就有多种 parser/core 变体。

如果现在为了模仿 Arm，硬把我们也拆成大量 core 变体：

* 会重复 parser 逻辑
* 会增加维护成本
* 但不一定先带来收益

### 6.3 Arm 方案对使用者不够直观

Arm 更偏向“对象引用结果导向”，不是“用户配置体验导向”。

这对最终产物很好，但对当前用户讨论“默认 printf 到底支持哪些格式”并不友好。

用户现在更需要的是：

* 明确的入口分档
* 明确的默认配置项
* 明确的测试和 codesize 结果

这恰恰不是 Arm 风格最擅长直接提供的。

---

## 7. emRun 方法的缺点

### 7.1 能力分档清晰，但对象边界不够像 Arm 那么细

emRun/SEGGER 很适合给用户暴露：

* `iprintf`
* `printf`
* `wprintf`
* 以及不同能力档

这点对我们很有价值。

但如果完全停在 emRun 式分档上，缺点是：

* 它更像“预先定义好的几个能力包”
* 而不是“足够细的对象引用边界”

这会让最终向 Arm 风格演进时，仍然要再做一次内部拆分。

### 7.2 对 LLVM 友好的信息粒度还不够细

如果我们的长期目标还包括“配合 LLVM 更好识别和优化 printf 族调用”，那么只做 emRun 式几档入口还不够。

还需要：

* 更稳定的符号边界
* 更清楚的 family/single-specifier 语义
* 更明确的默认支持范围

也就是说，emRun 式入口分档很适合作为外层用户接口，但不应该成为最终内部边界的终点。

### 7.3 容易让默认 `printf` 再次退回“半黑盒”

如果只说：

* `iprintf` 是整数版
* `printf` 是普通版
* `printf_full` 是完整版

但不把默认版的支持矩阵和裁剪项写死，最后默认 `printf` 还是容易重新变成“半黑盒能力包”。

这正是我们这几轮讨论里一直在避免的事。

---

## 8. 我们多次更改方案的原因

前面多次改方案，不是因为方向混乱，而是因为每次讨论都暴露出一个更底层的问题。

### 8.1 第一轮变化：从“改旧 vfprintf”转向“新目录重做”

一开始如果继续在旧 `vfprintf.c` 上缝补，会遇到两个问题：

* parser、formatter、stream、API 全缠在一起
* 很难做变体、很难做裁剪、也很难做测试

所以必须先切到 `src/libc/stdio/printf/` 新目录，把骨架重新立起来。

### 8.2 第二轮变化：从“一个大 core”转向“入口分档 + 内部拆分”

如果只保留一个全包 core，再靠 runtime `caps` 判定能力：

* 语义上能工作
* 但 codesize 未必好
* 也不利于往 Arm 目标走

所以必须承认：

* 外层要有入口分档
* 内层要有对象分层
* 两者不能互相替代

### 8.3 第三轮变化：从“默认 printf 就是 full”转向“默认 printf 可配置”

这是最关键的一次收敛。

如果默认 `printf` 直接等于 full：

* 裁剪讨论就失去意义
* `iprintf` 会变成孤立特例
* 用户无法明确知道默认成本和默认语义

所以必须把默认版单独拿出来，并允许它通过配置调整支持范围。

### 8.4 第四轮变化：从“只按大档位”转向“profile + family toggle”

只按 `INT / LONG / FLOAT_LONG / FULL` 这种大档位分层还不够。

因为用户会自然提出这种需求：

* 默认版不要 `%c`
* 默认版不要 `%p`
* 默认版不要 `%n`
* 默认版只留整数和字符串

这时如果只能重新发明一堆新入口函数，体系会失控。

所以必须增加中间层：

* profile 决定大档位
* family toggle 决定具体裁剪

### 8.5 第五轮变化：从“先追 Arm 效果”转向“先做过渡态可交付”

后来逐渐明确一点：

* 最终目标可以是 Arm 风格
* 但当前阶段不能为了模仿最终形态，把过渡态做成不可维护

所以当前正式思路才收敛为：

```text
对外先给用户一个可配置、可测试、可解释的默认版
对内继续往 Arm 风格对象边界推进
等链接器条件成熟后，再把裁剪继续下沉
```

这不是妥协，而是正确的工程顺序。

---

## 9. 当前最终建议

当前建议正式确定为：

1. 默认 `printf` 继续保留为独立可配置变体，不退回 full
2. 对外配置模型固定为 `PROFILE + FAMILY TOGGLE`
3. `iprintf`、`printf_full`、`wprintf` 继续保留为显式入口
4. `sprintf/snprintf` 继续通过 memory-backed bridge 复用同一个 core
5. `fflush`、buffering、backend 行为继续留在 stream 层
6. 内部继续朝 Arm 风格 specifier object split 演进
7. 在链接器能力到位之前，不强行把整个方案伪装成 Arm 成品

一句话总结：

```text
当前最合理的方案不是“照搬 Arm”，也不是“停在 emRun”。
而是“外层采用 emRun 式可配置分档，内层持续向 Arm 式对象边界收敛”。
```

---

## 10. 基于最新讨论的修正方案

上面那套结论还需要再修正一层。

你现在提出的目标是：

```text
源码层只维护一份 printf_core
编译层生成多份 printf_core_xxx
再通过链接阶段的重定位/选择方式，拼出不同能力版本
```

这个方向我接受，而且我认为它比“手写多份完全独立 core 源码”更合理。

### 10.1 修正后的核心原则

现在建议把 core 层设计改成：

```text
一份 core 模板源码
    ↓
多份编译产物
    ↓
不同 public variant 绑定不同 core 产物
    ↓
后续如果链接器能力足够，再进一步下沉到对象级组合
```

也就是说，后续不应该继续维护：

* 一份 `__printf_core_default.c`
* 一份 `__printf_core_integer.c`
* 一份 `__printf_core_full.c`

这种“逻辑重复、只是在文件级分叉”的模式。

而应该维护：

* 一份 `printf_core_body.inc` 或类似模板
* 多个极薄的编译壳

例如：

* `printf_core_base.c`
* `printf_core_flags.c`
* `printf_core_ss.c`
* `printf_core_wp.c`
* `printf_core_flags_ss_wp.c`

这些壳文件本身不承载业务逻辑，只负责：

* 定义编译宏
* 定义导出符号名
* include 同一份 core 模板

这才是“源码一份，产物多份”的正确做法。

### 10.2 为什么这个方案比前一版更好

因为它同时满足三件事：

1. 源码维护成本不会因为多 core 变体而爆炸
2. 对象层仍然可以长成 Arm 喜欢的形状
3. 后续如果要做 emRun 式或链接器式组合，不需要再推翻 core 设计

换句话说，真正应该统一的是：

* 源码逻辑

而不是强行要求：

* 最终只有一个编译出来的 core 函数

### 10.3 修正后的推荐流程图

```text
                +---------------------------+
                | single core template      |
                | printf_core_body.inc      |
                +---------------------------+
                             |
         +-------------------+-------------------+
         |                   |                   |
         v                   v                   v
+----------------+  +----------------+  +----------------------+
| core shell A   |  | core shell B   |  | core shell C         |
| flags only     |  | ss/wp only     |  | flags+ss+wp          |
| exports:       |  | exports:       |  | exports:             |
| __printf$flags |  | __printf$...   |  | __printf$flags$...   |
+----------------+  +----------------+  +----------------------+
         |                   |                   |
         +-------------------+-------------------+
                             |
                             v
                +---------------------------+
                | specifier entry objects   |
                | _printf_d/_printf_i/...   |
                +---------------------------+
                             |
                             v
                +---------------------------+
                | shared helpers            |
                | int_dec / int_hex / fp... |
                +---------------------------+
```

这张图的关键点是：

* 只有一份 core 逻辑源
* 但不是只有一个 core 产物
* 产物层仍然保留多符号、多变体

---

## 11. 只看 Arm 对象和汇编，证据是什么

这里专门回答一个问题：

```text
Arm 的 C 库到底是不是“只有一个 core”？
不要看链接器，只看对象和汇编。
```

结论必须分成两层说：

### 11.1 如果说“源码逻辑可以来自一个模板”，这是可能的

只看对象层，无法证明 Arm 内部源码是不是单模板生成。
它完全可能是：

* 一份模板源码
* 编译出多份不同符号的 core 变体

这和你现在想做的方式是一致的。

### 11.2 如果说“最终对象层只有一个 core 函数”，证据不支持

这点我给出直接证据，而且不依赖链接器。

在 `/home/shaokai/ArmCompilerforEmbedded6.24/lib/armlib/c_2.l` 里，直接有这些对象：

* `__printf.o`
* `__printf_flags.o`
* `__printf_ss.o`
* `__printf_wp.o`
* `__printf_flags_ss.o`
* `__printf_flags_wp.o`
* `__printf_ss_wp.o`
* `__printf_flags_ss_wp.o`

直接看符号表：

```text
__printf.o                  -> __printf                         size 104
__printf_flags.o            -> __printf$flags                   size 144
__printf_ss.o               -> __printf$sizespec               size 184
__printf_wp.o               -> __printf$widthprec              size 270
__printf_flags_ss.o         -> __printf$flags$sizespec         size 232
__printf_flags_wp.o         -> __printf$flags$widthprec        size 308
__printf_ss_wp.o            -> __printf$sizespec$widthprec     size 352
__printf_flags_ss_wp.o      -> __printf$flags$sizespec$widthprec size 388
```

这组证据说明三件事：

1. 它不是只有一个导出 core 符号
2. 这些函数大小不同，不是同一个符号的简单别名
3. 它们分别放在不同对象里，说明对象层确实存在多 core 变体

所以如果把“Arm 是一个 core”理解成：

```text
最终对象层只有一个 __printf
```

这个说法不成立。

### 11.3 只看 specifier entry，对象层也不是“一个大函数”

再看这些对象：

* `_printf_c.o`
* `_printf_d.o`
* `_printf_i.o`
* `_printf_x.o`
* `_printf_p.o`
* `_printf_f.o`
* `_printf_a.o`

直接看符号依赖：

```text
_printf_c.o -> weak UND _printf_char
_printf_d.o -> weak UND _printf_int_dec
_printf_i.o -> weak UND _printf_int_dec
_printf_x.o -> weak UND _printf_int_hex
_printf_p.o -> weak UND _printf_hex_ptr
_printf_f.o -> weak UND _printf_fp_dec
_printf_a.o -> weak UND _printf_fp_hex
```

这说明：

* Arm 的对象层不是“一个大 core + 一个大 switch 就完了”
* 它还有很薄的 specifier entry object
* 这些 entry object 再跳到 shared helper

因此更准确的判断应该是：

```text
Arm 在对象层是“多 core 变体 + 薄 specifier entry + shared helper”
```

而不是：

```text
Arm 在对象层只有一个 core
```

### 11.4 当前最合理的统一说法

如果把你的目标翻译成一句准确的话，应该是：

```text
源码层尽量只有一份 core 逻辑
对象层允许并且应该生成多份 core 变体
```

这句话我现在认为是对的。

也是我建议我们下一步真正采用的方案。

---

## 12. 现在正式采用的目标设计

这一节不是再讨论路线，而是把后续实现要收敛成什么样子直接定下来。

### 12.1 总体目标

目标分三层：

1. 源码层
   `printf` 的 parser/core 逻辑只维护一份。

2. 编译产物层
   由这份 core 模板生成多份 `__printf*` 变体符号，而不是手写多份独立 core。

3. 对外接口层
   继续保留 `printf / iprintf / printf_full / wprintf` 这种明确分层的入口。

压缩成一句话：

```text
单份 core 逻辑
+ 多份 core 产物
+ 明确分层的 public API
+ 逐步向 Arm 风格对象边界收敛
```

### 12.2 明确不做的事

为了避免后面再次走偏，这里把不做的事也明确写下：

1. 不回到“旧 `vfprintf.c` 上继续缝补”
2. 不长期维护多份逻辑重复的 `__printf_core_xxx.c`
3. 不把默认 `printf` 重新退回 full
4. 不把 `iprintf` 退化成普通 `printf` 的简单 alias
5. 不为了模仿 Arm 表面形式，提前引入一堆当前解释不清的链接魔法

---

## 13. 文件级设计

这一节给出我们后续建议的文件骨架。

### 13.0 总体目录树

建议后续逐步收敛成下面这种目录结构：

```text
src/libc/stdio/printf/
  core/
    printf_core_body.inc
    printf_core_parser.inc
    printf_core_dispatch.inc

  parser/
    __printf_flags.c
    __printf_wp.c
    __printf_ss.c

  shells/
    __printf.c
    __printf_flags.c
    __printf_ss.c
    __printf_wp.c
    __printf_flags_ss.c
    __printf_flags_wp.c
    __printf_ss_wp.c
    __printf_flags_ss_wp.c

  entry/
    _printf_c.c
    _printf_s.c
    _printf_d.c
    _printf_i.c
    _printf_u.c
    _printf_x.c
    _printf_o.c
    _printf_p.c
    _printf_n.c
    _printf_percent.c
    _printf_f.c
    _printf_e.c
    _printf_g.c
    _printf_a.c

  helper/
    _printf_char_core.c
    _printf_string_core.c
    _printf_text_core.c
    _printf_int_dec.c
    _printf_int_hex.c
    _printf_int_oct.c
    _printf_hex_ptr.c
    _printf_fp_dec.c
    _printf_fp_hex.c

  out/
    __printf_out.c
    __printf_emit_unsupported.c

  bridge/
    __printf_stream.c
    __printf_mem.c
    __printf_wstream.c

  backend/
    __printf_flush.c
    __printf_backend_stdio.c
    __printf_backend_semihost.c
    __printf_backend_rtt.c

  api/
    printf.c
    fprintf.c
    vprintf.c
    vfprintf.c
    sprintf.c
    snprintf.c
    vsprintf.c
    vsnprintf.c
    iprintf.c
    fiprintf.c
    siprintf.c
    vfiprintf.c
    printf_full.c
    fprintf_full.c
    vfprintf_full.c
    sprintf_full.c
    snprintf_full.c
    wprintf.c
    fwprintf.c
    swprintf.c
    vfwprintf.c
    vswprintf.c

  include/
    printf_config.h
    printf_core.h
    printf_parse.h
    printf_caps.h
    printf_out.h
    printf_backend.h
```

这个目录树不是要求一步到位重命名完成，而是作为目标结构图使用。
当前仓库里文件名可以暂时不完全一致，但职责边界建议按这张图收敛。

### 13.0.1 分层关系图

```text
api
  -> bridge
  -> core shell
  -> caps/config

core shell
  -> core template
  -> parser
  -> entry

entry
  -> helper
  -> out

bridge
  -> out
  -> backend

backend
  -> device / FILE / memory policy
```

这张关系图的重点是：

* `api` 不直接依赖 helper
* `helper` 不反向依赖 `api`
* `backend` 不回流污染 parser/core
* `entry` 和 `helper` 之间是单向格式化依赖

### 13.1 core 模板层

建议收敛成下面这种组织：

```text
src/libc/stdio/printf/
  core/
    printf_core_body.inc
    printf_core_parser.inc
    printf_core_dispatch.inc
```

这三层职责建议固定为：

* `printf_core_body.inc`
  负责主循环、状态推进、unsupported/fallback 收口
* `printf_core_parser.inc`
  负责 flags / width / precision / size-spec 的解析
* `printf_core_dispatch.inc`
  负责把 specifier 分发到 `_printf_*` entry

如果实现时觉得不需要拆成三个 `*.inc`，也可以先只有一个 `printf_core_body.inc`，但职责边界仍然按上面定义。

### 13.1.1 core 模板文件职责表

| 文件 | 职责 | 不应承载的内容 |
| --- | --- | --- |
| `printf_core_body.inc` | core 主循环、普通字符输出、`%` 序列驱动、fallback 收口 | 具体整数/浮点算法、设备 flush 策略 |
| `printf_core_parser.inc` | 驱动 flags/wp/ss 解析步骤，组织 parse descriptor | specifier 真实格式化 |
| `printf_core_dispatch.inc` | 根据 specifier 和 capability 分发到 `_printf_*` | backend 写出细节 |

### 13.1.2 core 模板内部调用关系

这三个文件不是三套不同实现，而是同一份 core 模板内部的三个逻辑层。

调用关系建议固定成：

```text
core shell (__printf*.c)
    |
    v
printf_core_body.inc
    |
    +--> printf_core_parser.inc
    |       |
    |       +--> __printf_flags
    |       +--> __printf_wp
    |       +--> __printf_ss
    |
    +--> printf_core_dispatch.inc
            |
            +--> _printf_c / _printf_s / _printf_d / ...
```

可以把它们理解成：

* `printf_core_body.inc`
  总控层
* `printf_core_parser.inc`
  `%...` 解析层
* `printf_core_dispatch.inc`
  specifier 分发层

### 13.1.3 每个模板文件的输入输出

为了后面实现时不混乱，建议把输入输出边界也写死。

#### `printf_core_body.inc`

输入：

* 输出对象 `out`
* 格式串 `fmt`
* `va_list ap`
* 当前 core 壳启用的 parser/capability 宏

输出：

* 最终返回值
* 输出计数更新
* 错误态更新

它应该负责：

* 扫描普通字符
* 在 `%` 处创建 parse descriptor
* 调 parser
* 调 dispatch
* 处理 disabled/unsupported fallback

#### `printf_core_parser.inc`

输入：

* 当前格式串位置
* parse descriptor
* `va_list ap`
* 当前 core 壳启用的 parser 能力宏

输出：

* 解析后的 descriptor
* 新的格式串位置

它应该负责：

* 解析 flags
* 解析 width / precision
* 解析 size-spec
* 识别 conversion specifier

它不应该负责：

* 真实输出
* 真实数值转换
* backend flush

#### `printf_core_dispatch.inc`

输入：

* 输出对象 `out`
* parse descriptor
* `va_list ap`
* 当前 capability/profile/family toggle

输出：

* 调用对应 `_printf_*` 后的结果
* 或 disabled/unsupported 路径结果

它应该负责：

* 判断 specifier 是否启用
* 决定调用哪个 `_printf_*`
* 决定 fallback 还是正常 entry

它不应该负责：

* 重新解析 `%...`
* 自己做整数/浮点格式化算法

### 13.1.4 一个简化伪代码例子

如果以后真的拆成三层，它们之间的关系大概是这样：

```c
/* __printf_flags_ss_wp.c */
#define __PRINTF_CORE_SYMBOL __printf$flags$sizespec$widthprec
#define __PRINTF_ENABLE_FLAGS 1
#define __PRINTF_ENABLE_SIZESPEC 1
#define __PRINTF_ENABLE_WIDTHPREC 1
#include "core/printf_core_body.inc"
```

```c
/* printf_core_body.inc */
int __PRINTF_CORE_SYMBOL(__printf_out_t *out, const char *fmt, va_list ap) {
  while (*fmt) {
    if (*fmt != '%') {
      __printf_out_char(out, *fmt++);
      continue;
    }

    __printf_desc_t desc;
    fmt = __printf_parse_conversion(fmt + 1, &desc, &ap);
    __printf_dispatch_conversion(out, &desc, &ap);
  }
  return out->count;
}
```

```c
/* printf_core_parser.inc */
const char *__printf_parse_conversion(const char *fmt, __printf_desc_t *desc, va_list *ap) {
  fmt = __printf_flags(fmt, desc);
  fmt = __printf_wp(fmt, desc, ap);
  fmt = __printf_ss(fmt, desc);
  desc->spec = *fmt++;
  return fmt;
}
```

```c
/* printf_core_dispatch.inc */
int __printf_dispatch_conversion(__printf_out_t *out, const __printf_desc_t *desc, va_list *ap) {
  switch (desc->spec) {
    case 'd': return _printf_d(out, desc, ap);
    case 'x': return _printf_x(out, desc, ap);
    case 's': return _printf_s(out, desc, ap);
    default:  return __printf_emit_unsupported(out, desc);
  }
}
```

这个伪代码的重点不是具体函数名，而是职责边界。

### 13.1.5 为什么要保留这三个逻辑层

这样分层主要是为了四件事：

1. 保证 core 源码仍然只有一份
2. 让多个 `__printf*` 壳文件只负责“开哪些 parser 能力”
3. 让后续对象分析时能分清 parser 成本和 specifier 成本
4. 避免 `printf_core_body.inc` 继续膨胀成不可维护的大文件

### 13.2 core shell 层

建议用极薄壳文件来生成多个 core 变体，例如：

```text
src/libc/stdio/printf/
  __printf.c
  __printf_flags.c
  __printf_ss.c
  __printf_wp.c
  __printf_flags_ss.c
  __printf_flags_wp.c
  __printf_ss_wp.c
  __printf_flags_ss_wp.c
```

这些壳文件只做三件事：

1. 定义导出符号名
2. 定义本壳启用的 parser 能力宏
3. `#include "core/printf_core_body.inc"`

也就是说，壳文件本身不承载业务逻辑。

### 13.2.1 core shell 职责表

| 壳文件 | 导出符号 | 启用能力 |
| --- | --- | --- |
| `__printf.c` | `__printf` | base |
| `__printf_flags.c` | `__printf$flags` | flags |
| `__printf_ss.c` | `__printf$sizespec` | sizespec |
| `__printf_wp.c` | `__printf$widthprec` | width/precision |
| `__printf_flags_ss.c` | `__printf$flags$sizespec` | flags + sizespec |
| `__printf_flags_wp.c` | `__printf$flags$widthprec` | flags + width/precision |
| `__printf_ss_wp.c` | `__printf$sizespec$widthprec` | sizespec + width/precision |
| `__printf_flags_ss_wp.c` | `__printf$flags$sizespec$widthprec` | flags + sizespec + width/precision |

base 的含义是：

* 能处理最基本 `%<specifier>` 路径
* 不必天然支持所有 parser 子能力
* 通过壳文件宏控制 parser 能力编译入哪一份 core 产物

### 13.3 specifier entry 层

这一层继续保持独立文件：

```text
_printf_c.c
_printf_s.c
_printf_d.c
_printf_i.c
_printf_u.c
_printf_x.c
_printf_o.c
_printf_p.c
_printf_n.c
_printf_percent.c
_printf_f.c
_printf_e.c
_printf_g.c
_printf_a.c
```

这一层的职责要刻意做薄：

* 从 `va_list` 取参
* 处理本 specifier 的少量语义分叉
* 跳到 shared helper

不应该把大段 parser 或 output 逻辑塞回这里。

### 13.3.1 entry 文件职责图

```text
_printf_d.c
  -> 读取 signed 参数
  -> 根据 size-spec 做取参
  -> 调 _printf_int_dec

_printf_x.c
  -> 读取 unsigned 参数
  -> 处理大小写标志
  -> 调 _printf_int_hex

_printf_f.c
  -> 读取 float/double/long double 参数
  -> 处理固定小数语义分叉
  -> 调 _printf_fp_dec
```

也就是说，entry 是“薄跳板 + 少量本 specifier 语义”，不是第二个 core。

### 13.4 shared helper 层

建议继续保持这种分组：

```text
_printf_int_dec.c
_printf_int_hex.c
_printf_int_oct.c
_printf_hex_ptr.c
_printf_fp_dec.c
_printf_fp_hex.c
_printf_string_core.c
_printf_char_core.c
```

原则是：

* 被多个 entry 复用的格式化算法进入 helper
* 只被一个 entry 使用且很小的逻辑，可以继续留在 entry 文件里

### 13.4.1 helper 职责表

| helper | 主要职责 | 被谁复用 |
| --- | --- | --- |
| `_printf_char_core` | 字符宽度、对齐、填充 | `%c`、`%lc` |
| `_printf_string_core` | 字符串截断、宽度、对齐 | `%s`、`%ls` |
| `_printf_int_dec` | 十进制整数格式化 | `%d/%i/%u` |
| `_printf_int_hex` | 十六进制整数格式化 | `%x/%X` |
| `_printf_int_oct` | 八进制整数格式化 | `%o` |
| `_printf_hex_ptr` | 指针固定前缀与宽度语义 | `%p` |
| `_printf_fp_dec` | `%f/%e/%g` 的共享十进制浮点路径 | `%f/%e/%g` |
| `_printf_fp_hex` | `%a/%A` 的十六进制浮点路径 | `%a/%A` |

### 13.5 output/backend 层

建议明确单独维持：

```text
__printf_out.c
__printf_stream.c
__printf_mem.c
__printf_flush.c
__printf_backend_stdio.c
__printf_backend_semihost.c
__printf_backend_rtt.c
```

名字不一定必须完全如此，但职责边界要这样分。

### 13.5.1 output/backend 文件职责图

```text
__printf_out.c
  -> 统一 putc / write-span / count / error 更新

__printf_stream.c
  -> 把 printf 输出映射到 FILE/stream bridge

__printf_mem.c
  -> 把 printf 输出映射到内存缓冲区

__printf_flush.c
  -> 处理 stream flush 时机与策略

__printf_backend_*.c
  -> 真正对接 semihost / RTT / stdio / 设备写接口
```

这里要特别强调：

* `__printf_flush.c` 只属于 stream/backend 路径
* 不应被 core parser 或 `_printf_d/_printf_s` 直接依赖

---

## 14. 符号级设计

### 14.1 core 变体符号

建议目标符号集合与 Arm 靠拢：

```text
__printf
__printf$flags
__printf$sizespec
__printf$widthprec
__printf$flags$sizespec
__printf$flags$widthprec
__printf$sizespec$widthprec
__printf$flags$sizespec$widthprec
```

这样做的价值有三点：

1. 语义直观
2. 容易和 Arm 调研结果对齐
3. 后续如果要做链接器选择，符号边界已经稳定

### 14.2 public API 到内部实现的绑定

当前建议固定为：

| public API | 默认绑定 |
| --- | --- |
| `printf/fprintf/vprintf/vfprintf` | 默认版 core |
| `sprintf/snprintf/vsprintf/vsnprintf` | 默认版 core + memory backend |
| `iprintf/fiprintf/vfiprintf/siprintf` | 整数版能力集合 |
| `printf_full/fprintf_full/vfprintf_full` | full 能力集合 |
| `wprintf/fwprintf/vfwprintf/swprintf/vswprintf` | wide 能力集合 |

这里有一个关键点：

* `iprintf`、`printf_full` 是“能力入口”概念
* `__printf$flags`、`__printf$widthprec` 是“core 解析能力对象”概念

这两层不要混淆。

### 14.3 specifier entry 目标边界

建议明确收敛到下面这种依赖图：

```text
_printf_d  -> _printf_int_dec
_printf_i  -> _printf_int_dec
_printf_u  -> _printf_int_dec
_printf_x  -> _printf_int_hex
_printf_o  -> _printf_int_oct
_printf_p  -> _printf_hex_ptr
_printf_f  -> _printf_fp_dec
_printf_e  -> _printf_fp_dec
_printf_g  -> _printf_fp_dec
_printf_a  -> _printf_fp_hex
_printf_c  -> _printf_char_core
_printf_s  -> _printf_string_core
```

`%n` 和 `%%` 可以保留为基本独立的小对象。

---

## 15. 配置模型设计

### 15.1 对外配置层次

正式定为三层：

1. public API 分层
   `printf / iprintf / printf_full / wprintf`

2. 默认 `printf` profile
   `MCULIB_PRINTF_DEFAULT_PROFILE`

3. 默认 `printf` family toggle
   `MCULIB_PRINTF_DEFAULT_ENABLE_*`

当前先不对外暴露第四层“单 specifier 逐个开关”。

### 15.2 profile 的职责

`PROFILE` 只负责决定这几类大能力：

* 默认 integer 能力
* 是否支持 `long`
* 是否支持 `long long`
* 是否支持 float
* 是否支持 long double

它不负责表达：

* `%c` 要不要
* `%p` 要不要
* `%n` 要不要

这些属于 family toggle。

### 15.3 family toggle 的职责

family toggle 负责裁剪默认 `printf` 的具体格式族，例如：

* `%c`
* `%s`
* `%d/%i`
* `%u`
* `%x/%X`
* `%o`
* `%p`
* `%n`
* `%lc/%ls`

当前维持 family 粒度，而不是每个 specifier 都独立，是为了：

1. 测试矩阵可控
2. 用户配置可理解
3. 未来继续下沉到对象级时不冲突

### 15.4 禁用 specifier 的语义

这一点必须固定，不然测试永远不稳定。

当前建议明确为：

1. 禁用的 specifier 仍然正确消费参数
2. 但不执行真实格式化
3. 输出 fallback 文本，当前约定为原始 `%<specifier>` 形式
4. 后续参数解析不能错位

例如：

```c
snprintf(buf, sizeof(buf), "%c %s %d", 'A', "ok", 7);
```

如果 `%c` 被禁用，期望语义是：

```text
%c ok 7
```

而不是：

* 崩溃
* 吞掉字符不输出
* 后续 `va_list` 错位

### 15.5 未来可扩展的第四层

如果以后确实需要更细控制，建议扩展顺序是：

1. 先拆 `%f/%e/%g/%a`
2. 再拆 `%d` 与 `%i`
3. 再拆 `%x` 与 `%X`

但这不是当前阶段的首要任务。

---

## 15A. public API 绑定设计

这一节专门回答一个最容易混淆的问题：

```text
默认 printf / iprintf / printf_full / wprintf
到底分别绑定到什么 core 壳、什么 capability、什么 entry/helper 集合
```

### 15A.1 绑定总图

```text
default family
  printf / fprintf / vprintf / vfprintf
  sprintf / snprintf / vsprintf / vsnprintf
    |
    +--> default capability set
    +--> default core shell selection
    +--> normal entry/helper subset
    +--> stream bridge or memory bridge

integer family
  iprintf / fiprintf / siprintf / vfiprintf
    |
    +--> integer capability set
    +--> integer-oriented core shell selection
    +--> integer/string/char entry subset
    +--> stream bridge or memory bridge

full family
  printf_full / fprintf_full / vfprintf_full
  sprintf_full / snprintf_full
    |
    +--> full capability set
    +--> full parser-capable core shell
    +--> all enabled entry/helper objects
    +--> stream bridge or memory bridge

wide family
  wprintf / fwprintf / vfwprintf
  swprintf / vswprintf
    |
    +--> wide capability set
    +--> full parser-capable core shell
    +--> wchar-capable entry/helper subset
    +--> wide stream or wide memory bridge
```

### 15A.2 先区分两种“绑定”

这里必须把两种绑定分开：

1. public API 绑定
   某个入口函数选择哪一套能力语义

2. core shell 绑定
   某套能力语义最终使用哪一个 `__printf*` parser/core 产物

也就是说：

* `iprintf` 不是直接等于某个 `__printf$flags`
* `printf_full` 也不是简单等于某个 `__printf$sizespec`

中间还隔着 capability 选择和 bridge 选择。

### 15A.3 default family 绑定

#### 入口集合

* `printf`
* `fprintf`
* `vprintf`
* `vfprintf`
* `sprintf`
* `snprintf`
* `vsprintf`
* `vsnprintf`

#### 能力语义

由下面两层共同决定：

* `MCULIB_PRINTF_DEFAULT_PROFILE`
* `MCULIB_PRINTF_DEFAULT_ENABLE_*`

#### core 壳选择

当前目标设计下，默认建议绑定到“默认 parser 能力需要的最小壳”。

如果默认 `printf` 需要完整的：

* flags
* width / precision
* size-spec

那它就应绑定到：

```text
__printf$flags$sizespec$widthprec
```

如果未来确实有“极简默认版”，也可以绑定到更小壳，但这属于后续优化，不作为第一阶段前提。

#### entry/helper 子集

default family 不一定天然等于 full。
它应只引用：

* 当前 profile 允许的长度能力
* 当前 family toggle 允许的 specifier family

例如：

* 禁 `%c` 时不引用 `_printf_c`
* 禁 `%p` 时不引用 `_printf_p`
* 非 float profile 时不应建立 `_printf_f/_printf_e/_printf_g/_printf_a` 的有效引用路径

### 15A.4 integer family 绑定

#### 入口集合

* `iprintf`
* `fiprintf`
* `siprintf`
* `vfiprintf`

#### 能力语义

整数版能力集合建议固定包含：

* `%c`
* `%s`
* `%%`
* `%d/%i`
* `%u`
* `%x/%X`
* `%o`
* `%p`

默认不包含：

* `%f/%e/%g/%a`
* `%n` 是否保留，需要单独拍板
* `%lc/%ls`

#### core 壳选择

整数版不代表 parser 简化版。
如果整数版仍支持完整 flags / width / precision / size-spec，它也可以绑定到：

```text
__printf$flags$sizespec$widthprec
```

区别不在 core 壳，而在 capability 和 entry/helper 子集。

这点非常关键：

```text
core 壳解决“能解析哪些 printf 语法”
capability 解决“允许哪些格式能力”
```

#### entry/helper 子集

整数版建议只允许：

* `_printf_c`
* `_printf_s`
* `_printf_d`
* `_printf_i`
* `_printf_u`
* `_printf_x`
* `_printf_o`
* `_printf_p`
* `_printf_percent`

以及对应 helper：

* `_printf_char_core`
* `_printf_string_core`
* `_printf_int_dec`
* `_printf_int_hex`
* `_printf_int_oct`
* `_printf_hex_ptr`

### 15A.5 full family 绑定

#### 入口集合

* `printf_full`
* `fprintf_full`
* `vfprintf_full`
* `sprintf_full`
* `snprintf_full`

#### 能力语义

full family 代表完整能力集合，至少包括：

* 全整数族
* 全浮点族
* `%n`
* 宽字符相关能力
* 完整长度修饰支持

#### core 壳选择

full family 应直接绑定完整 parser 能力壳：

```text
__printf$flags$sizespec$widthprec
```

#### entry/helper 子集

full family 可以引用全部 `_printf_*` entry 与全部 helper。

### 15A.6 wide family 绑定

#### 入口集合

* `wprintf`
* `fwprintf`
* `vfwprintf`
* `swprintf`
* `vswprintf`

#### 能力语义

wide family 的重点不是“它一定比 full 更多”，而是：

* 输出单元是 wide-oriented
* `%lc/%ls` 等宽字符路径必须成立
* bridge 层需要宽字符版本

#### core 壳选择

wide family 通常也应绑定完整 parser 能力壳：

```text
__printf$flags$sizespec$widthprec
```

#### entry/helper 子集

wide family 至少需要：

* `_printf_c` / `_printf_s` 的宽字符兼容路径
* 必要的整数与浮点 helper
* 宽 stream / 宽 memory bridge

### 15A.7 一张汇总表

| family | 典型 API | parser/core 壳 | capability 焦点 | bridge |
| --- | --- | --- | --- | --- |
| default | `printf/snprintf/vfprintf` | 通常完整 parser 壳 | `PROFILE + FAMILY TOGGLE` | stream / memory |
| integer | `iprintf/fiprintf/vfiprintf` | 通常完整 parser 壳 | 整数/字符串/字符能力集 | stream / memory |
| full | `printf_full/vfprintf_full` | 完整 parser 壳 | 全能力集 | stream / memory |
| wide | `wprintf/swprintf/vfwprintf` | 完整 parser 壳 | 宽字符 + 全 parser | wide stream / wide memory |

### 15A.8 当前实现时最重要的约束

实现时必须避免两个错误：

1. 把 family 差异硬塞成“不同 core 逻辑源码”
2. 把 core 壳差异误当成“public API 语义分层”

正确关系应是：

```text
public API family
    -> capability set
    -> bridge choice
    -> selected core shell
    -> selected entry/helper subset
```

而不是：

```text
public API family
    -> 一份完全独立的 formatter 源码
```

---

## 16. parser 与 core 状态设计

### 16.1 parser 状态对象

无论最终结构体叫什么名字，都建议把 core 解析状态统一成一份 descriptor，例如：

```text
flags
width
precision
size-spec
specifier
output count
error state
```

这份 descriptor 应该是：

* core 模板的唯一状态载体
* specifier entry 的统一输入
* unsupported/fallback 路径也能复用的描述对象

### 16.2 parser 子步骤

建议逻辑上固定成这四步：

1. 解析 flags
2. 解析 width / precision
3. 解析 size-spec
4. 识别 conversion specifier

这正好和我们已经在做的：

* `__printf_flags`
* `__printf_wp`
* `__printf_ss`

相对应。

### 16.3 core 主循环职责

core 主循环只做这些事：

1. 扫描普通字符
2. 遇到 `%` 时构造 parse descriptor
3. 调 parser 子步骤
4. 根据当前能力集合与 family toggle 做分发
5. 执行 entry 或 fallback
6. 更新计数和错误态

不应再让 core 主循环直接承担：

* 具体整数转换算法
* 具体浮点转换算法
* 具体 FILE 刷新策略

---

## 17. 输出层、FILE 与 fflush 设计

### 17.1 为什么 core 不应直接依赖 FILE

这是当前设计里必须坚持的一点。

原因很直接：

* `sprintf/snprintf` 根本不是 FILE 输出
* 后续 semihost/RTT/backend 也不一定是标准 FILE
* 如果 core 直接吃 `FILE *`，字符串路径和自定义 backend 会被迫绕路

因此建议继续坚持：

```text
core 只面向统一 out/context 抽象
FILE 只是其中一种桥接方式
```

### 17.2 输出抽象建议

建议输出层至少抽象出下面这些能力：

* `putc`
* `write span`
* `count`
* `error`
* `flush if needed`

如果已经有现成结构，就按现有结构收敛，不需要为抽象而抽象。

### 17.3 `sprintf/snprintf` 的正确位置

`sprintf/snprintf` 应继续复用同一 core，只是换成 memory backend。

也就是说：

* 不再单独维护 string formatter
* 也不让 `sprintf` 假装调用真正的 `vfprintf(FILE*, ...)`

它们应该是：

```text
public wrapper
-> memory out bridge
-> shared core
-> trailing '\0'
```

### 17.4 fflush 支持应该放在哪里

`fflush` 不应该放在 specifier 层，也不应该放在 core parser 层。

它应该只出现在：

* stream bridge
* FILE backend
* line/full/unbuffered policy 处理层

core 唯一需要知道的是：

* 某次写出是否失败
* 累计输出计数是多少

### 17.5 backend 分层建议

建议把 backend 明确分成三层：

1. `printf` core
   只做格式化

2. stream bridge
   负责把 core 输出映射到 FILE 或 memory

3. device/backend ops
   semihost / RTT / newlib syscall / board-specific write

这样后续讨论“fflush 开还是关”“line buffered 要不要启用”时，就不会再次污染 core 设计。

---

## 18. 宽字符与 locale 设计边界

### 18.1 wchar 支持的定位

当前建议：

* `wprintf` 族是单独 public API 能力层
* `%lc/%ls` 在默认 `printf` 里是否支持，继续由 `MCULIB_PRINTF_DEFAULT_ENABLE_WCHAR` 控制

这样做的原因是：

* `wprintf` 族本身是一条独立语义线
* 默认 `printf` 对宽字符的支持常常是成本敏感项

### 18.2 locale 的处理原则

当前阶段不建议把 locale 设计做深。

建议先明确：

1. 默认路径不引入复杂 locale 依赖
2. 浮点和数字输出先以稳定基本语义为主
3. 后续如果要补 locale，再作为单独能力块讨论

这点和 `newlib-nano` 的启发是一致的：

* 先保住默认路径的小而稳
* 不在第一阶段引入大而散的语义面

---

## 19. 测试与 codesize 观测设计

### 19.1 测试分层

建议把测试分成四类：

1. smoke
   验证常见 `%c/%s/%d/%u/%x/%p`

2. config
   验证 family toggle 禁用后的 fallback 与 `va_list` 正确性

3. profile
   验证 `INT/LONG/LONG_LONG/FLOAT_*` 档位差异

4. wide/float/flush
   验证大能力块与 stream 行为

### 19.2 必补的裁剪测试

后续至少建议补这些场景：

* 禁 `%s`
* 禁 `%p`
* 禁 `%n`
* 禁 `%f`
* `LONG` profile 下拒绝 `%lld`
* `INT` profile 下拒绝 `%ld/%lld`

### 19.3 codesize 对比口径

codesize 对比要固定口径，不然数据没有可讨论性。

建议至少固定：

1. 同一份测试源码
2. 同一组编译选项
3. 只切换 `PROFILE` 或 family toggle
4. 分别观察：
   * 整体 ELF 大小
   * `printf` 相关对象拉入集合
   * 浮点 helper 是否被带入

---

## 20. 实施顺序

### 20.1 第一阶段

先把文档和文件骨架收敛到“单模板、多壳文件”的设计。

交付标准：

* 文档不再混淆“core 产物”和“public 能力入口”
* 文件级命名方案拍板

### 20.2 第二阶段

把现有 core 改造成模板结构，但先不追求一次到位做完所有变体。

交付标准：

* 默认 core 与至少一个扩展 core 能由同模板生成
* 现有测试不退化

### 20.3 第三阶段

继续压实 specifier entry 与 helper 边界。

交付标准：

* `%d/%i/%u`
* `%x/%o/%p`
* `%f/%e/%g/%a`

三组边界能稳定解释。

### 20.4 第四阶段

再决定是否引入更明显的链接期能力选择。

交付标准：

* 已有对象边界足够稳定
* codesize 数据表明值得继续下沉

---

## 21. 最终拍板

这份文档现在正式收口成下面这组结论：

1. 我们的目标不是维护多份独立 `printf core` 源码，而是维护一份 core 模板源码。
2. 我们的目标也不是“最终只有一个 core 产物”，而是允许对象层存在多份 `__printf*` 变体。
3. 对外 public API 继续保留 `printf / iprintf / printf_full / wprintf` 的能力分层。
4. 默认 `printf` 继续是非 full、可配置、可测试、可解释的入口。
5. `sprintf/snprintf` 继续通过 memory bridge 复用同一 core，不单独维护另一套 formatter。
6. `fflush`、buffering、backend 继续停留在 stream/device 层，不回流到 core/parser/specifier 层。
7. 内部对象边界继续向 Arm 风格收敛，外层能力分层继续吸收 emRun 的做法，默认能力策略继续吸收 `newlib-nano` 的做法。

一句话定稿：

```text
单份 core 模板
+ 多份 core 产物
+ 明确 public 能力分层
+ 默认 printf 非 full
+ backend/flush 与格式化核心解耦
```

---

## 22. 当前文件到目标角色的映射

这一节的目的不是列“所有文件清单”，而是回答：

```text
当前仓库里已经有的 printf 文件
在目标架构里分别应该扮演什么角色
```

这样后面改代码时，就能直接按“保留 / 收敛 / 吸收 / 过渡”四类处理。

### 22.1 映射状态定义

先固定四种状态：

* `保留`
  当前职责和目标设计一致，继续保留
* `收敛`
  当前方向对，但后续要按目标边界进一步整理
* `吸收`
  当前文件逻辑后续会被模板层或更稳定边界吸收
* `过渡`
  当前能用，但不是最终形态

### 22.2 public API 层映射

| 当前文件类型 | 目标角色 | 状态 | 说明 |
| --- | --- | --- | --- |
| `printf.c` | default family API wrapper | 保留 | 继续作为默认入口 |
| `fprintf.c` | default family API wrapper | 保留 | 继续走 stream bridge |
| `vprintf.c` | default family API wrapper | 保留 | 薄封装 |
| `vfprintf.c` | default family API wrapper | 保留 | 薄封装，不再回到旧大实现 |
| `sprintf.c` | default family API wrapper | 保留 | 走 memory bridge |
| `snprintf.c` | default family API wrapper | 保留 | 走 memory bridge |
| `vsprintf.c` / `vsnprintf.c` | default family API wrapper | 保留 | 走 memory bridge |
| `iprintf.c` / `fiprintf.c` / `siprintf.c` / `vfiprintf.c` | integer family wrapper | 保留 | 明确保持为整数能力入口 |
| `printf_full.c` 等 full 族入口 | full family wrapper | 保留 | 明确保持为 full 能力入口 |
| `wprintf.c` 等 wide 族入口 | wide family wrapper | 保留 | 宽字符能力入口 |

这层的原则很明确：

* public API 文件大部分都应该保留
* 后续变化主要是它们绑定到哪套 capability / bridge / core 壳
* 不应该再把它们重新膨胀成真正的大 formatter

### 22.3 core / parser 层映射

| 当前文件 | 目标角色 | 状态 | 说明 |
| --- | --- | --- | --- |
| `__printf_core.c` | 单份 core 模板的主要来源 | 吸收 | 后续应收敛进 `printf_core_body.inc` 一类模板 |
| `__printf_core_default.c` | default family 特化逻辑来源 | 吸收 | 其中 gating/fallback 逻辑要并入统一模板和 capability 机制 |
| `__printf.c` | core shell | 收敛 | 保留为壳文件，不再承载重复逻辑 |
| `__printf_flags.c` | core shell 或 parser壳 | 收敛 | 目标是只负责壳/边界，不承载额外业务 |
| `__printf_wp.c` | core shell 或 parser壳 | 收敛 | 同上 |
| `__printf_ss.c` | core shell 或 parser壳 | 收敛 | 同上 |

这层后续最关键的动作是：

* 把真正共享的 core 循环逻辑收拢成一份模板
* 把 `__printf*.c` 压成薄壳
* 不再继续维持多份逻辑近似重复的 core 源文件

### 22.4 capability / config 层映射

| 当前文件 | 目标角色 | 状态 | 说明 |
| --- | --- | --- | --- |
| `printf_config.h` | 默认配置模型定义 | 保留 | 继续承载 `PROFILE + FAMILY TOGGLE` |
| `__printf_caps_default.c` | default capability set | 收敛 | 后续要和模板化 core 绑定关系再理顺 |
| `__printf_caps_full.c` | full capability set | 收敛 | 同上 |
| 其他 `caps` 相关实现 | family capability set | 收敛 | 最终应明确谁控制 family，谁控制 parser 能力 |

这层最容易混淆的点是：

* `caps` 不是 public API
* `caps` 也不是 core shell
* `caps` 负责的是“允许哪些 conversion 语义”

### 22.5 specifier entry 层映射

| 当前文件 | 目标角色 | 状态 | 说明 |
| --- | --- | --- | --- |
| `_printf_c.c` | char entry | 保留 | 保持薄 entry |
| `_printf_s.c` | string entry | 保留 | 保持薄 entry |
| `_printf_d.c` | signed decimal entry | 保留 | 目标边界 `_printf_d -> _printf_int_dec` |
| `_printf_i.c` | signed integer entry | 保留 | 目标边界 `_printf_i -> _printf_int_dec` |
| `_printf_u.c` | unsigned decimal entry | 保留 | 目标边界 `_printf_u -> _printf_int_dec` |
| `_printf_x.c` | hex entry | 保留 | 目标边界 `_printf_x -> _printf_int_hex` |
| `_printf_o.c` | octal entry | 保留 | 目标边界 `_printf_o -> _printf_int_oct` |
| `_printf_p.c` | pointer entry | 保留 | 目标边界 `_printf_p -> _printf_hex_ptr` |
| `_printf_n.c` | percent-n entry | 保留 | 小对象即可 |
| `_printf_percent.c` | literal percent entry | 保留 | 小对象即可 |
| `_printf_f.c` / `_printf_e.c` / `_printf_g.c` | decimal float entry | 保留 | 目标边界指向 `_printf_fp_dec` |
| `_printf_a.c` | hex float entry | 保留 | 目标边界指向 `_printf_fp_hex` |

这层的处理原则是：

* 不再把 entry 做大
* 不把 parser 回流到 entry
* 不把 stream/backend 逻辑塞进 entry

### 22.6 helper 层映射

| 当前文件类型 | 目标角色 | 状态 | 说明 |
| --- | --- | --- | --- |
| 整数公共格式化 helper | shared integer helper | 收敛 | 继续压实 dec/hex/oct 边界 |
| 浮点公共格式化 helper | shared float helper | 收敛 | 继续压实 fp_dec/fp_hex 边界 |
| 字符/字符串公共 helper | shared char/string helper | 收敛 | 统一宽度、对齐、截断逻辑 |
| long long / float 辅助文件 | capability-dependent helper | 过渡 | 需要后续梳理谁属于 family，谁属于 parser/caps |

这里的重点不是“文件名今天是否完美”，而是：

* helper 必须是可复用算法层
* helper 不应再知道 public API 语义

### 22.7 output / bridge / backend 层映射

| 当前文件类型 | 目标角色 | 状态 | 说明 |
| --- | --- | --- | --- |
| `printf_stdio_bridge` 一类文件 | stream bridge | 保留 | 继续承担 FILE 路径桥接 |
| memory buffer 路径相关实现 | memory bridge | 保留 | `sprintf/snprintf` 继续复用 |
| `fflush` / stream flush 相关实现 | flush policy layer | 收敛 | 明确只留在 stream/backend 层 |
| semihost / RTT / syscall backend 对接 | backend ops | 保留 | 继续与 core 解耦 |

这一层最关键的约束是：

* core 不直接依赖 `FILE`
* `sprintf/snprintf` 不单独发明第二套 formatter
* `fflush` 不回流到 parser/specifier 层

### 22.8 哪些东西后续最可能发生物理重组

后续最可能被重组的是这几类：

1. `__printf_core*`
   这些最可能被收拢到单模板 + 多壳文件结构

2. parser 相关分散逻辑
   会逐步向 `printf_core_parser.inc + __printf_flags/wp/ss` 的边界收敛

3. 浮点和 long long helper
   会继续按 capability 和 shared helper 关系整理

4. flush / stream 行为
   会继续从一般输出逻辑里剥离得更明确

### 22.9 哪些东西原则上不应再大改方向

下面这些东西现在原则上不应再大改方向：

1. public API 分层
   `printf / iprintf / printf_full / wprintf`

2. `sprintf/snprintf` 复用 shared core

3. backend/flush 留在 stream/device 层

4. specifier entry 继续薄化

5. 默认 `printf` 继续非 full 且可配置

### 22.10 用这张映射表指导后续修改

后续每次改代码，都建议先回答三个问题：

1. 这个文件在目标架构里属于哪一层
2. 它当前状态是 `保留 / 收敛 / 吸收 / 过渡` 哪一种
3. 这次修改是在“压实职责边界”，还是又把逻辑搅回去了

如果这三点回答不清楚，就不应该直接开始改实现。
