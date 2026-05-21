# `entry/`

这一层预留给 conversion 入口文件。

推荐拆法：

* `_printf_c.c`
* `_printf_s.c`
* `_printf_n.c`
* `_printf_d.c`
* `_printf_i.c`
* `_printf_u.c`
* `_printf_o.c`
* `_printf_x.c`
* `_printf_p.c`
* `_printf_f.c`
* `_printf_e.c`
* `_printf_g.c`
* `_printf_a.c`

入口层应该尽量薄，只负责：

* 对应 specifier 的语义入口
* 调用 helper
* 少量 specifier 专属规则
