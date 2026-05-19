# `newlib-nano` `printf` 技术路线调研

## 1. 调研目标

这份文档回答的是 `newlib-nano` 的 `printf`/`vfprintf` 技术路线，而不是它的完整 libc 设计。

重点问题有四个：

1. `newlib-nano` 的 formatted I/O 是什么形态
2. 它和 regular newlib 的 `vfprintf` 关系是什么
3. `_printf_float` / `iprintf` 这些常见机制本质上是什么
4. 它和 Arm / emRun 的路线差异在哪里

---

## 2. 调研范围与证据来源

本地仓库里没有直接携带 `newlib-nano` 源码，所以这份报告主要基于：

### 2.1 官方/一手来源

* Newlib 邮件列表关于引入 `--enable-newlib-nano-formatted-io` 的讨论
* Newlib 邮件列表里对 `nano-vfprintf.c` / `_printf_float` 的修复与说明
* Newlib 构建系统补丁中可见的 `nano-vfprintf*.c` 文件列表

### 2.2 本地辅助来源

* `clib/picolibc/README.md` 中对 `newlib-nano` 的提及
* 我们当前仓库里已经实现的 `iprintf` / float stub 相关经验

### 2.3 主要参考链接

* 2014 引入说明：`--enable-newlib-nano-formatted-io`
  来源：Newlib 邮件列表  
  <https://sourceware.org/legacy-ml/newlib/2014/msg00169.html>

* 同一主题的原始 patch 帖子
  来源：Newlib 邮件列表  
  <https://sourceware.org/pipermail/newlib/2014/011491.html>

* 2020 关于 long double/功能取舍的说明
  来源：Newlib 邮件列表  
  <https://sourceware.org/pipermail/newlib/2020/017584.html>

* 2025 关于 `_printf_float` 未链接时 `nano-vfprintf.c` 行为的修复
  来源：Newlib 邮件列表  
  <https://sourceware.org/pipermail/newlib/2025/021568.html>

* 2022 构建文件中可见 `nano-vfprintf_float.c`
  来源：Newlib 邮件列表 / Makefile diff  
  <https://sourceware.org/pipermail/newlib/2022/018944.html>

---

## 3. 总结先行

先给结论：

```text
newlib-nano 不是 Arm 那种“极细对象级 printf 拆分”路线，
也不是 emRun 那种“同一 core 源码多次编译出很多 vfprintf_xxx 变体”的路线。

newlib-nano 更像是：
一套单独的精简 formatted-io 实现
+ 若干编译期能力裁剪
+ 少量特殊符号控制大能力（尤其浮点）
+ iprintf/iscanf 家族直接别名到普通族
```

换句话说，它的技术重点是：

```text
small-footprint 替代实现
而不是
对象收集级别的精细装配
```

---

## 4. `newlib-nano` 是如何引入的

2014 年 Newlib 邮件列表中，Bin Cheng 提交了引入小尺寸 formatted I/O 的 patch。

文中明确提到新增配置项：

```text
--enable-newlib-nano-formatted-io
```

并说明这是一个：

```text
special implementation of formatted IO functions
designed to lower the size of applications on small systems
```

直接证据见：

* `turn4view0` 第 105-107 行
* `turn4view1` 第 97-103 行

也就是说，`newlib-nano` 的 formatted I/O 从一开始就不是 regular `vfprintf` 上做一点点裁剪，而是**独立的一套实现**。

---

## 5. 它不是 regular `vfprintf` 的“小开关模式”

这是理解 `newlib-nano` 最关键的点。

### 5.1 它引入了独立文件族

在 2014 的引入说明里，直接列出了新增文件：

```text
libc/stdio/nano-vfprintf.c
libc/stdio/nano-vfprintf_float.c
libc/stdio/nano-vfprintf_i.c
libc/stdio/nano-vfprintf_local.h
libc/stdio/nano-vfscanf.c
libc/stdio/nano-vfscanf_float.c
libc/stdio/nano-vfscanf_i.c
libc/stdio/nano-vfscanf_local.h
```

这在 `turn4view0` 的 219-225 行，以及 `turn4view1` 的 211-218 行里都能直接看到。

这意味着：

* `newlib-nano` 不是简单在 regular `vfprintf.c` 上打几个宏
* 它是单独维护一套 `nano-vfprintf*.c` 家族

### 5.2 upstream 讨论里也明确说“两套实现彼此独立”

同一封邮件中明确写到：

```text
The two implementations are totally independent to each other
```

可见 `turn4view1` 第 155-160 行。

这句话很重要。

它直接说明：

```text
regular newlib formatted IO
和
newlib-nano formatted IO
在实现层就是两套体系
```

所以从工程路线看，`newlib-nano` 更接近：

```text
单独一套“精简版 printf 库”
```

而不是：

```text
在同一套 core 上做很多对象变体组合
```

---

## 6. 它的核心技术点是什么

### 6.1 只支持较小的标准能力面

引入说明里明确写了：

```text
The non-wide-char formatted I/O functions only support the C89 standard
```

可见 `turn4view0` 第 110-114 行，`turn4view1` 第 102-106 行。

这意味着：

* 它从一开始就不是为了“完整标准兼容”
* 它的首要目标是 size，而不是功能完备

### 6.2 浮点支持从主路径里拆出去

引入说明写得非常明确：

```text
It removes direct dependency on floating-point IO handling code.
Programs that need to handle floating-point IO must now explicitly
request the feature by providing options "-u _printf_float" or "-u _scanf_float"
```

可见 `turn4view0` 第 115-118 行，`turn4view1` 第 107-110 行。

这说明 `newlib-nano` 的一个核心技术就是：

```text
默认 printf 不直接依赖浮点格式化实现
浮点通过特殊符号按需拉入
```

### 6.3 `iprintf` / `iscanf` 家族不再是独立实现

同一份引入说明里又写到：

```text
It removes now redundant integer-only implementation of the printf/scanf family
(iprintf/iscanf, etc.). These functions now alias the standard routines.
```

可见 `turn4view0` 第 119-123 行，`turn4view1` 第 111-115 行。

也就是说，在 `newlib-nano` 里：

* `iprintf` 不再对应另一套 integer-only formatter 源码
* 它只是普通族函数的别名/同实现入口

这是 `newlib-nano` 和 emRun、Arm 很不同的一点。

### 6.4 一些 regular newlib 的配置项在 nano 下直接失效

引入说明中明确列出，在 nano formatted I/O 下，下列选项对非宽字符 formatted I/O 不再适用：

```text
enable-newlib-io-pos-args
enable-newlib-io-c99-formats
enable-newlib-io-long-long
enable-newlib-io-long-double
```

可见 `turn4view1` 第 140-149 行。

这说明 `newlib-nano` 的取舍不是“精细选择某些对象”，而是：

```text
直接缩小支持面
```

---

## 7. `_printf_float` 机制本质是什么

这是嵌入式用户最常接触到的 `newlib-nano` 特征。

### 7.1 行为特征

默认使用 `newlib-nano` 时：

* `%f/%e/%g/%a` 这类浮点格式说明符可能被识别
* 但对应真正的 float formatting helper 默认不一定被链接进来

需要用户显式加：

```text
-u _printf_float
```

来强制把浮点打印支持拉入。

### 7.2 2025 修复邮件证明了这个机制依然存在

2025 的 patch 讨论里，直接写到：

```text
when printing float without "_printf_float" being linked
```

而且 patch 修改的是：

```text
newlib/libc/stdio/nano-vfprintf.c
```

可见 `turn2view2` 第 31-42 行。

这说明：

* `_printf_float` 不是“旧历史残留”
* 它就是 `newlib-nano` 目前依然存在的实际机制

### 7.3 这不是 Arm/emRun 那种多 core 变体方案

这个机制的本质更像：

```text
主 printf 实现保留浮点入口点
但把重的浮点转换例程拆成可选链接块
```

所以它优化的是：

* 是否拉入“浮点支持块”

而不是：

* 编译期生成很多 `vfprintf_xxx`
* 或对象层精细选择 `%f` / `%e` / `%g`

---

## 8. `iprintf` 在 `newlib-nano` 中到底是什么

很多人容易误以为 `newlib-nano` 的 `iprintf` 仍然是一套独立 integer-only 实现。

但从引入说明看，nano 路线恰恰反过来做了：

```text
integer-only implementation is removed
iprintf/iscanf family alias the standard routines
```

因此在 `newlib-nano` 里：

* `iprintf` 更像是 API 别名兼容层
* 它不是像 emRun 那样有单独变体策略意义
* 它也不是像 Arm 那样能天然带来对象级整数路径裁剪

这点非常重要。

因为如果你希望：

```text
iprintf 真的成为一个明确的整数能力版本
```

那 `newlib-nano` 的路线并不适合作为最终模板。

---

## 9. `newlib-nano` 和 Arm / emRun 的本质差异

### 9.1 与 Arm 的差异

Arm 的重点在：

* 多个 core 变体对象
* 薄 specifier entry object
* shared helper object
* 最终形成细粒度对象裁剪边界

而 `newlib-nano` 的重点在：

* 单独一套小尺寸 formatted I/O 实现
* 直接缩掉很多功能面
* 用 `_printf_float` 这类特殊符号控制大块能力
* `iprintf` 直接 alias，不再维持第二套整数实现

所以它不是 Arm 式对象化路线。

### 9.2 与 emRun 的差异

emRun 的重点在：

* 一份 core 源码
* 多次编译生成很多 `__SEGGER_RTL_vfprintf_*` 变体
* public wrapper 调 canonical symbol
* 由构建体系/选择机制决定最终落到哪个 core

而 `newlib-nano` 并没有表现出这种“同一 core 多次编译为很多 vfprintf 变体”的思路。

它更像：

```text
regular newlib 一套
newlib-nano 再一套
```

所以它不是 emRun 式同源多变体路线。

---

## 10. `newlib-nano` 方案的优点

### 10.1 设计直接

它没有 Arm 那种很多对象边界，也没有 emRun 那种很多 core 变体名。

从使用者角度很简单：

* 用 `nano` 版本
* 默认更小
* 要浮点时显式 `-u _printf_float`

### 10.2 默认行为非常偏向 code size

这点对嵌入式很有效。

因为它不是“让用户组合 20 个开关”，而是：

```text
先给你一套默认就更小的 printf
```

### 10.3 浮点分离策略非常实用

`_printf_float` / `_scanf_float` 这种按符号拉入的方式非常适合：

* 浮点很重
* 但很多项目根本不用

这是一种很典型的嵌入式优化手法。

---

## 11. `newlib-nano` 方案的缺点

### 11.1 不是细粒度对象裁剪路线

如果目标是未来配合链接器做到：

* `%d` 单独
* `%x` 单独
* `%f` 单独
* `%a` 单独

那 `newlib-nano` 提供的启发有限。

它更像“整套替代实现”，不是“细粒度对象图”。

### 11.2 `iprintf` 不再是真正的独立能力变体

因为它 alias 到普通族，所以：

* `iprintf` 不再天然代表“独立整数路径”
* 对我们想要的“显式能力分层”帮助有限

### 11.3 与完整语义差异更大

由于它直接限定：

* 非 wide-char 格式化只支持 C89
* 一些 `newlib` 选项在 nano 下无效

所以它更适合作为“小系统默认精简库”，不太适合作为“后续逐层增强成 Arm 风格对象体系”的主框架。

### 11.4 维护上是“双实现”

引入讨论里已经明确：

```text
The two implementations are totally independent to each other
```

这意味着维护成本是：

* regular newlib 一套 formatted I/O
* nano newlib 再一套 formatted I/O

这条路对我们并不一定合适，因为我们现在正希望尽量避免长期维持两套 core 逻辑。

---

## 12. 对我们设计的直接启发

### 12.1 值得借鉴的点

最值得借鉴的是两点：

1. 默认 `printf` 应该保守，而不是默认 full
2. 浮点支持最好有独立拉入机制

特别是第二点，很值得我们后续吸收：

```text
默认 core 不应强依赖浮点大实现
浮点块可以独立控制是否进入最终链接结果
```

如果把这两点翻译成我们当前库里的具体落地动作，可以变成下面几条：

1. 默认 `printf` 保持非 full 档位
   继续保留默认 `printf` 和 `printf_full` 的分层，不把默认入口退回 full。

2. 浮点实现不要绑死在默认主路径上
   即使默认 profile 允许 `%f/%e/%g/%a`，对应实现也应该尽量保持对象边界清晰，后续好做显式拉入。

3. 大能力最好有单独的“开关符号”或“独立入口”
   `newlib-nano` 用的是 `_printf_float`；我们未必照抄名字，但“用户显式请求浮点大能力”这个思想值得保留。

4. 用户视角的配置要简单
   即使内部实现很细，对外也应尽量维持少量清晰选项，而不是让用户组合过多低层开关。

5. 文档和构建系统要把“默认能力面”说死
   默认支持哪些 specifier、哪些长度修饰、哪些浮点格式，应该在构建选项和文档里明确列出。

### 12.2 更细的可借鉴方法

除了上面的总原则，`newlib-nano` 还有三种方法论可以吸收：

1. 先按“大块成本”分层，而不是一上来追求极细粒度
   `newlib-nano` 先把最贵的浮点块单独处理。对我们来说，可以先把 `integer / float / wchar / locale` 这几类能力边界做清楚。

2. 保持默认路径尽量短
   默认路径里少做分支、少拉 helper、少带 full 语义，这样当前过渡版本的 code size 更容易控制。

3. 把“能力选择”放到构建和链接层，而不是运行时
   `_printf_float` 本质上就是让能力选择发生在最终链接阶段，这和我们后面想做的“单 core 源码、多份变体产物、由链接选择公共入口”是相容的。

### 12.3 不建议直接照抄的点

不建议照抄的点也很明确：

1. 不建议长期维护“regular printf + nano printf”两套独立 core 实现
2. 不建议把 `iprintf` 退化成单纯 alias
3. 不建议只靠 `_printf_float` 这种大块开关结束全部设计

因为我们的目标不只是“做一个更小的默认 printf”，还包括：

* 后续往 Arm 风格对象边界演进
* 保留 emRun 式单模板多编译变体能力

### 12.4 对当前方案的直接结论

结合我们现在已经讨论过的方向，`newlib-nano` 对当前方案最合理的吸收方式是：

1. 保留单份 `printf_core` 模板源码
2. 默认 `printf` 继续走非 full 变体
3. 浮点保持独立能力块，后续可做成显式拉入
4. `iprintf` 继续保留为明确的整数能力入口，不退化成 alias
5. 内部实现仍然继续朝 Arm 风格 object split 靠拢，而不是复制一套 `nano printf`

### 12.5 四种路线对照

下面这张表把 `Arm / emRun / newlib-nano / 我们目标方案` 放在一起看：

| 维度 | Arm armlib | emRun | newlib-nano | 我们目标方案 |
| --- | --- | --- | --- | --- |
| core 源码组织 | 很可能单模板，但对象层明确多 core 变体 | 单份 `prinops.c` 多次编译出很多 `vfprintf_xxx` | regular/newlib-nano 两套独立实现 | 单份 `printf_core` 模板，多壳文件编译 |
| 对象边界 | 很细，`__printf$flags` / `_printf_d` / `_printf_f` 这种级别 | 中等，重点在多个 `vfprintf_xxx` 变体 | 不细，重点是整套 nano 实现 | 目标是逐步细化到 Arm 风格 |
| public API 组织 | `printf/iprintf/...` + 内部很多 object | `printf/vfprintf` + 多个内部能力变体 | `printf` 主族 + alias 化的 `iprintf` | `printf/iprintf/printf_full/wprintf` 分层保留 |
| 默认能力面 | 可裁剪，依赖对象收集 | 由选择的 `vfprintf_xxx` 决定 | 默认保守，非 full | 默认保守，非 full |
| 浮点处理 | 独立对象/组 | 单独 float 变体 | `_printf_float` 显式拉入 | 独立能力块，后续支持显式拉入 |
| `iprintf` 语义 | 独立能力入口 | 独立能力入口 | alias 到普通族 | 独立能力入口 |
| 主要优势 | 链接裁剪潜力最好 | 单源多变体，工程选择直接 | 用户视角简单，默认更小 | 兼顾过渡期可用性和后续 object split |
| 主要问题 | 需要较强的对象组织和链接配合 | 变体多，命名和构建组织复杂 | 双实现维护，不利于继续对象化 | 过渡期仍要平衡配置、size 和维护成本 |

如果只看“对我们最应该吸收什么”，可以压缩成下面四条：

1. 吸收 Arm 的对象边界思路
2. 吸收 emRun 的单模板多变体编译思路
3. 吸收 `newlib-nano` 的默认非 full 和浮点显式拉入思路
4. 避免 `newlib-nano` 的双实现维护方式

---

## 13. 最终结论

最终结论压缩成五句话：

1. `newlib-nano` 的 formatted I/O 是一套独立于 regular newlib 的 small-footprint 实现。
2. 它的代表文件族是 `nano-vfprintf.c / nano-vfprintf_i.c / nano-vfprintf_float.c`。
3. 它的关键技术不是对象级精细拆分，而是“独立精简实现 + 默认去功能 + 浮点按符号显式拉入”。
4. 在 `newlib-nano` 里，`iprintf/iscanf` 家族不再维持独立整数实现，而是 alias 到普通族。
5. 对我们最有价值的启发是“默认非 full”和“浮点独立拉入”，但它不适合作为我们最终对象化框架的直接模板。
