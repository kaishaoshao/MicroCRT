# `MicroCRT` `printf` 实现说明

这套实现现在只保留两层目录：

```text
src/libc/stdio/
  printf.c fprintf.c vprintf.c vfprintf.c sprintf.c snprintf.c iprintf.c
  fileops.c iob.c
  printf/
```

约束是：

* C 标准入口函数放在 `src/libc/stdio/`
* `printf/` 目录只放内部实现
* 不再使用 `api/core/entry/helper/shells` 这类子目录

## 当前文件职责

### `src/libc/stdio/`

这一层只放公开入口：

* `printf.c`
* `fprintf.c`
* `vprintf.c`
* `vfprintf.c`
* `sprintf.c`
* `snprintf.c`
* `iprintf.c`
* `fileops.c`
* `iob.c`

它们都很薄，只负责：

* 处理 `va_list`
* 选择 `stdout` / `FILE *`
* 或构造 `char buffer` 输出
* 调用内部 `printf core`

### `src/libc/stdio/printf/`

这一层是内部实现，按职责平铺：

* 输出与 bridge
  * `printf_out.h`
  * `printf_bridge.h`
  * `__printf_stream_api.h`
  * `__printf_string_api.h`
* core 变体壳
  * `vfprintf_default.c`
  * `vfprintf_integer.c`
* core 模板
  * `vfprintf_impl.inc`
  * `vfprintf_variant_config.inc`
  * `vfprintf_support.inc`
  * `printf_core_body.inc`
  * `printf_core_parser.inc`
  * `printf_core_dispatch.inc`
  * `printf_core_dispatch_char.inc`
  * `printf_core_dispatch_string.inc`
  * `printf_core_dispatch_percent_n.inc`
  * `printf_core_dispatch_integer.inc`
  * `printf_core_dispatch_float.inc`
  * `printf_core_stream.inc`
  * `printf_core_epilogue.inc`
* core 私有头
  * `printf_core_private.h`
  * `printf_float_private.h`
  * `printf_float_support.h`
* conversion 与 helper
  * `_printf_c.c`
  * `_printf_s.c`
  * `_printf_n.c`
  * `_printf_int.c`
  * `_printf_d.c`
  * `_printf_i.c`
  * `_printf_u.c`
  * `_printf_o.c`
  * `_printf_x.c`
  * `_printf_p.c`
  * `_printf_float.c`
  * `_printf_f.c`
  * `_printf_e.c`
  * `_printf_g.c`
  * `_printf_a.c`
  * `_printf_text.c`
  * `_printf_parser.c`
  * `_printf_int_dec.c`
  * `_printf_int_base.c`
  * `_printf_pointer.c`
  * `_printf_fp_dec.c`
  * `_printf_fp_hex.c`
* 浮点转换引擎
  * `dtoa.h`
  * `dtoa_engine.c`
  * `dtox_engine.c`
  * `ldtoa_engine.c`
  * `ldtox_engine.c`
  * `matchcaseprefix.c`
  * `ultoa_invert.c`

## 当前执行链

默认 `printf` 路径：

```text
printf.c / fprintf.c / vprintf.c / vfprintf.c
  -> __printf_stream_api.h
  -> __printf_core_default
  -> vfprintf_impl.inc
  -> vfprintf_variant_config.inc
  -> vfprintf_support.inc
  -> printf_core_body.inc
  -> printf_core_parser.inc
  -> printf_core_dispatch.inc
  -> _printf_c / _printf_s / _printf_n / _printf_int / _printf_float
```

`sprintf/snprintf` 路径：

```text
sprintf.c / snprintf.c
  -> __printf_string_api.h
  -> __printf_core_default
  -> 同一条 shared core 链
```

`iprintf` 路径：

```text
iprintf.c
  -> __printf_stream_api.h / __printf_string_api.h
  -> __printf_core_integer
  -> 同一条 shared core 链
```

## 当前关键设计

### `FILE` 和 `char buffer` 已经分开

shared core 不再吃 `FILE *`，而是吃 `struct __printf_out *`。

这意味着：

* `fprintf/vfprintf/printf` 走 file-backed output
* `sprintf/snprintf` 走 cstr-backed output
* 两条路径在进入 core 前就已经分开

### `fflush` 支持保留

* 显式 `fflush(FILE *)` 仍然是 stream/backend 行为
* `printf` 返回前是否自动 flush 由 `MICROCRT_PRINTF_ENABLE_AUTO_FLUSH` 控制
* 默认关闭

### `printf core` 仍然保持“单份模板 + 多个壳”

当前已经有两份壳：

* `__printf_core_default`
* `__printf_core_integer`

差异主要通过 `vfprintf_variant_config.inc` 表达，不复制整份 core。

## 当前构建输入

`MicroCRT/src/libc/stdio/CMakeLists.txt` 直接编译：

* `src/libc/stdio/*.c` 里的公开入口
* `src/libc/stdio/fileops.c`
* `src/libc/stdio/iob.c`
* `src/libc/stdio/printf/vfprintf_default.c`
* `src/libc/stdio/printf/vfprintf_integer.c`
* `src/libc/stdio/printf` 下的 dtoa/辅助实现

## 现在不再使用的旧结构

下面这些层级已经废弃，不应该再恢复：

* `printf/api/`
* `printf/core/`
* `printf/entry/`
* `printf/helper/`
* `printf/shells/`

如果后面继续拆逻辑，也只在当前这个单一 `printf/` 目录里处理。
