# `MicroCRT` `printf` 目录说明

这个目录已经不是“准备重构的占位区”，而是 `MicroCRT` 当前真实使用的 `printf` 实现。

## 当前目标

当前目标不是直接做成 Arm 成品，而是先把框架收敛成：

```text
单份 shared printf core 逻辑
+ 多份 variant shell
+ 薄 public API wrapper
+ 独立 stream / memory bridge
+ 后续可继续向 object split 演进
```

## 当前实际流程

### 1. public API 层

默认族：

* `printf.c`
* `fprintf.c`
* `vprintf.c`
* `sprintf.c`
* `snprintf.c`

整数族：

* `iprintf.c`
  同时导出：
  `iprintf / viprintf / fiprintf / vfiprintf / siprintf / vsiprintf`

这些 wrapper 都很薄，只负责：

* `va_start / va_end`
* 选择 `stdout` / `FILE *stream`
* 或构造 memory-backed `FILE`
* 然后转到某个 `vfprintf` 变体

### 2. bridge 层

`printf_bridge.h` 提供：

* `struct __file_str`
* `FDEV_SETUP_STRING_WRITE`
* `__flockfile / __funlockfile`
* string-backed `FILE` bridge 声明

这里的关键结论是：

* `sprintf/snprintf` 不是独立 formatter
* 它们通过 memory-backed `FILE` 复用同一条 `vfprintf` core 链路

### 3. shell 层

当前已经有两份 `vfprintf` shell：

* `vfprintf.c`
  默认 shell
  `PRINTF_VARIANT = __IO_VARIANT_DOUBLE`
  `PRINTF_NAME = __d_vfprintf`
* `vfprintf_integer.c`
  整数 shell
  `PRINTF_VARIANT = __IO_VARIANT_INTEGER`
  `PRINTF_NAME = __i_vfprintf`

注意：

* 这两份文件本身不承载 formatter 逻辑
* 它们只是给 shared core 选择 variant

### 4. shared implementation 拼装层

`vfprintf_impl.inc` 现在只负责把 shared core 需要的几层拼起来：

```text
vfprintf_impl.inc
    -> vfprintf_variant_config.inc
    -> vfprintf_support.inc
    -> printf_core_body.inc
```

这是当前最关键的结构收敛点。

### 5. variant 配置层

`vfprintf_variant_config.inc` 负责：

* `PRINTF_VARIANT / PRINTF_NAME`
* `CHAR / UCHAR`
* `_NEED_IO_*`
* `SKIP_FLOAT_ARG`
* `arg_to_signed / arg_to_unsigned`
* `PRINTF_BUF_SIZE`

这一层表达的是：

```text
这个 shell 到底要编译成哪一种 printf_core 变体
```

### 6. support 层

`vfprintf_support.inc` 负责：

* `ultoa_invert.c` 接入
* `FL_*` flags 定义
* `CHECK_INT_SIZES`
* `skip_to_arg`
* `_mbslen / _wcslen`

这一层是 shared core 的支持代码，但不直接等于主循环。

### 7. core 主体层

`printf_core_body.inc` 是真正的 `vfprintf` 主函数骨架。

它内部再调用：

* `printf_core_parser.inc`
* `printf_core_dispatch.inc`

职责划分是：

* `printf_core_body.inc`
  主循环、局部状态、输出计数、错误收口
* `printf_core_parser.inc`
  `%...` 的解析逻辑
* `printf_core_dispatch.inc`
  conversion specifier 分发

### 8. conversion 层

当前 conversion 还是以 `inc` 形式存在：

* `_printf_char.inc`
* `_printf_str.inc`
* `_printf_n.inc`
* `_printf_int.inc`
* `_printf_float.inc`

这说明：

* 语义已经迁到 `printf/` 目录
* 但对象边界还没有完全细化成 Arm 风格 `_printf_d.o/_printf_f.o/...`

### 9. float support 层

当前浮点相关又单独分成：

* `printf_float_support.h`
* `printf_float_private.h`
* `dtoa.h`
* `dtoa_engine.c`
* `dtox_engine.c`
* `ldtoa_engine.c`
* `ldtox_engine.c`
* `matchcaseprefix.c`

这说明当前框架已经做到：

* core 私有层
* float 私有层

不再混成一个大内部头。

## 当前目录分层图

```text
public wrappers
  -> printf_bridge.h
  -> __d_vfprintf / __i_vfprintf shells
  -> vfprintf_impl.inc
      -> vfprintf_variant_config.inc
      -> vfprintf_support.inc
      -> printf_core_body.inc
          -> printf_core_parser.inc
          -> printf_core_dispatch.inc
              -> _printf_char/_str/_n/_int/_float
```

## 按执行顺序看一遍

如果只想顺着源码看一次默认 `printf("x=%d\n", 3)` 的执行路径，可以按下面顺序：

1. `printf.c`
   建 `va_list`，转到 `vfprintf(stdout, ...)`
2. `vfprintf.c`
   这是默认 shell，只定义默认 variant，然后包含 shared 实现
3. `vfprintf_impl.inc`
   负责拼装 shared 实现层次
4. `vfprintf_variant_config.inc`
   决定当前是默认 double 变体，打开哪些 `_NEED_IO_*`
5. `vfprintf_support.inc`
   提供 flags、size-spec、positional arg 等支持逻辑
6. `printf_core_body.inc`
   进入真正的 `vfprintf` 主函数和主循环
7. `printf_core_parser.inc`
   扫到 `%d`，解析 flags / width / precision / size
8. `printf_core_dispatch.inc`
   看到 `%d`，落到整数路径
9. `_printf_int.inc`
   取出参数、做整数格式化、写回输出
10. `printf_core_body.inc`
   统一处理尾部填充、错误返回、解锁和返回长度

`sprintf/snprintf` 唯一不同的地方在第 1 步：

* 它们先用 `printf_bridge.h` 里的 memory-backed `FILE`
* 然后仍然走同一条 `vfprintf` 主链

## conversion 层怎么看

如果你接下来要读 conversion 细节，可以按下面理解：

* `_printf_char.inc`
  `%c / %lc`
  负责单字符取参，并复用字符串式 padding 路径
* `_printf_str.inc`
  `%s / %ls`
  负责字符串长度、null 处理、precision 和宽窄字符输出
* `_printf_n.inc`
  `%n`
  只负责把当前输出计数回写到目标指针
* `_printf_int.inc`
  `%d / %i / %u / %o / %x / %X / %p`
  负责整数族和指针族的统一格式化
* `_printf_float.inc`
  `%f / %e / %g / %a`
  负责浮点参数抽取、dtoa/dtox engine 选择、宽度精度和特殊值处理

这几份现在还是 `inc`，说明：

* conversion 语义已经独立分块
* 但对象边界还没最终细化

所以当前阅读方式应该是先把它们看成“conversion 逻辑块”，而不是最终的 object split 形态。

## 后续文件框架

为了先把框架搭起来，目录下已经补了后续目标骨架：

* `framework.md`
* `api/`
* `shells/`
* `core/`
* `entry/`
* `helper/`

这批文件当前的定位是：

* 固定后续落代码的位置
* 让后面的分割不再边做边起名字
* 暂时不接入默认构建

也就是说，后续正确做法是：

```text
先把现有顶层实现逐步迁进这些目录
而不是重新发明第二套文件结构
```

当前真实进度：

* `core/` 已经是真实共享实现位置
* `shells/` 已经是真实 shell 位置
* `api/` 已经是真实 wrapper 位置
* 顶层同名文件现在主要承担兼容壳角色

## 当前与 `legacy/` 的关系

* `../legacy/`
  现在主要保留：
  * `fileops.c`
  * `iob.c`
  * 旧代码参考基线
* `./`
  现在已经承载：
  * 真实 `vfprintf` 变体
  * 真实 wrapper
  * 真实 float engine
  * 真实测试覆盖目标

所以当前不是：

```text
legacy 真跑，printf 只是重构草稿
```

而是：

```text
printf 已经是主实现
legacy 主要剩 backend/参考基线
```

## 现在最该继续做什么

如果只围绕“把 `printf` 流程整理好”，后续最该做的不是再补很多 family，而是：

1. 把当前 `README` 和设计文档中的旧 `caps/__printf_core_default` 表述彻底清掉
2. 继续把 conversion `inc` 收成更清楚的 entry/helper 边界
3. 再决定是否继续长出更多 shell 变体

也就是说，接下来应该优先稳定：

```text
shell
  -> variant config
  -> support
  -> core body
  -> parser
  -> dispatch
  -> conversion
  -> bridge/backend
```
