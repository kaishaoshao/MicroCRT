# emRun `vfprintf` 变体与别名/重定位机制调研

## 1. 调研目标

这份文档专门分析 `clib/emrun` 里的 `printf`/`vfprintf` 体系，重点回答三个问题：

1. emRun 是否把同一份 `printf` core 源码编译成多份 `vfprintf` 变体
2. public `vfprintf` / `printf` 包装层是如何接到内部 core 上的
3. 你说的“通过别名技术让重定位到不同 `vfprintf` 实现”这一点，在当前仓库里哪些部分能直接证明，哪些还只是合理推断

本报告尽量只基于仓库里能看到的源码、工程文件和本地编译出的汇编。

---

## 2. 总结先行

先给压缩结论：

### 2.1 已经可以直接证明的事实

1. emRun 的 `prinops.c` 是一份共享 core 源码。
2. 同一份 `prinops.c` 会被工程文件多次编译，生成很多 `__SEGGER_RTL_vfprintf_*` 变体。
3. public 的 `vfprintf_l()` 包装层不会自己做格式解析，而是调用一个 canonical core 符号：

```text
__SEGGER_RTL_vfprintf
```

### 2.2 目前源码里不能直接证明、但非常像是设计意图的事实

仓库源码和 `.emProject` 文件里，没有直接看到这种显式 C 级别别名：

```c
int __SEGGER_RTL_vfprintf(...) __attribute__((alias("__SEGGER_RTL_vfprintf_long")));
```

也没有直接看到显式汇编等价式：

```asm
.equiv __SEGGER_RTL_vfprintf, __SEGGER_RTL_vfprintf_long
```

所以：

* “canonical `__SEGGER_RTL_vfprintf` 最终如何绑定到某个具体变体”
  这一点在当前源码树里**没有被直接展示出来**

### 2.3 最稳妥的结论

因此最严谨的结论应当是：

```text
emRun 已经明确采用了：
单份 core 源码
  -> 多份 vfprintf 变体产物
  -> public wrapper 调 canonical symbol

至于 canonical symbol 最终如何指向某个具体变体，
当前仓库可见部分没有直接给出最终别名规则，
但工程组织方式明显就是为这种选择/重定位机制服务的。
```

---

## 3. 关键源码与工程文件

调研主要看了这些文件：

### 3.1 core 源码

```text
clib/emrun/emRun/prinops.c
clib/emrun/emRun/wprinops.c
```

### 3.2 内部接口定义

```text
clib/emrun/include/__SEGGER_RTL_Int.h
clib/emrun/include/stdio.h
```

### 3.3 工程文件

```text
clib/emrun/emRun/arm_auto.emProject
clib/emrun/emRun/riscv_auto.emProject
```

---

## 4. emRun 的 core 源码结构

### 4.1 `prinops.c` 里同时包含两层内容

`prinops.c` 不是只写 public `printf` 接口，也不是只写 internal core。
它同时包含：

1. core 格式化函数
2. public `snprintf/sprintf/vfprintf/printf/...` 包装层
3. 输出上下文和 stream/string write 适配层

### 4.2 internal core 叫 `__SEGGER_RTL_prin`

在 `__SEGGER_RTL_Int.h` 里可以直接看到：

```c
int __SEGGER_RTL_prin(__SEGGER_RTL_prin_t *ctx, const char *fmt, ARGTYPE args);
```

而 `prinops.c` 里有真正的 core 实现：

```c
int __SEGGER_RTL_prin(__SEGGER_RTL_prin_t *ctx, const char *fmt, ARGTYPE args) { ... }
```

这个函数负责：

* 扫描格式串
* 解析 flags
* 解析 width/precision
* 解析长度修饰符
* 处理 `%d/%i/%u/%x/%o/%p/%s/%c/%f/%e/%g/%a/%n`
* 通过 `__SEGGER_RTL_putc()` 输出字符

所以 emRun 的 core 本质上也是：

```text
格式化核心
+ 输出上下文
+ output_fn 回调
```

### 4.3 output 抽象在 `__SEGGER_RTL_prin_t`

在 `__SEGGER_RTL_Int.h` 中：

```c
typedef struct __SEGGER_RTL_prin_tag {
  __SEGGER_RTL_SIZE_T   charcount;
  __SEGGER_RTL_SIZE_T   maxchars;
  __SEGGER_RTL_PRIN_STR string;
  __SEGGER_RTL_PRIN_BUF buffer;
  locale_t              locale;
  int (* output_fn)(struct __SEGGER_RTL_prin_tag *ctx, const char *pData, unsigned DataLen);
} __SEGGER_RTL_prin_t;
```

这说明 emRun 的格式化层和最终输出目标是分开的。

---

## 5. 同一源码多次编译成多份 `vfprintf` 变体

这里是最关键的证据。

### 5.1 `arm_auto.emProject` 明确列出很多 `prinops.c` 变体

在 `clib/emrun/emRun/arm_auto.emProject` 中可以直接看到：

```text
__SEGGER_RTL_vfprintf_int
__SEGGER_RTL_vfprintf_int_nwp
__SEGGER_RTL_vfprintf_long
__SEGGER_RTL_vfprintf_long_nwp
__SEGGER_RTL_vfprintf_long_long
__SEGGER_RTL_vfprintf_long_long_nwp
__SEGGER_RTL_vfprintf_float_long
__SEGGER_RTL_vfprintf_float_long_long
__SEGGER_RTL_vfprintf_short_float_long
__SEGGER_RTL_vfprintf_short_float_long_long
...
以及对应 wchar 版本
```

`riscv_auto.emProject` 也有同样结构。

### 5.2 它们全部来自同一个文件 `prinops.c`

工程文件里每个变体条目都写的是：

```text
<file Name="__SEGGER_RTL_vfprintf_long" file_name="prinops.c">
```

这说明：

* 不是很多不同 `.c` 文件各写一套
* 而是同一个 `prinops.c` 被重复编译

### 5.3 变体名通过预处理宏注入

在同一个 folder 的公共配置中，直接写了：

```text
c_preprocessor_definitions="__SEGGER_RTL_prin=$(ProjectNodeName)"
```

这意味着：

* 当项目节点名是 `__SEGGER_RTL_vfprintf_long`
* 预处理就会得到：

```c
#define __SEGGER_RTL_prin __SEGGER_RTL_vfprintf_long
```

于是 `prinops.c` 里的 core 定义：

```c
int __SEGGER_RTL_prin(__SEGGER_RTL_prin_t *ctx, const char *fmt, ARGTYPE args)
```

会在预处理后变成：

```c
int __SEGGER_RTL_vfprintf_long(__SEGGER_RTL_prin_t *ctx, const char *fmt, ARGTYPE args)
```

### 5.4 本地预处理直接证明了这一点

我用 clang 对 `prinops.c` 做了预处理，定义：

```text
__SEGGER_RTL_prin=__SEGGER_RTL_vfprintf_long
__SEGGER_RTL_FORMAT_INT_WIDTH=__WIDTH_LONG
__SEGGER_RTL_FORMAT_FLOAT_WIDTH=__WIDTH_NONE
__SEGGER_RTL_FORMAT_WIDTH_PRECISION=1
__SEGGER_RTL_FORMAT_WCHAR=0
```

预处理结果里直接出现：

```text
int __SEGGER_RTL_vfprintf_long(__SEGGER_RTL_prin_t *ctx, const char *fmt, __SEGGER_RTL_VA_LIST args)
```

这已经足够证明：

```text
emRun 使用“同一份 core 源码 + 宏改名 + 多次编译”的方式生成多份 vfprintf 变体
```

---

## 6. emRun 变体能力是靠宏裁剪的

每个变体不仅名字不同，功能集合也不同。

### 6.1 整数宽度档位

工程文件里会传：

```text
SUPPORT_INT
SUPPORT_LONG
SUPPORT_LONG_LONG
```

在 `prinops.c` 中又通过：

```c
#if __SEGGER_RTL_FORMAT_INT_WIDTH == __WIDTH_LONG_LONG
...
#elif __SEGGER_RTL_FORMAT_INT_WIDTH == __WIDTH_LONG
...
#elif __SEGGER_RTL_FORMAT_INT_WIDTH == __WIDTH_INT
...
#endif
```

把 `value_t / uvalue_t / SUPPORT_LONG / SUPPORT_LONG_LONG` 等行为裁剪掉。

### 6.2 浮点档位

工程文件里会传：

```text
__SEGGER_RTL_FORMAT_FLOAT_WIDTH=__WIDTH_NONE
__SEGGER_RTL_FORMAT_FLOAT_WIDTH=__WIDTH_FLOAT
__SEGGER_RTL_FORMAT_FLOAT_WIDTH=__WIDTH_DOUBLE
```

这样同一份源码能编出：

* 无浮点版
* short-float 版
* double 版

### 6.3 width/precision 档位

工程文件里还有：

```text
__SEGGER_RTL_FORMAT_WIDTH_PRECISION=1
__SEGGER_RTL_FORMAT_WIDTH_PRECISION=0
```

也就是：

* 正常 width/precision 支持
* nwp 版，去掉 width/precision 解析

### 6.4 wchar 档位

工程文件里还有：

```text
__SEGGER_RTL_FORMAT_WCHAR=0
__SEGGER_RTL_FORMAT_WCHAR=1
```

于是又能进一步生成宽字符变体。

所以 emRun 的变体模型本质上是：

```text
一个 core 源
+ 多组功能宏
+ 多个导出符号名
```

---

## 7. public `vfprintf` 包装层是怎么接到 core 上的

### 7.1 `WITH_PUBLICS` 版不会再定义 internal core

`prinops.c` 中有这段条件编译：

```c
#if !defined(WITH_PUBLICS) && !defined(WITH_PRINTF)
int __SEGGER_RTL_prin(...) { ... }
#endif
```

这说明：

* 当编译 internal variant object 时，不带 `WITH_PUBLICS`，会生成真正的 core
* 当编译 public wrapper 版时，带 `WITH_PUBLICS`，不会再生成 core 本体

这很关键，说明 public 和 internal 在构建上就是分开的两类产物。

### 7.2 public `vfprintf_l()` 会调用 canonical core 符号

在 public 包装层里：

```c
int vfprintf_l(FILE *stream, locale_t loc, const char *format, va_list arg) {
  ...
  n = __SEGGER_RTL_prin(&iod.prin, format, arg);
  ...
}
```

而在 public 这次编译中，工程又传了：

```text
WITH_PRINTF
WITH_PUBLICS
__SEGGER_RTL_prin=__SEGGER_RTL_vfprintf
```

因此这段源码在 public 编译下实际会变成：

```c
n = __SEGGER_RTL_vfprintf(&iod.prin, format, arg);
```

### 7.3 本地汇编也直接证实了这一点

我用 clang 把 public 版 `prinops.c` 编成汇编后，看到：

```text
vfprintf:
  callq vfprintf_l

vfprintf_l:
  ...
  callq __SEGGER_RTL_vfprintf@PLT
```

这说明：

* public `vfprintf_l()` 自己不是 formatter 核心
* 它只做 stream/context 初始化
* 最终它会跳到 canonical core 符号 `__SEGGER_RTL_vfprintf`

---

## 8. internal variant 汇编证据

我又把 internal variant 版 `prinops.c` 编成汇编，定义：

```text
__SEGGER_RTL_prin=__SEGGER_RTL_vfprintf_long
__SEGGER_RTL_FORMAT_INT_WIDTH=__WIDTH_LONG
__SEGGER_RTL_FORMAT_FLOAT_WIDTH=__WIDTH_NONE
__SEGGER_RTL_FORMAT_WIDTH_PRECISION=1
__SEGGER_RTL_FORMAT_WCHAR=0
```

编译得到的汇编起始符号是：

```text
__SEGGER_RTL_vfprintf_long:
```

并且这个函数内部直接调用：

```text
__SEGGER_RTL_putc
__SEGGER_RTL_prin_flush
```

这说明：

* internal variant 真的是 formatter 主体
* public `vfprintf_l()` 和 internal `__SEGGER_RTL_vfprintf_long()` 是两层不同角色

---

## 9. “别名技术让重定位到不同实现”到底看到了什么

这里要把已证实和未证实分开。

### 9.1 已证实

已证实的链路是：

```text
public wrappers
    -> call canonical symbol __SEGGER_RTL_vfprintf

internal variants
    -> define __SEGGER_RTL_vfprintf_int / long / long_long / float_long / ...
```

也就是说：

* emRun 明确把“公共入口”与“具体 core 变体”分开
* 公共入口面向一个 canonical symbol
* 内部存在很多具体实现可供选择

### 9.2 当前源码树里没有直接看到的东西

我在 `clib/emrun` 里没有找到下面这些直接证据：

* `__attribute__((alias("__SEGGER_RTL_vfprintf_long")))`
* `.equiv __SEGGER_RTL_vfprintf, __SEGGER_RTL_vfprintf_long`
* `#define __SEGGER_RTL_vfprintf __SEGGER_RTL_vfprintf_long`
* 明确把某个变体重命名成 canonical `__SEGGER_RTL_vfprintf` 的 C 源码

也就是说：

```text
“canonical __SEGGER_RTL_vfprintf 最终如何被绑定到某个具体变体”
这一跳，
当前仓库可见源码没有直接把规则写出来。
```

### 9.3 最合理的解释

从工程结构推断，最合理的解释是：

```text
emRun 的构建/打包流程在仓库之外或工程工具内部，
会选择一个具体变体，
再让 canonical __SEGGER_RTL_vfprintf 指向它。
```

这个“指向”可以通过很多办法实现：

* 别名符号
* 弱符号覆盖
* 库对象选择
* 构建时重命名
* 打包时再导出统一名

但仅从当前仓库源码，不能把其中某一种写成“已证实”。

所以更严谨的表述是：

```text
emRun 的整体组织明显是为“canonical wrapper + selected variant core”服务的，
但当前源码树没有直接暴露最后一跳的别名实现细节。
```

---

## 10. emRun 方法的工程特点

基于上面的证据，emRun 的 `vfprintf` 体系可以概括成：

### 10.1 优点

1. 真正做到“同一份 core 源码，多份变体产物”
2. public wrapper 与 internal core 边界清楚
3. 变体组合覆盖面大：整数宽度、浮点宽度、width/precision、wchar
4. 输出层通过 `__SEGGER_RTL_prin_t + output_fn` 解耦，适配文件流、字符串和各种后端

### 10.2 缺点

1. 变体名字很多，理解门槛高
2. 最终 canonical symbol 如何选中某个变体，源码层可见性不强
3. 变体依赖编译系统组织，普通用户只看 C 源码不容易一眼看懂最终装配关系
4. 它的拆分重点更偏“同源多编译变体”，不像 Arm 那样在 specifier entry object 上拆得那么细

---

## 11. 对我们的直接启发

这份调研对我们自己的设计有很直接的启发。

### 11.1 应该学习 emRun 的部分

最值得学的是这三点：

1. 只维护一份 core 逻辑源
2. 用不同宏编译出多份 `printf_core_xxx`
3. public `vfprintf` wrapper 永远只面向 canonical core 符号

这正是你现在希望我们走的方向。

### 11.2 不应该照抄的部分

不建议完全照抄的点是：

1. 不要把最终 canonical symbol 的绑定规则藏得太深
2. 不要只停在“多份 core 变体”，还要继续考虑 Arm 风格的 specifier/helper 对象边界

也就是说：

```text
core 变体生成方式可以像 emRun
对象边界细化方向仍然要参考 Arm
```

---

## 12. 最终结论

最终结论压缩成四句话：

1. emRun 的 `prinops.c` 的确是一份共享 `printf` core 源码。
2. emRun 的工程文件明确把这份源码编译成大量 `__SEGGER_RTL_vfprintf_*` 变体。
3. public `vfprintf_l()` 包装层会调用 canonical `__SEGGER_RTL_vfprintf`，这一点源码和汇编都能直接证明。
4. canonical `__SEGGER_RTL_vfprintf` 最终如何绑定到某个具体变体，在当前仓库源码里没有直接看到最终别名实现，因此这一步只能谨慎表述为“面向选择/重定位机制设计”，不能写成“源码里已直接证明的 alias 规则”。

