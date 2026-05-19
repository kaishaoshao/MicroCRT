# Arm `armlib` `printf` 调研文档

## 1. 调研目标

这份文档只回答 Arm `armlib` 的对象和汇编层事实，不讨论链接器脚本，不讨论我们自己的方案取舍。

调研目标是三个：

1. Arm 的 `printf` 在对象层到底是不是“只有一个 core”
2. Arm 的 specifier 和 shared helper 是怎么拆的
3. 这些事实对我们后续设计有什么启发

---

## 2. 调研对象与工具

调研对象：

```text
/home/shaokai/ArmCompilerforEmbedded6.24/lib/armlib/c_2.l
```

使用工具：

```text
llvm-ar-14
llvm-nm-14
llvm-readelf-14
llvm-objdump-14
llvm-size
```

本报告只基于：

* archive 内的对象文件列表
* 每个对象的符号表
* 每个对象的导出/未定义符号关系
* 反汇编可见的调用边界

---

## 3. 总结先行

先给结论：

```text
Arm 在对象层不是“只有一个 core 函数”。
Arm 在对象层是：
多 core 变体 + 薄 specifier entry object + shared helper
```

但这不等于说 Arm 源码层一定维护了很多份完全独立的 core 源码。

更准确的说法应该是：

```text
Arm 很可能允许“源码逻辑来自同一套 core 模板”，
但最终编译产物层明确生成了多份 core 变体。
```

这个区分很重要。

---

## 4. archive 中可见的关键对象

从 `c_2.l` 里可以直接列出这些关键对象：

### 4.1 core 相关对象

```text
__printf.o
__printf_flags.o
__printf_ss.o
__printf_wp.o
__printf_flags_ss.o
__printf_flags_wp.o
__printf_ss_wp.o
__printf_flags_ss_wp.o
```

### 4.2 specifier entry 对象

```text
_printf_c.o
_printf_s.o
_printf_n.o
_printf_x.o
_printf_p.o
_printf_o.o
_printf_i.o
_printf_d.o
_printf_u.o
_printf_f.o
_printf_e.o
_printf_g.o
_printf_a.o
_printf_percent.o
```

### 4.3 shared helper / sink 相关对象

```text
_printf_char_common.o
_printf_char_file.o
_printf_char_file_locked.o
_printf_dec.o
_printf_intcommon.o
_printf_fp_dec.o
_printf_fp_dec_accurate.o
_printf_fp_hex.o
_printf_fp_infnan.o
_printf_str.o
_printf_pad.o
```

只看对象名，已经能看出它不是“一个大 `vfprintf.c` 全包”的结构。

---

## 5. core 层的直接证据

这里是最关键的部分。

### 5.1 导出符号不是一个

`llvm-readelf-14 -s` 直接能看到：

```text
__printf.o                  -> __printf
__printf_flags.o            -> __printf$flags
__printf_ss.o               -> __printf$sizespec
__printf_wp.o               -> __printf$widthprec
__printf_flags_ss.o         -> __printf$flags$sizespec
__printf_flags_wp.o         -> __printf$flags$widthprec
__printf_ss_wp.o            -> __printf$sizespec$widthprec
__printf_flags_ss_wp.o      -> __printf$flags$sizespec$widthprec
```

这说明：

1. Arm 对象层不只导出一个 `__printf`
2. core 变体是显式命名出来的
3. `flags / sizespec / widthprec` 组合在对象层就是一等公民

### 5.2 大小也不同，不是简单别名

符号大小如下：

```text
__printf                                   size 104
__printf$flags                             size 144
__printf$sizespec                          size 184
__printf$widthprec                         size 270
__printf$flags$sizespec                    size 232
__printf$flags$widthprec                   size 308
__printf$sizespec$widthprec                size 352
__printf$flags$sizespec$widthprec          size 388
```

对象整体 `.text` 大小也对应增长：

```text
__printf.o                  text 104
__printf_flags.o            text 165
__printf_ss.o               text 184
__printf_wp.o               text 284
__printf_flags_ss.o         text 253
__printf_flags_wp.o         text 343
__printf_ss_wp.o            text 366
__printf_flags_ss_wp.o      text 423
```

这说明：

* 这些不是同一个函数名的别名
* 也不是一个统一实现上做极薄跳板
* 对象层确实存在多份不同能力组合的 core 产物

### 5.3 每个 core 变体都直接依赖 `_printf_percent`

符号表里都能看到：

```text
UND _printf_percent
```

这说明这些 core 变体都不是孤立模块，而是都会进入同一套 percent/specifier 分发体系。

也就是说，Arm 的结构不是：

```text
多个完全不同的 printf 实现
```

而是：

```text
多份 core 变体
+ 一套共享 specifier entry / helper 体系
```

---

## 6. specifier entry 对象的直接证据

再看 `_printf_*.o` 这些对象。

### 6.1 每个 specifier entry 极薄

例如：

```text
_printf_c.o
_printf_d.o
_printf_i.o
_printf_x.o
_printf_p.o
_printf_f.o
_printf_a.o
```

它们在符号表里都只有一个 6-byte 的 section，对应一个极薄入口。

这说明它们本身不是 formatter 主体，而只是 very thin entry object。

### 6.2 每个 entry object 只弱依赖一个 helper

直接看未定义符号关系：

```text
_printf_c.o -> weak UND _printf_char
_printf_d.o -> weak UND _printf_int_dec
_printf_i.o -> weak UND _printf_int_dec
_printf_x.o -> weak UND _printf_int_hex
_printf_p.o -> weak UND _printf_hex_ptr
_printf_f.o -> weak UND _printf_fp_dec
_printf_a.o -> weak UND _printf_fp_hex
```

这个证据非常强，说明 Arm 的 specifier entry 层已经被刻意压到很薄。

也说明 Arm 的对象边界不是：

```text
一个大 core 里直接塞满所有 %d/%x/%f 实现
```

而是：

```text
core
  -> specifier entry object
      -> shared helper
```

---

## 7. sink / adapter 层的直接证据

这里再看两个重要对象：

### 7.1 `_printf_char_common.o`

符号表显示：

```text
GLOBAL _printf_char_common
UND    __printf
```

而且这个对象里还存在本地函数：

```text
_printf_input_char
```

这说明：

* `_printf_char_common` 会直接调用 `__printf`
* 它处在 core 的上一层或者旁路适配层

这个对象不是纯 formatter helper，更像输入/输出适配壳。

### 7.2 `_printf_char_file.o`

符号表显示：

```text
GLOBAL _printf_char_file
UND    _printf_char_common
UND    ferror
UND    fputc
```

这说明：

* `_printf_char_file` 不自己做格式解析
* 它调用 `_printf_char_common`
* 它把字符真正对接到 `FILE` 路径

所以这里也有非常明确的分层：

```text
__printf
    ^
    |
_printf_char_common
    ^
    |
_printf_char_file
```

这说明 Arm 的 `FILE` 输出链也是对象化拆开的。

---

## 8. 只看对象层，Arm 到底是不是“一个 core”

这个问题要分两层回答。

### 8.1 如果说“源码逻辑可以只有一份 core 模板”

这个结论对象层不能反驳。

因为对象层只告诉我们：

* 最终产物里有多个 core 变体

但对象层并不能证明：

* 这些变体在源码层是手写多份
  还是
* 来自同一个模板源码反复编译

所以如果你的表述是：

```text
Arm 可能源码层只有一份 core 逻辑模板
```

这个说法是成立的，至少对象层没有否定它。

### 8.2 如果说“最终对象层只有一个 core 函数”

这个说法不成立。

证据就是：

* 有八个不同的 core 相关对象
* 有八个不同导出符号
* 大小都不同

所以更准确的结论只能是：

```text
Arm 在对象层不是一个 core，
而是多个 core 变体。
```

---

## 9. 对我们设计的启发

这份调研对我们自己的设计有三个直接启发。

### 9.1 源码统一和对象分体并不冲突

从 Arm 的对象层看，完全可以接受这样的设计：

```text
源码层只有一份 printf_core 模板
编译层产出多份 printf_core_xxx
对象层继续和 specifier entry/helper 组合
```

这恰恰是一个合理的工程方向。

### 9.2 真正重要的是对象边界，而不是文件数量

Arm 最有价值的不是“它有几个 `.c` 文件”，而是：

* core 变体边界清楚
* specifier entry 对象很薄
* shared helper 独立
* sink/adaptor 独立

也就是说，我们后续要学的是这些对象边界，而不是简单抄文件命名。

### 9.3 用户接口和对象边界是两层问题

Arm 的对象层拆得很细，但这不等于用户接口就一定要暴露成同样粒度。

对我们来说，更合理的分工是：

* 对外可以保留 `printf / iprintf / printf_full / wprintf`
* 对内则把 core shell、specifier entry、helper 继续做成 Arm 风格的对象边界

---

## 10. 最终结论

最终结论压缩成三句话：

1. Arm `armlib` 在对象层不是“只有一个 core”，而是明确存在多份 `__printf*` core 变体。
2. Arm 的 specifier entry object 非常薄，并且分别依赖独立 shared helper。
3. Arm 的事实支持我们后续采用“单份 core 模板源码 + 多份 core 编译产物 + 薄 specifier entry + shared helper”的方向。

