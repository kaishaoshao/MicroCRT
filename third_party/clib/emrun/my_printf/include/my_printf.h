#ifndef MY_PRINTF_H
#define MY_PRINTF_H

#include <stdarg.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 定义输出后端的接口结构体。
 *
 * @param putc_f 一个函数指针，用于输出单个字符。
 * @param context 一个上下文指针，传递给 putc_f，用于后端维持状态（例如，字符串缓冲区指针）。
 */
typedef struct {
    void (*putc_f)(char c, void* context);
    void* context;
} my_printf_backend_t;

/**
 * @brief 核心格式化函数。
 *
 * 这是所有 printf 族函数内部调用的核心实现。
 * 它解析格式字符串，并使用指定的后端输出字符。
 *
 * @param backend 指向输出后端接口的指针。
 * @param format 格式字符串。
 * @param args va_list 参数列表。
 * @return int 成功输出的字符数。
 */
void my_vprintf_core(my_printf_backend_t* backend, const char* format, va_list args);

/**
 * @brief 格式化输出到标准输出。
 */
int my_printf(const char* format, ...);

/**
 * @brief 格式化输出到字符串。
 * 
 * @warning 不进行边界检查，请确保缓冲区足够大。
 */
int my_sprintf(char* buffer, const char* format, ...);

/**
 * @brief 安全地格式化输出到字符串，带长度限制。
 *
 * @param buffer 输出缓冲区。
 * @param count 缓冲区的最大容量（包括末尾的 '\0'）。
 * @param format 格式字符串。
 * @return int 如果缓冲区足够大，将返回写入的字符数（不包括'\0'）。如果缓冲区被截断，也返回理论上应写入的字符数。
 */
int my_snprintf(char* buffer, size_t count, const char* format, ...);

/**
 * @brief my_printf 的 va_list 版本。
 */
int my_vprintf(const char* format, va_list args);

/**
 * @brief my_sprintf 的 va_list 版本。
 */
int my_vsprintf(char* buffer, const char* format, va_list args);

/**
 * @brief my_snprintf 的 va_list 版本。
 */
int my_vsnprintf(char* buffer, size_t count, const char* format, va_list args);


#ifdef __cplusplus
}
#endif

#endif // MY_PRINTF_H
