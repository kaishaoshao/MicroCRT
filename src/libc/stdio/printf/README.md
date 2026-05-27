# `MicroCRT` `printf` 当前实现说明

本文档描述截至 `2026-05-25` 已经落地的 `printf` 内部结构，只回答“现在代码怎么组织、主链怎么走、各层职责是什么”。

## 目录边界

`src/libc/stdio/`

* 对外 wrapper：
  `printf.c` `vprintf.c` `fprintf.c` `vfprintf.c`
  `sprintf.c` `snprintf.c`
  `iprintf.c`
  `printf_full.c`
* stream/backend：
  `fileops.c` `iob.c`

`src/libc/stdio/printf/`

* core 壳：
  `printf_core_default.c`
  `printf_core_integer.c`
  `printf_core_full.c`
* core 模板：
  `printf_core_impl.inc`
  `printf_core.inc`
* config/assembly：
  `printf_config.h`
* bridge/out：
  `printf_bridge.h`
  `printf_out.h`
  `__printf_stream_api.h`
  `__printf_string_api.h`
* helper/entry：
  `_printf_parser.c`
  `_printf_positional.c`
  `_printf_text_core.c`
  `_printf_c.c`
  `_printf_s.c`
  `_printf_n.c`
  `_printf_d.c`
  `_printf_u.c`
  `_printf_o.c`
  `_printf_x.c`
  `_printf_p.c`
  `_printf_f.c`
  `_printf_e.c`
  `_printf_g.c`
  `_printf_a.c`
  `_printf_conversion_dispatch.c`
  `_printf_int_dec.c`
  `_printf_int_base.c`
  `_printf_pointer.c`
  `_printf_fp_dec.c`
  `_printf_fp_hex.c`

## 主链

默认 `printf` 主链：

```text
printf
  -> vprintf
  -> vfprintf
  -> __printf_vformat_stream
  -> __printf_core_default
  -> printf_core.inc
```

`fprintf` 主链：

```text
fprintf
  -> vfprintf
  -> __printf_vformat_stream
  -> __printf_core_default
```

`sprintf/snprintf` 主链：

```text
sprintf/snprintf
  -> __printf_vformat_cstr/__printf_vformat_cstrn
  -> __printf_core_default
```

这里要特别强调：

* `printf/fprintf/vprintf/vfprintf` 属于 `FILE` 路线
* `sprintf/snprintf/vsprintf/vsnprintf` 属于 `char buffer` 路线
* 两条路线共享 `printf_core`，但 `string` 路线不经过 `vfprintf`

整数版和 full 版只是在 wrapper/bridge 后切到不同 core 壳：

```text
iprintf family -> __printf_core_integer
printf_full family -> __printf_core_full
```

## 细化架构图

### 1. 对外入口到 core

```text
FILE 路线
  printf
    -> vprintf
    -> vfprintf
    -> __printf_vformat_stream
    -> __printf_core_default

  fprintf
    -> vfprintf
    -> __printf_vformat_stream
    -> __printf_core_default

字符串路线
  snprintf
    -> __printf_vformat_cstrn
    -> __printf_core_default

  sprintf
    -> __printf_vformat_cstr
    -> __printf_core_default

profile 路线
  iprintf family
    -> __printf_core_integer

  printf_full family
    -> __printf_core_full
```

### 2. core 内部主循环

```text
__printf_core_*
  -> __printf_validate_core_inputs
  -> __printf_out_begin
  -> __printf_scan_literal
  -> __printf_spec_reset
  -> __printf_parse_flag_char
  -> __printf_parse_number_char
  -> __printf_parse_dynamic_field
  -> __printf_parse_precision_marker
  -> __printf_parse_positional_char
  -> __printf_parse_size_modifier
  -> __printf_spec_normalize
  -> __printf_dispatch_conversion
  -> __printf_emit_width_tail
  -> __printf_out_finish / __printf_out_finish_failed
```

### 3. conversion dispatcher 到 family

```text
__printf_dispatch_conversion
  -> float family
     -> __printf_dispatch_float_conversion
        -> __printf_entry_float_f
        -> __printf_entry_float_e
        -> __printf_entry_float_g
        -> __printf_entry_float_a

  -> text family
     -> __printf_dispatch_char
        -> __printf_format_char
     -> __printf_dispatch_string
        -> __printf_format_string
     -> __printf_dispatch_percent_n

  -> integer family
     -> __printf_dispatch_integer_conversion
        -> __printf_entry_signed_decimal
        -> __printf_entry_unsigned_decimal
        -> __printf_entry_unsigned_octal
        -> __printf_entry_unsigned_hex
        -> __printf_entry_pointer
        -> __printf_entry_unsigned_binary
```

### 4. family 内部层次

```text
text family
  _printf_c.c
    -> _printf_char_core.c
       -> _printf_text_core.c

  _printf_s.c
    -> _printf_string_core.c
       -> _printf_text_core.c

  _printf_n.c
    -> %n store helper

integer family
  _printf_d.c
    -> _printf_int_dec.c

  _printf_u.c
  _printf_o.c
  _printf_x.c
  _printf_b.c
    -> _printf_int_base.c

  _printf_p.c
    -> _printf_pointer.c

float family
  _printf_f.c
  _printf_e.c
  _printf_g.c
    -> _printf_fp_dec.c
       -> dtoa.h
       -> dtoa_engine.c / ldtoa_engine.c

  _printf_a.c
    -> _printf_fp_hex.c
       -> dtox_engine.c / ldtox_engine.c
```

### 5. 装配层到底在做什么

```text
printf_core_default.c / integer.c / full.c
  -> include printf_core_impl.inc

printf_core_impl.inc
  -> include printf_config.h
  -> include helper graph
     -> shared primitive
     -> parser/text family
     -> integer family
     -> float family
     -> final dispatcher
  -> include printf_core.inc
```

最直白地说：

* wrapper 选 FILE 还是字符串路线
* adapter 把输出目标变成统一的 `__printf_out`
* core 只负责“扫描、解析、调度、统一收尾”
* 真正的格式化算法在各 family helper 里

### 6. 当前 `FILE` 模型 vs 后续统一 backend 模型

当前 `FILE` 相关结构还是保守模式：

```text
FILE
  -> put(char)
  -> get()
  -> flush()

printf_out
  -> put(int)
  -> write(buf,len)
  -> flush()
  -> finalize()
```

也就是说，当前状态是：

* `printf` 后端抽象已经比 `FILE` 更强
* `FILE` 自身还没升级
* FILE-backed `write` 目前通过 `put` 循环兜底

对应关系可以画成：

```text
当前模型

FILE
  put/get/flush
      |
      v
__printf_out_init_file
      |
      +-> put      直接映射 FILE.put
      +-> write    目前是 put 循环 fallback
      +-> finalize 目前按 backend policy 决定
```

如果后续要做更像 emRun 的统一 backend，建议目标是：

```text
统一 backend 模型

FILE
  -> putc
  -> write
  -> flush
  -> finalize
  -> optional strategy / buffering state

stdio API
  -> putc/fwrite/fputs/printf
  -> 共用同一套 backend 能力
```

两者差别是：

* 当前模型：
  先升级 `printf` backend，不强制改 `FILE` ABI
* 目标模型：
  `FILE` 和 `printf_out` 共用同一套 backend 语义

所以现阶段结论是：

* 现在不需要修改 `FILE` 结构体就能支持块写 backend
* 但如果以后要让 `printf/fwrite/fputs` 真正共用统一后端能力，`FILE` 最终仍应升级

## 现在这套架构在做什么

如果只用一句话概括，当前实现是在把历史 `vfprintf` 大函数拆成四层：

```text
wrapper
  -> adapter
  -> core loop
  -> family helper / specifier entry
```

具体对应关系是：

* wrapper:
  `printf/fprintf/snprintf/...`
* adapter:
  `__printf_stream_api.h`
  `__printf_string_api.h`
* core loop:
  `printf_core.inc`
* family helper / specifier entry:
  `_printf_parser.c`
  `_printf_* entry`
  `_printf_*_core.c`
  `_printf_* backend`
  `_printf_conversion_dispatch.c`

也就是说，我这段时间在做的不是“零碎改文件名”，而是在把旧代码里的三类混杂职责拆开：

* API 路线选择
* core 主循环
* specifier 具体格式化

现在代码还没完全到终态，但主方向已经固定：

* `printf_core.inc`
  只保留主循环
* `_printf_conversion_dispatch.c`
  只保留 conversion 路由
* `_printf_d/_u/_f/_g/...`
  只保留薄 specifier entry
* `_printf_int_dec/_printf_fp_dec/_printf_text_core/...`
  保留真正可复用的 family helper

## 阶段职责

### `printf_core.inc`

负责：

* 组织整个 core 函数骨架
* 初始化局部解析状态和 runtime scratch
* 驱动 output begin、parser、dispatch、tail、统一收尾

不负责：

* 具体 specifier 算法
* 具体 parser 子步骤

核心内部阶段现在作为单模板中的顺序代码存在：

* literal scan 和 parse flow 由 `_printf_parser.c` 的 helper 驱动
* conversion dispatch 统一走 `_printf_conversion_dispatch.c` 的 `__printf_dispatch_conversion(...)`
* `ret/fail/secure error` 在模板底部统一收口

## config 和装配

`printf_core_impl.inc` 现在只负责两件事：

1. include `printf_config.h`
2. 以固定顺序直接装配 helper `.c`

配置入口已经集中到 `printf_config.h`：

* `PRINTF_CORE_PROFILE` / `PRINTF_CORE_SYMBOL`
  是 profile 选择主入口
* `PRINTF_CAP_*`
  是 capability 主入口，包括 `PRINTF_CAP_SECURE` 和 `PRINTF_CAP_PERCENT_N`
* `PRINTF_FLAG_*`
  是 runtime flag 主入口
* parser 局部类型和 size-modifier 规则
  已经并回 `printf_core_private.h`，不再单独保留 parser-private header

配置优先级现在固定为：

```text
built-in default
  < profile default
  < explicit PRINTF_CAP_* override
```

也就是说：

* `PRINTF_CORE_PROFILE`
  只是常用预设模板
* `PRINTF_CAP_*`
  才是最终构建真值

因此后续要新增一个快速裁剪变体时，允许直接这样做：

```c
#define PRINTF_CORE_PROFILE   __IO_VARIANT_INTEGER
#define PRINTF_CORE_SYMBOL    __printf_core_iprintf_noll
#define PRINTF_CAP_LONG_LONG  0
#include "printf_core_impl.inc"
```

helper 装配不再经过 `printf_support_*.inc` 中间层，而是直接按顺序并入：

* parser/text：
  `_printf_positional.c`
  `_printf_text_core.c`
  `_printf_parser.c`
  `_printf_c.c`
  `_printf_s.c`
  `_printf_n.c`
* integer：
  `_printf_d.c`
  `_printf_u.c`
  `_printf_o.c`
  `_printf_x.c`
  `_printf_p.c`
  `_printf_int_dec.c`
  `_printf_int_base.c`
  `_printf_pointer.c`
* float：
  `_printf_f.c`
  `_printf_e.c`
  `_printf_g.c`
  `_printf_a.c`
  `_printf_fp_dec.c`
  `_printf_fp_hex.c`
* dispatch：
  `_printf_conversion_dispatch.c`

## 当前实现收敛点

和最开始的过渡版本相比，现在已经明确收敛了这些点：

* wrapper 主链已经统一到
  `printf -> vprintf -> vfprintf -> core`
* core 层已经从多份 `printf_core_*.inc` 收敛成单一 `printf_core.inc`
* parser 主循环里的 flags/数字/`*`/`.`/size/positional 状态迁移已经收进 `_printf_parser.c`
* `char/string/%n`、整数族、浮点族都已经变成显式 helper 调用

## 仍然保留的现实约束

当前实现仍然是“单模板 + include 装配”的组织，不是对象级独立编译裁剪方案。

这意味着：

* 边界已经比旧实现清楚很多
* 但 helper 仍然是源码级装配
* 后续如果要继续向更细粒度对象边界收敛，应优先从 helper 对象化入手
