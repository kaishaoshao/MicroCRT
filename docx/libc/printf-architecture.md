# `MicroCRT` `printf` 架构约束

本文档不是 review，也不是迁移记录。它只定义后续 `printf` 重构必须遵守的目标结构。

---

## 1. 设计原则

### 1.1 两条输出路线必须分开

`FILE` 输出和 `char buffer` 输出必须是两条不同的 adapter 路线：

* `printf/fprintf/vprintf/vfprintf`
  只属于 `stream` 路线
* `sprintf/snprintf/vsprintf/vsnprintf`
  只属于 `string` 路线

这两条路线都可以复用同一个 `printf_core`，但 `string` 路线不能通过 `vfprintf(FILE *, ...)` 伪装进入。

目标不是“所有入口都绕到 `vfprintf`”，而是：

* 每类输出介质先被转换成自己的 adapter
* adapter 再把输出抽象成统一的 `__printf_out`
* 最后统一调用 `__printf_core_*`

### 1.2 一个 core，多组 helper

`core` 层应该只有一个模板：

* `printf_core.inc`

`core` 负责：

* 驱动 parse loop
* 管理 runtime scratch
* 调用 parser helper
* 调用 conversion dispatcher
* 统一处理 `ret/fail/secure error`

`core` 不负责：

* `%d/%f/%s/%c` 的具体格式化算法
* `FILE` / string buffer 的具体写出策略

### 1.3 helper 必须是真边界，不是伪拆分

helper 的职责应该稳定且可复用：

* parser helper
* text helper
* integer helper
* float helper
* dispatch helper

helper 不能依赖“外层正好有某个局部变量或宏”才能工作。允许 core 为兼容提供少量参数和别名，但不能继续扩张成旧式隐式 include 模板。

### 1.4 `.inc` 只用于模板装配，不用于承载算法碎片

`.inc` 的目标用途应该收敛成两类：

* 变体配置
* 模板装配

具体算法应优先放在真实 `.c` helper 文件里。

也就是说：

* `printf_core.inc` 可以存在
* `printf_config.h` 应成为唯一配置入口
* 不应保留只做转发 include 的 `printf_support_*.inc`
* 不应再回到“每个 conversion 分支一个 `.inc`”的状态

---

## 2. 目标流程图

### 2.1 `FILE` 路线

```text
printf / fprintf / vprintf / vfprintf
                |
                v
      __printf_stream_api.h
                |
                v
   __printf_out_init_file(...)
                |
                v
       __printf_core_default
       __printf_core_integer
       __printf_core_full
```

说明：

* `vfprintf` 是 `FILE` 路线的公开入口
* `printf/vprintf/fprintf` 只是它的薄 wrapper
* 这条线上不出现 string buffer 适配逻辑

### 2.2 `char buffer` 路线

```text
sprintf / snprintf / vsprintf / vsnprintf
                  |
                  v
      __printf_string_api.h
                  |
                  v
   __printf_out_init_cstr(...)
                  |
                  v
       __printf_core_default
       __printf_core_integer
       __printf_core_full
```

说明：

* `vsnprintf` 不是 `vfprintf`
* `string` 路线直接构造 memory-backed `__printf_out`
* 路线分开，但 core 复用

### 2.3 统一 core 内部

```text
printf_core.inc
    |
    +-- __printf_scan_literal(...)
    +-- __printf_parse_*()
    +-- __printf_dispatch_conversion(...)
    +-- __printf_emit_*()
```

---

## 3. 文件分层

### 3.1 对外 wrapper

位于 `src/libc/stdio/`：

* `printf.c`
* `vprintf.c`
* `fprintf.c`
* `vfprintf.c`
* `sprintf.c`
* `snprintf.c`
* `iprintf.c`
* `printf_full.c`

规则：

* wrapper 只做 `va_start/va_end`、公开 API 适配、选择 adapter
* wrapper 不直接碰 parser/formatter 细节

### 3.2 adapter 层

位于 `src/libc/stdio/printf/`：

* `__printf_stream_api.h`
* `__printf_string_api.h`
* `printf_out.h`

规则：

* `stream` adapter 只做 `FILE -> __printf_out`
* `string` adapter 只做 `char buffer -> __printf_out`
* adapter 是路线分隔带，不能互相绕行

### 3.3 core 层

位于 `src/libc/stdio/printf/`：

* `printf_core_default.c`
* `printf_core_integer.c`
* `printf_core_full.c`
* `printf_core_impl.inc`
* `printf_core.inc`

规则：

* `.c` 壳文件只负责编译一个 profile
* `printf_core_impl.inc` 只负责装配
* `printf_core.inc` 只负责共享主流程

### 3.4 helper 层

位于 `src/libc/stdio/printf/`：

* parser:
  `_printf_parser.c`
  `_printf_positional.c`
* text:
  `_printf_text_core.c`
  `_printf_char_core.c`
  `_printf_string_core.c`
  `_printf_c.c`
  `_printf_s.c`
  `_printf_n.c`
* integer:
  `_printf_d.c`
  `_printf_u.c`
  `_printf_o.c`
  `_printf_x.c`
  `_printf_p.c`
  `_printf_int_dec.c`
  `_printf_int_base.c`
  `_printf_pointer.c`
* float:
  `_printf_f.c`
  `_printf_e.c`
  `_printf_g.c`
  `_printf_a.c`
  `_printf_fp_dec.c`
  `_printf_fp_hex.c`
* dispatch:
  `_printf_conversion_dispatch.c`

规则：

* `_printf_*.c` 文件应是可命名、可说明、可复用的 helper
* 如果文件本身无法一句话说明职责，说明拆分方式有问题

### 3.5 config 与装配层

位于 `src/libc/stdio/printf/`：

* `printf_config.h`
* `printf_core_impl.inc`

规则：

* `printf_config.h` 统一承接 profile、capability、runtime flag、arg extraction
* `printf_core_impl.inc` 只允许做固定顺序的 helper 装配
* 不应再保留只做 include 转发的中间 support 文件

### 3.6 Arm 参考后的目标对象层

如果参考 Arm `armlib` 的对象层，而不是只参考它的源码表象，建议把目标拆分理解成三层：

1. core shell 变体
2. 薄 specifier entry
3. shared helper / sink

也就是说，应该模仿的是：

```text
多份 core 变体
+ 薄 entry 对象
+ 可复用 helper
```

不应该模仿的是：

```text
为了长得像 Arm，先恢复很多空壳 entry 文件
```

对当前仓库，建议目标对象层如下：

* core shell:
  `printf_core_default.c`
  `printf_core_integer.c`
  `printf_core_full.c`
* thin entry:
  `_printf_c.c`
  `_printf_s.c`
  `_printf_n.c`
  `_printf_d.c`
  `_printf_i.c`
  `_printf_u.c`
  `_printf_x.c`
  `_printf_o.c`
  `_printf_p.c`
  `_printf_f.c`
  `_printf_e.c`
  `_printf_g.c`
  `_printf_a.c`
* shared helper:
  `_printf_char_core.c`
  `_printf_string_core.c`
  `_printf_int_dec.c`
  `_printf_int_base.c`
  `_printf_pointer.c`
  `_printf_fp_dec.c`
  `_printf_fp_hex.c`
* parser / dispatch:
  `_printf_parser.c`
  `_printf_positional.c`
  `_printf_conversion_dispatch.c`

当前实现已经按这个方向走到下一步了：

* integer family 已拆成 `_printf_d/_u/_o/_x/_p`
* float family 已拆成 `_printf_f/_e/_g/_a`
* `conversion_dispatch` 直接选择 per-specifier entry

后续如果继续向 Arm 靠拢，重点就不再是“拆掉 family dispatcher”，而是补齐仍缺的 `%i` / `%%` 这类最薄 entry，以及进一步考虑对象级编译边界。

---

## 4. 函数命名规则

### 4.1 public API

对外接口保持标准库名称：

* `printf`
* `vfprintf`
* `snprintf`
* `vsnprintf`

### 4.2 adapter API

adapter 统一使用：

* `__printf_vformat_stream_*`
* `__printf_vformat_cstr*`

后续如果再规范命名，可以把 `cstr` 系列进一步统一成 `string`/`buffer` 词汇，但原则不变：
函数名必须直接表达“它属于哪条路线”。

### 4.3 core API

core 统一使用：

* `__printf_core_default`
* `__printf_core_integer`
* `__printf_core_full`

### 4.4 helper API

helper 命名应体现职责：

* parser:
  `__printf_parse_*`
* dispatch:
  `__printf_dispatch_*`
* format:
  `__printf_format_*`
* emit:
  `__printf_emit_*`

不建议继续增加语义含糊的名字，例如只有单字母 specifier 感的公共 helper 名称。

---

## 5. 宏设计规则

当前宏需要重新分层，而不是继续混用。

### 5.1 profile 选择宏

这一层只回答“编译哪一个 core profile”：

* 当前实现：
  `PRINTF_CORE_PROFILE`
  `PRINTF_CORE_SYMBOL`
* 兼容旧名：
  `PRINTF_VARIANT`
  `PRINTF_NAME`

### 5.2 capability 宏

这一层只回答“这个 profile 打开了哪些能力”：

* 当前实现：
  `PRINTF_CAP_WCHAR`
  `PRINTF_CAP_DOUBLE`
  `PRINTF_CAP_POSITIONAL`
* 兼容旧名：
  `_NEED_IO_WCHAR`
  `_NEED_IO_DOUBLE`
  `_NEED_IO_POS_ARGS`

### 5.3 parser/format flag 宏

这一层只表示 parse descriptor 里的运行时状态：

* 当前实现：
  `PRINTF_FLAG_ZERO_FILL`
  `PRINTF_FLAG_PRECISION`
  `PRINTF_FLAG_LONG`
* 兼容旧名：
  `FL_ZFILL`
  `FL_PREC`
  `FL_LONG`

### 5.4 宏规则

必须满足：

* 宏名前缀能直接说明层级
* capability 宏不能和 runtime flag 宏混名
* profile 宏不能混进 parser helper
* 新名字落地时，优先先建兼容映射，再逐步替换正文
* capability 宏应尽量采用统一的 `0/1 table`，避免“有的 define、有的不 define”
* 兼容旧名只能用于过渡，不应再作为新代码入口

---

## 6. 当前实现和目标设计的关系

当前代码已经满足的点：

* `FILE` 路线和 `string` 路线已经在 adapter 层分开
* 两条路线都复用同一个 `printf_core`
* `core` 已收敛成单一 `printf_core.inc`
* parser / dispatch / text / integer / float 已经下沉成 helper 组

当前仍偏离目标的点：

* `support` 层 `.inc` 仍然偏多
* 兼容别名仍然存在，尚未完全移除旧 `PRINTF_* / _NEED_IO_* / FL_*` 历史名字
* 但主路径已经开始以 `PRINTF_CORE_* / PRINTF_CAP_* / PRINTF_FLAG_*` 为准
* 一些 `_printf_*.c` 仍然带过强的“specifier 文件名”色彩，而不是职责文件名

---

## 7. 后续整理顺序

建议按这个顺序推进：

1. 固定本文档，不再反复改设计目标
2. 让 `README.md` 只描述当前实现，不再夹带目标设计
3. 先规范宏分层，再做大规模改名
4. 收敛 `support` 装配层，让 `.inc` 数量继续下降
5. 最后再决定是否把 `_printf_*.c` 中的一部分重命名为更职责化的名字
