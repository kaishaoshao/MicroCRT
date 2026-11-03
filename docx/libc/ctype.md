# MicroCRT Ctype 模块函数规范与实现清单

`ctype.h` 是 MicroCRT 的字符分类和大小写转换头文件，定义在 [include/libc/ctype.h](file:///include/libc/ctype.h)。

---

## 一、模块设计原理

在嵌入式系统中，`ctype` 函数在字符串解析、网络协议解码和 `printf`/`scanf` 格式化中被极高频调用。
MicroCRT 采用 **256 字节位掩码查表法（Lookup Table）** 实现所有分类函数：
- 内存开销极低（仅需 256 字节常量 ROM `const unsigned short _ctype_table[256]`）。
- 时间复杂度恒定为 **O(1)**，无分支预测跳转惩罚。

### 属性位掩码设计
```c
#define _CT_U   0x01  /* 大写字母 [A-Z] */
#define _CT_L   0x02  /* 小写字母 [a-z] */
#define _CT_D   0x04  /* 十进制数字 [0-9] */
#define _CT_X   0x08  /* 十六进制字符 [0-9a-fA-F] */
#define _CT_S   0x10  /* 空白字符 (空格, \t, \n, \r, \f, \v) */
#define _CT_P   0x20  /* 标点符号 */
#define _CT_C   0x40  /* 控制字符 [0x00-0x1F, 0x7F] */
#define _CT_B   0x80  /* 空格或水平制表符 (Blank: ' ', '\t') */
```

---

## 二、函数列表与规范

### 1. 字符分类函数 (Classification)
| 函数名称 | 原型声明 | 判断条件 |
| :--- | :--- | :--- |
| `isalnum` | `int isalnum(int c);` | `isalpha(c) \|\| isdigit(c)` (字母或数字) |
| `isalpha` | `int isalpha(int c);` | `isupper(c) \|\| islower(c)` (字母) |
| `isblank` | `int isblank(int c);` | 空格 `' '` 或水平制表符 `'\t'` |
| `iscntrl` | `int iscntrl(int c);` | 控制字符 (ASCII 0x00~0x1F 以及 0x7F) |
| `isdigit` | `int isdigit(int c);` | 十进制数字 `'0'` ~ `'9'` |
| `isgraph` | `int isgraph(int c);` | 除空格外的任何可打印字符 |
| `islower` | `int islower(int c);` | 小写英文字母 `'a'` ~ `'z'` |
| `isprint` | `int isprint(int c);` | 所有可打印字符 (包括空格 `' '`) |
| `ispunct` | `int ispunct(int c);` | 所有除字母、数字、空格外的标点字符 |
| `isspace` | `int isspace(int c);` | 标准空白符 (`' '`, `'\t'`, `'\n'`, `'\r'`, `'\f'`, `'\v'`) |
| `isupper` | `int isupper(int c);` | 大写英文字母 `'A'` ~ `'Z'` |
| `isxdigit`| `int isxdigit(int c);`| 十六进制字符 (`0-9`, `a-f`, `A-F`) |

### 2. 字符转换函数 (Conversion)
| 函数名称 | 原型声明 | 描述 |
| :--- | :--- | :--- |
| `tolower` | `int tolower(int c);` | 若为大写英文字母则转换为小写，否则原样返回 |
| `toupper` | `int toupper(int c);` | 若为小写英文字母则转换为大写，否则原样返回 |

### 3. ASCII 兼容扩展 (Extensions)
| 函数名称 | 原型声明 | 描述 |
| :--- | :--- | :--- |
| `isascii` | `int isascii(int c);` | 判断是否在 7-bit ASCII 范围 (`0 <= c <= 127`) |
| `toascii` | `int toascii(int c);` | 将输入截断为 7-bit ASCII 码 (`c & 0x7F`) |
