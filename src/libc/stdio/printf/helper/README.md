# `helper/`

这一层预留给 conversion 共享 helper。

推荐拆法：

* `_printf_int_dec.c`
* `_printf_int_base.c`
* `_printf_pointer.c`
* `_printf_fp_dec.c`
* `_printf_fp_hex.c`

helper 层不直接暴露 public API，也不直接决定 `PRINTF_VARIANT`。
它只负责被 entry 层复用。
