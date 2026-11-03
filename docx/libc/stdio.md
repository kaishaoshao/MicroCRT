# MicroCRT Stdio 模块函数规范与实现清单

`stdio.h` 是 MicroCRT 的标准输入输出核心头文件，定义在 [include/libc/stdio.h](file:///include/libc/stdio.h)。在嵌入式和微型运行时环境下，Stdio 严格划分为**通用高级 API**和**底层板级桩函数**。

---

## 一、底层板级桩函数 (Target Retargeting Stubs)

当 MicroCRT 运行在裸机、QEMU 或轻量级 RTOS 上时，开发者仅需实现以下桩函数即可驱动整个 Stdio 库：

### 1. `_sys_write`
- **函数原型**：`int _sys_write(FILEHANDLE fh, const unsigned char *buf, unsigned len, int mode);`
- **作用**：将缓冲区数据写入指定设备句柄。
- **参数说明**：
  - `fh`: 设备文件句柄（标准输出 stdout 通常为 1，stderr 为 2）。
  - `buf`: 待写入数据字节缓冲。
  - `len`: 待写入长度。
  - `mode`: 历史兼容保留字段，忽略。
- **返回值**：成功返回 0；部分写入返回未写入的字节数；发生严重错误返回负数。
- **调用时机**：`printf`、`fprintf`、`fputc`、`puts`、`fwrite` 冲刷缓冲区时的唯一硬件出口。

### 2. `_sys_read`
- **函数原型**：`int _sys_read(FILEHANDLE fh, unsigned char *buf, unsigned len, int mode);`
- **作用**：从指定设备句柄读取数据。
- **返回值**：成功读取全部返回 0；遇到 EOF 或读取不足返回未读取字节数；出错返回负值。
- **调用时机**：`scanf`、`fscanf`、`fgetc`、`getchar`、`fread` 的唯一硬件入口。

### 3. `_sys_open`
- **函数原型**：`FILEHANDLE _sys_open(const char *name, int openmode);`
- **作用**：打开设备通道或存储文件，匹配 `":tt"` 打开标准控制台流。

### 4. `_sys_close`
- **函数原型**：`int _sys_close(FILEHANDLE fh);`
- **作用**：关闭文件句柄，释放底层资源。

### 5. `_sys_istty`
- **函数原型**：`int _sys_istty(FILEHANDLE fh);`
- **作用**：判断句柄是否是交互终端（如串口），决定高层是行缓冲（`_IOLBF`）还是全缓冲（`_IOFBF`）。

### 6. `_ttywrch`
- **函数原型**：`void _ttywrch(int ch);`
- **作用**：紧急单字符硬件串口直接输出，用于断言失败（`assert`）或硬故障异常输出。

---

## 二、通用高级函数列表 (Generic Stdio Functions)

### 1. 格式化输出系列 (Formatted Output)
| 函数名称 | 原型声明 | 描述 |
| :--- | :--- | :--- |
| `printf` | `int printf(const char *format, ...);` | 格式化输出到 `stdout` |
| `fprintf` | `int fprintf(FILE *stream, const char *format, ...);` | 格式化输出到指定 `FILE` 流 |
| `sprintf` | `int sprintf(char *str, const char *format, ...);` | 格式化输出到字符串内存缓冲区 |
| `snprintf` | `int snprintf(char *str, size_t n, const char *format, ...);` | 带长度上限截断安全的格式化输出 |
| `vprintf` | `int vprintf(const char *format, va_list ap);` | `va_list` 版本的标准输出 |
| `vfprintf` | `int vfprintf(FILE *stream, const char *format, va_list ap);` | `va_list` 版本的流输出 |
| `vsprintf` | `int vsprintf(char *str, const char *format, va_list ap);` | `va_list` 版本的字符串输出 |
| `vsnprintf` | `int vsnprintf(char *str, size_t n, const char *format, va_list ap);` | `va_list` 版本的带上限字符串输出 |

> **实现策略**：统一收敛到 MicroCRT 模块化 Printf 引擎（`printf_core_template.inc`），支持纯整数裁剪模式、标准浮点模式及扩展转换（十六进制浮点 `%a`、二进制 `%b` 等）。

### 2. 格式化输入系列 (Formatted Input)
| 函数名称 | 原型声明 | 描述 |
| :--- | :--- | :--- |
| `scanf` | `int scanf(const char *format, ...);` | 从 `stdin` 格式化扫描输入 |
| `fscanf` | `int fscanf(FILE *stream, const char *format, ...);` | 从指定流格式化扫描输入 |
| `sscanf` | `int sscanf(const char *str, const char *format, ...);` | 从字符串内存扫描输入 |
| `vscanf` | `int vscanf(const char *format, va_list ap);` | `va_list` 版本的 `scanf` |
| `vfscanf` | `int vfscanf(FILE *stream, const char *format, va_list ap);` | `va_list` 版本的 `fscanf` |
| `vsscanf` | `int vsscanf(const char *str, const char *format, va_list ap);` | `va_list` 版本的 `sscanf` |

### 3. 字符与行 I/O (Character & Line I/O)
| 函数名称 | 原型声明 | 描述 |
| :--- | :--- | :--- |
| `fputc` / `putc` | `int fputc(int character, FILE *stream);` | 向流写入一个字节字符 |
| `putchar` | `int putchar(int character);` | 向 `stdout` 写入一个字符 |
| `fgetc` / `getc` | `int fgetc(FILE *stream);` | 从流读取一个字节字符 |
| `getchar` | `int getchar(void);` | 从 `stdin` 读取一个字符 |
| `ungetc` | `int ungetc(int character, FILE *stream);` | 将一个字符推回流缓冲区 |
| `fputs` | `int fputs(const char *str, FILE *stream);` | 向流写入一个字符串 |
| `puts` | `int puts(const char *str);` | 向 `stdout` 写入字符串并追加换行符 `\n` |
| `fgets` | `char *fgets(char *str, int n, FILE *stream);` | 从流安全按行读取字符串 |
| `gets` | `char *gets(char *str);` | (历史兼容接口，建议使用 `fgets`) |

### 4. 数据块直接 I/O (Direct Binary I/O)
| 函数名称 | 原型声明 | 描述 |
| :--- | :--- | :--- |
| `fread` | `size_t fread(void *ptr, size_t size, size_t count, FILE *stream);` | 二进制块读取 |
| `fwrite` | `size_t fwrite(const void *ptr, size_t size, size_t count, FILE *stream);` | 二进制块写入 |

### 5. 流控制与文件定位 (Stream Control & Positioning)
| 函数名称 | 原型声明 | 描述 |
| :--- | :--- | :--- |
| `fopen` | `FILE *fopen(const char *filename, const char *mode);` | 打开文件并绑定缓冲区 |
| `fclose` | `int fclose(FILE *stream);` | 冲刷并关闭流 |
| `fflush` | `int fflush(FILE *stream);` | 强制冲刷流缓冲区到设备 |
| `setbuf` | `void setbuf(FILE *stream, char *buffer);` | 设置流为自定义缓冲或无缓冲 |
| `setvbuf` | `int setvbuf(FILE *stream, char *buffer, int mode, size_t size);` | 精细控制流缓冲模式与大小 |
| `fseek` | `int fseek(FILE *stream, long offset, int origin);` | 移动流读写位置 |
| `ftell` | `long ftell(FILE *stream);` | 获取流当前读写偏移 |
| `rewind` | `void rewind(FILE *stream);` | 复位流位置到起始位置 |
| `clearerr` | `void clearerr(FILE *stream);` | 清除 EOF 和错误标志 |
| `feof` | `int feof(FILE *stream);` | 检测流是否已达到末尾 |
| `ferror` | `int ferror(FILE *stream);` | 检测流是否存在读写错误 |
