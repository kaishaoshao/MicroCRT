# `printf` 后续文件框架

这个文件只描述后续要落代码的框架，不代表这些文件现在已经接入构建。

当前策略：

* 现有实现继续保留在顶层 `printf/` 目录
* 未来拆分目标先把目录和文件骨架摆出来
* 等框架稳定后，再逐步把顶层 `inc` 逻辑迁过去

## 目标目录

```text
printf/
  api/
  shells/
  core/
  entry/
  helper/
```

## 当前文件到目标文件的迁移映射

```text
顶层 printf.c            -> api/printf.c
顶层 fprintf.c           -> api/fprintf.c
顶层 vprintf.c           -> api/vprintf.c
顶层 sprintf.c           -> api/sprintf.c
顶层 snprintf.c          -> api/snprintf.c
顶层 iprintf.c           -> api/iprintf.c

顶层 vfprintf.c          -> shells/vfprintf_default.c
顶层 vfprintf_integer.c  -> shells/vfprintf_integer.c

顶层 vfprintf_variant_config.inc -> core/vfprintf_variant_config.inc
顶层 vfprintf_support.inc        -> core/vfprintf_support.inc
顶层 printf_core_body.inc        -> core/printf_core_body.inc
顶层 printf_core_parser.inc      -> core/printf_core_parser.inc
顶层 printf_core_dispatch.inc    -> core/printf_core_dispatch.inc

顶层 _printf_char.inc     -> entry/_printf_c.c
顶层 _printf_str.inc      -> entry/_printf_s.c
顶层 _printf_n.inc        -> entry/_printf_n.c
顶层 _printf_int.inc      -> 先拆到 entry/_printf_d|i|u|o|x|p.c
顶层 _printf_float.inc    -> 先拆到 entry/_printf_f|e|g|a.c

顶层 ultoa_invert.c       -> helper/_printf_int_dec.c 或 helper/_printf_int_base.c 配套位置
顶层 dtoa/dtox engines    -> 暂时仍留顶层，后续再决定是否迁入 helper/
```

## 推荐迁移顺序

1. `core/`
   先迁共享层，风险最低
2. `shells/`
   再把默认/整数壳指到新 `core/`
3. `api/`
   再迁 wrapper
4. `entry/`
   最后按 specifier 拆 conversion
5. `helper/`
   在 entry 稳定后，再细化 helper 边界

## 当前进度

当前已经完成的真实迁移：

* `core/`
  已经承载真实 shared core 文件
* `shells/`
  已经承载真实 default / integer shell
* `api/`
  已经承载真实 public wrapper

当前仍然处于兼容壳阶段的层：

* 顶层 `vfprintf*.c`
  现在只是指向 `shells/` 的兼容入口
* 顶层 `printf_core_*` / `vfprintf_*support*`
  现在只是指向 `core/` 的兼容入口
* 顶层 `printf/fprintf/vprintf/sprintf/snprintf/iprintf`
  现在只是指向 `api/` 的兼容入口

下一步应当轮到：

* `entry/`

## 各层职责

* `api/`
  public wrapper
  例如 `printf/fprintf/sprintf/iprintf`
* `shells/`
  `printf_core` 变体壳
  例如 default/integer/long-long/double
* `core/`
  shared core 主体
  例如 variant config / support / body / parser / dispatch
* `entry/`
  conversion 入口层
  例如 `%c/%s/%d/%x/%f`
* `helper/`
  conversion 共享 helper
  例如 integer decimal / integer base / float decimal / float hex

## 现在先不做的事

* 这些文件当前不默认接入构建
* 不在这一步强行把现有顶层实现全部迁过去
* 不在这一步改 public 行为
