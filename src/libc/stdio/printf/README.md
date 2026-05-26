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
* 旧 `PRINTF_VARIANT` / `PRINTF_NAME` / `_NEED_IO_*` / `FL_*`
  现在只应视为兼容层，不应再新增使用
* parser 局部类型和 size-modifier 规则
  已经并回 `printf_core_private.h`，不再单独保留 parser-private header

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
