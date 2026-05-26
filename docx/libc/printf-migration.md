# `printf` 迁移说明

这次迁移的目标不是立刻让 `MicroCRT` 可编译出最终版 `printf`，而是先把当前项目里和 `printf` 相关的自有实现与外部参考实现都收拢到 `MicroCRT` 仓库内，后续再在 `MicroCRT` 内部继续收敛。

## 1. 迁移后的目录

```text
MicroCRT/
  src/libc/stdio/legacy/
    旧 src/libc/stdio printf 实现

  src/libc/stdio/printf/
    当前正在收敛的 core 模板骨架

  third_party/clib/
    完整第三方 libc 参考目录复制
```

## 2. 自有实现来源

从主工程 `src/libc/stdio/` 迁入了这些文件：

* `printf.c`
* `fprintf.c`
* `sprintf.c`
* `snprintf.c`
* `vfprintf.c`
* `vfprintf_char.c`
* `vfprintf_str.c`
* `vfprintf_n.c`
* `vfprintf_int.c`
* `vfprintf_float.c`
* `vfprintf_s.c`
* `fileops.c`
* `iob.c`
* `stdio_private.h`
* `conv_flt.c`
* `dtoa.h`
* `ultoa_invert.c`

这些文件现在放在：

```text
MicroCRT/src/libc/stdio/legacy/
```

用途是：

* 保留旧 `FILE`/`vfprintf` 路线的完整上下文
* 作为后续抽取 `entry/helper/backend` 的事实来源

## 3. 当前模板骨架来源

从主工程当前正在进行的 `vfprintf` 收敛工作中，迁入了：

* `printf_core.inc`

这些文件现在放在：

```text
MicroCRT/src/libc/stdio/printf/
```

用途是：

* 作为后续 `MicroCRT printf` 的单模板 core 起点
* 明确主循环以及 parser/dispatch 在模板内的收口边界

## 4. 外部参考源码来源

### 4.1 完整 `clib` 复制

这次不是只拷 `printf` 零散文件，而是把第三方参考库的完整目录复制到了：

```text
MicroCRT/third_party/clib/
  emrun/
  picolibc/
  newlib/
```

用途是：

* 保留完整上游目录结构，方便后续追源码上下文
* 不只看 `printf` 单文件，也能结合它们自己的 `include`、`build`、`backend`、`test` 目录来分析
* 避免后面再次因为“只拷了部分文件”而丢失上下文

### 4.2 各第三方目录的参考用途

#### `third_party/clib/emrun`

主要用于参考：

* 单 core + backend 分层
* public API 与内部 `vfprintf` 变体的组织方式
* semihost / RTT / file / string 这类 backend 思路

#### `third_party/clib/picolibc`

主要用于参考：

* 旧 newlib/picolibc 风格的 `vfprintf` 结构
* `iprintf`、wide、float、`vfiprintf` 等接口族
* `stdio` 上下文与 `printf` 的耦合点

#### `third_party/clib/newlib`

主要用于参考：

* regular `printf` 家族
* `iprintf/siprintf/fiprintf` 家族
* `newlib-nano` 的 `nano-vfprintf.c / nano-vfprintf_i.c / nano-vfprintf_float.c`
* `_printf_float` 一类机制的源码上下文

## 5. 这次迁移的边界

这次只做了“源码搬运与落位”，没有做这些事：

1. 没有把 `MicroCRT` 立即接上新的构建系统
2. 没有把 `legacy` 实现改写成最终 `MicroCRT` 版
3. 没有删除主工程里原有 `src` 和 `clib` 的内容
4. 没有把 `vendor` 目录中的参考实现混编进 `MicroCRT`

## 6. 关于 submodule

这次迁入 `MicroCRT/third_party/clib/*` 的内容是普通源码目录复制，不是 git submodule。

也就是说：

* `MicroCRT/third_party/clib/emrun`
* `MicroCRT/third_party/clib/picolibc`
* `MicroCRT/third_party/clib/newlib`

内部都没有保留上游仓库的 `.git` 目录。

这符合“作为源代码上传，不作为 submodule”的要求。

## 7. 后续建议

后续在 `MicroCRT` 内部推进时，建议顺序是：

1. 先基于 `src/libc/stdio/printf/` 的模板骨架扩成真正的 `MicroCRT` core
2. 再从 `src/libc/stdio/legacy/` 抽取 entry/helper/backend
3. 最后再对照 `vendor/printf/*` 做 `iprintf/full/wide/nano` 能力分层
