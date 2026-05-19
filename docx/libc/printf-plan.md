# printf 阶段性定稿方案

## 1. 文档目的

这份文档只做一件事：把当前仓库里已经落下来的 `printf` 架构收敛成一份明确的阶段性方案，给后续实现、裁剪和测试提供统一边界。

它不是重新发散“理想架构”的讨论稿。关于参考来源和完整分析，继续看 [printf.md](/home/shaokai/Desktop/test/fusa/mculib-fusa/printf.md:1)。

---

## 2. 当前判断

当前代码已经完成了最关键的结构切分：

```text
printf / fprintf / sprintf / snprintf / iprintf
        ↓
wrapper
        ↓
__printf_core(out, fmt, ap, caps)
        ↓
specifier handlers / shared formatter helpers
        ↓
__printf_out
        ↓
FILE-backed stream 或 memory-backed stream
```

因此后续不建议再回到“每个入口各写一套 formatter”的方向，也不建议在当前阶段强行把所有 `FILE` 语义一次性从路径中彻底清除。

当前最合理的收口策略是：

1. 保留“多入口 + 一个共享 core + 能力配置 caps + 两类 stream backend”。
2. 把 `FILE bridge` 视为当前正式方案的一部分，而不是临时脏补丁。
3. 先把功能边界、裁剪边界、测试边界补完整，再决定是否把 `FILE bridge` 继续抽象成更纯粹的 sink。

---

## 3. 推荐拍板

### 3.1 总体架构

推荐继续采用下面这条主链：

```text
printf / fprintf / iprintf
        ↓
vfprintf / vfiprintf
        ↓
__printf_core
        ↓
__printf_out
        ↓
printf stream layer
        ↓
backend write / flush
```

以及：

```text
sprintf / snprintf / siprintf
        ↓
memory-backed FILE bridge
        ↓
__printf_core
```

这里的关键结论是：

* `sprintf/snprintf` 继续复用同一个 core，不单独维护第二套 formatter。
* `fflush`、buffering、backend flush、RTOS lock 都留在 stream / `FILE` 层，不放进 `__printf_core`。
* `__printf_core` 只负责解析、调度、格式化、写出字符流。

### 3.2 core 边界

当前推荐把 `__printf_core` 的职责固定为：

* 扫描格式串
* 解析 `flags / width / precision / size`
* 按 specifier 分发
* 调共享 formatter helper
* 通过 `__printf_out` 输出字符
* 维护返回值计数

`__printf_core` 不负责：

* backend 选择
* flush 策略
* file lock 策略
* stdout/stderr 初始化
* memory stream 生命周期

也就是说，后续新增能力时，优先扩展 parser / formatter / caps，不把 platform 行为继续塞回 core。

### 3.3 `__printf_out` 的定位

当前实现里，`__printf_out` 已经从 API 形态上把 core 和 `FILE *` 解耦：

```c
struct __printf_out {
    int (*putc)(int c, void *ctx);
    int (*write)(const char *buf, size_t len, void *ctx);
    void *ctx;
    uint8_t *flags;
};
```

这已经足够支撑现阶段的模块化目标，因此当前阶段不建议为了“更纯粹”而重写整个输出链。

结论很明确：

* `__printf_out` 现在就是正式 core 输出接口。
* `ctx` 目前承载 `FILE *` 是可以接受的。
* 只有当后面确实要支持“完全不依赖 stdio 对象的裸 sink”时，再考虑把 `flags`、error 状态、flush 信号继续下沉。

### 3.4 specifier 分层

当前目录已经基本按 Arm 风格拆开：

* parser 层：`__printf_flags.c`、`__printf_wp.c`、`__printf_ss.c`
* core 分发层：`__printf_core.c`
* 常规 specifier 入口：`_printf_c.c`、`_printf_s.c`、`_printf_d.c`、`_printf_i.c`、`_printf_u.c`、`_printf_x.c`、`_printf_o.c`、`_printf_p.c`、`_printf_n.c`
* long long 入口：`_printf_ll*.c`
* 浮点入口：`_printf_f.c`、`_printf_e.c`、`_printf_g.c`、`_printf_a.c`
* 共享 formatter helper：`_printf_int_*`、`_printf_longlong_*`、`_printf_fp_*`、`_printf_common.c`

后续建议继续沿着这个边界演进，不再把多个 specifier 重新揉回一个巨型 `vfprintf.c`。

---

## 4. 能力裁剪方案

### 4.1 统一使用 `caps`

当前已经有 `struct __printf_caps`，这是对的，后续继续以它作为唯一能力开关入口。

推荐把变体管理收敛成两层：

1. core runtime contract
   也就是 `__printf_core(..., caps)` 这一层按 `caps` 判定能力。
2. build-time preset
   也就是 `__printf_caps_full()`、`__printf_caps_int()` 等预置组合。

这样做的好处是：

* 入口函数保持简单
* 测试可以直接覆盖具体能力组合
* 后续即使裁掉 `long long`、`float`、`wchar`，core 代码路径仍然稳定

### 4.2 当前阶段的能力档位

建议文档和测试统一按下面几档理解：

* `full`
  支持常规整数、`long`、`long long`、指针、`%n`、浮点，按配置决定是否开 `wchar`
* `int`
  支持整数族和指针，不支持浮点
* `default`
  作为默认库配置，可按产品要求禁掉部分非关键 specifier，但必须有清晰测试
* `minimal`
  只在明确要做极限裁剪时再保留，不作为当前主路径

当前仓库里已经存在“默认配置禁掉 `%c`”这类行为测试，因此文档必须承认：默认档位可以不是“完整 libc 语义”，但这种差异必须被 `caps` 和测试显式表达，而不是靠隐式宏散落。

### 4.3 配置粒度拍板

当前建议把默认 `printf` 的可配置方案固定成三层，而不是一开始就把接口做成“每个 specifier 一个开关”。

第一层：profile

`MCULIB_PRINTF_DEFAULT_PROFILE`
决定默认 `printf` 的大能力边界：

* `INT`
* `LONG`
* `LONG_LONG`
* `FLOAT_LONG`
* `FLOAT_LONG_LONG`

这一层主要决定：

* 默认 `printf` 到底是不是整数版
* `long` / `long long` 到哪里为止
* 浮点是否进入默认 `printf`
* `long double` 是否进入默认 `printf`

第二层：family toggle

`MCULIB_PRINTF_DEFAULT_ENABLE_*`
在已经选定的 profile 内，再裁掉某些 conversion family。

当前已经确定保留的开关是：

* `CHAR` 对应 `%c`
* `STRING` 对应 `%s`
* `PERCENT` 对应 `%%`
* `SIGNED` 对应 `%d/%i`
* `UNSIGNED` 对应 `%u`
* `OCTAL` 对应 `%o`
* `HEX` 对应 `%x/%X`
* `POINTER` 对应 `%p`
* `PERCENT_N` 对应 `%n`
* `WCHAR` 对应 `%lc/%ls`

例如：

```text
MCULIB_PRINTF_DEFAULT_PROFILE=FLOAT_LONG
MCULIB_PRINTF_DEFAULT_ENABLE_CHAR=OFF
MCULIB_PRINTF_DEFAULT_ENABLE_STRING=OFF
MCULIB_PRINTF_DEFAULT_ENABLE_POINTER=OFF
```

这表示默认 `printf` 仍然支持 `long` 和 `double`，但故意不支持 `%c`、`%s`、`%p`。

第三层：未来的 object split

这一层暂时不急着暴露成用户配置接口，但架构上必须预留。
它对应 Arm 风格那种更细的对象边界，例如：

* `_printf_d.o`
* `_printf_i.o`
* `_printf_x.o`
* `_printf_p.o`
* `_printf_f.o`

这一层真正解决的问题是：

* `%d` 和 `%i` 能不能继续拆开
* `%x` 和 `%X` 能不能继续拆开
* 能不能让链接只保留真正被引用的 conversion object

当前为什么不直接把用户配置做成“每个 specifier 一个开关”：

1. `%d/%i`、`%x/%X`、`%c/%lc` 这些实现关系并不是完全独立。
2. 当前首要目标是把默认 `printf` 从 full 语义里剥出来，而不是先把配置矩阵做爆炸。
3. 单 specifier 级开关会显著放大测试组合数量，当前阶段维护成本不合理。
4. 在对象引用边界还没彻底做细之前，单 specifier 开关未必能换来成比例的 codesize 收益。

因此当前阶段的正式策略是：

1. 对外暴露 `profile + family toggle`
2. 对内继续朝 `single-specifier object split` 演进
3. 当链接器配合能力成熟后，再把裁剪重点下沉到对象引用边界

这三层并不冲突。

更准确地说：

* `profile` 负责选择默认入口的大档位
* `family toggle` 负责在这档能力内进一步裁剪 conversion family
* `object split` 负责未来把这些能力真正落到链接裁剪边界

因此路线应固定为：

* 阶段 1：先稳定 `profile + family toggle`
* 阶段 2：继续把 `_printf_d/_printf_i/_printf_x/_printf_p/_printf_f/...` 和共享 helper 的边界做实
* 阶段 3：有链接器配合后，再把默认变体的裁剪从“编译期开关”逐步下沉为“对象引用级裁剪”

当前禁用某个 family 时，默认路径必须满足下面三个约束：

1. 仍然正确消费对应的可变参数，避免后续参数错位。
2. 沿用 unsupported 语义输出原始转换记号，例如禁用 `%c` 后输出 `%c`。
3. 在默认 core 上避免引用被禁掉的 handler，便于观察 codesize 收缩。

### 4.4 不建议继续扩散的做法

后续不建议再增加“同名 API + 单独一套实现文件 + 静态条件编译分支极多”的旧式变体。

优先级应该是：

1. 保持单一 `__printf_core`
2. 扩展 `caps`
3. 在入口选择不同 preset
4. 用测试证明每个 preset 的行为

---

## 5. stream / backend 方案

### 5.1 当前拍板

当前 `printf_stdio_bridge.c`、`printf_stream_internal.h`、`printf_stream_backend.c` 已经定义出可工作的 stream 层。建议正式承认这一层是当前方案的一部分。

职责划分如下：

* `__printf_core`
  产生字符流
* `__printf_out`
  把字符流交给 stream
* stream layer
  管理 buffered / unbuffered / line-buffered 行为
* backend ops
  对接 `write` / `flush` / `isatty`

### 5.2 backend 接口

推荐继续使用当前这类 backend 形态：

```c
struct __printf_backend_ops {
    ssize_t (*write)(FILE *stream, const void *buf, size_t len, void *cookie);
    int (*flush)(FILE *stream, void *cookie);
    int (*isatty)(FILE *stream, void *cookie);
};
```

原因很简单：

* 它比只留 `_write(fd, ...)` 更能承载 line-buffer / tty 判断 / semihost / RTT 这类差异。
* 它又没有把设备细节带进 `__printf_core`。
* 它已经和当前实现方向一致，迁移成本最低。

### 5.3 `sprintf/snprintf` 的定位

这里明确拍板：

* `sprintf/snprintf/vsprintf/vsnprintf` 继续通过 memory-backed stream 复用 core。
* 它们不是独立 formatter。
* 它们也不是未来优先重构对象。

如果后续要优化 code size 或执行路径，优先优化 shared formatter helper，不要先拆掉 memory bridge。

---

## 6. `fflush`、锁和 RTOS

这些能力保留在 `FILE` / stream 层处理：

* `__flockfile` / `__funlockfile`
* stream buffer flush
* backend flush
* `stdout` line-buffer
* `stderr` unbuffered

`iprintf`、`fprintf`、`vfprintf` 这种 API 的差异，应该体现在入口选用的 `caps` 和 `FILE` 行为上，而不是在 core 内做平台分支。

---

## 7. 下一阶段实施顺序

### 阶段 1：固定 core 合同

目标：

* 入口全部经 `__printf_core(..., caps)`
* specifier 行为由 parser + dispatch + formatter 决定
* 不再新增旧式巨型 `vfprintf` 逻辑

### 阶段 2：固定能力档位

目标：

* 把 `full` / `int` / `default` 的支持矩阵写清楚
* 让默认配置行为由 `caps` 和配置文件显式驱动
* 为每个档位补齐 smoke test

### 阶段 3：补齐 stream 行为

目标：

* `fflush`
* line-buffer / unbuffered
* backend flush 错误传播
* `stdout/stderr` 默认行为

### 阶段 4：补齐高风险功能

目标：

* 浮点
* `long double`
* `wchar`
* `%n`
* 截断返回值和边界条件

只有前四阶段做稳之后，才讨论是否继续把 `FILE bridge` 演进成更纯粹的通用 sink。

---

## 8. 不再建议做的事

这部分单独写出来，避免后续又绕回去：

* 不再为 `sprintf/snprintf` 维护独立 formatter
* 不再把 backend / flush / RTOS 逻辑塞回 `__printf_core`
* 不再回到单文件巨型 `vfprintf.c` 的结构
* 不在当前阶段为了抽象洁癖强推“彻底去 FILE 化”

当前代码最缺的不是“再抽一层”，而是把现有边界稳定下来并用测试锁住。

---

## 9. 最终结论

当前阶段的正式方案是：

```text
共享 __printf_core
+ `caps` 做能力裁剪
+ `__printf_out` 做 core 输出接口
+ stream / FILE bridge 承接 buffering、flush、backend
+ memory-backed stream 复用到 sprintf/snprintf
```

一句话拍板：

```text
先把现有 core + caps + stream 架构做稳，再决定要不要继续做更纯的 sink 化。
```
