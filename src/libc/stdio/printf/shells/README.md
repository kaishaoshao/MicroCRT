# `shells/`

这一层预留给 `printf_core` 变体壳。

目标是做到：

* 一个 shell 只负责选择 `PRINTF_VARIANT`
* 一个 shell 只负责导出一个 `PRINTF_NAME`
* 不在 shell 里重新塞 parser / dispatch / conversion 逻辑

计划中的壳：

* `vfprintf_default.c`
* `vfprintf_integer.c`
* `vfprintf_long_long.c`
* `vfprintf_double.c`
