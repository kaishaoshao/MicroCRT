# `MicroCRT` `printf` 目录说明

这个目录不是旧 `stdio` 实现的直接复制，而是 `MicroCRT` 后续 `printf` 重构的起点。

## 当前文件

* `printf_core_body.inc`
  `printf` core 的主循环骨架
* `printf_core_parser.inc`
  `%...` 的解析逻辑
* `printf_core_dispatch.inc`
  specifier 分发逻辑

## 设计目标

目标不是继续维护一个巨大的 `vfprintf.c`，而是逐步收敛成：

```text
单份 core 模板
+ 多份 core shell
+ 薄 public API wrapper
+ 独立 entry/helper
+ stream/backend 解耦
```

## 当前与 `legacy/` 的关系

* `../legacy/`
  保留旧 `src/libc/stdio` 实现，作为语义来源和迁移基线
* `./`
  只放新的 core 模板和后续真正要保留的 `MicroCRT` 实现

换句话说：

* `legacy` 负责“旧代码还原”
* `printf` 负责“新框架落地”

## 下一步

后续应优先做这几件事：

1. 把 `vfprintf` 入口迁入 `MicroCRT/src/libc/stdio/printf/`
2. 把 `%c/%s/%n/%d...` 从旧 include 代码块收成真正的 `_printf_*` entry
3. 把 integer / float / wchar / backend 边界拉开
4. 再决定 `iprintf / printf_full / wprintf` 这些 family 的正式导出方式
