#include "my_printf.h"
#include <stddef.h>

// 字符串后端的上下文结构
typedef struct {
    char* buffer;      // 缓冲区指针
    size_t position;   // 当前写入位置
    size_t capacity;   // 缓冲区总容量
} string_backend_context_t;

// 字符串后端的 putc 实现
static void string_putc(char c, void* context) {
    string_backend_context_t* ctx = (string_backend_context_t*)context;
    if (ctx->position < ctx->capacity) {
        ctx->buffer[ctx->position] = c;
    }
    // 即使缓冲区满了，我们仍然增加 position，以便最终能计算出理论上需要多大的缓冲区
    ctx->position++;
}

// --- 公共 API 函数 ---

int my_vsprintf(char* buffer, const char* format, va_list args) {
    string_backend_context_t context = {
        .buffer = buffer,
        .position = 0,
        .capacity = (size_t)-1 // "无限"容量
    };

    my_printf_backend_t backend = {
        .putc_f = string_putc,
        .context = &context
    };

    my_vprintf_core(&backend, format, args);
    
    // 在末尾添加空终止符
    buffer[context.position] = '\0';

    return context.position;
}

int my_vsnprintf(char* buffer, size_t count, const char* format, va_list args) {
    string_backend_context_t context = {
        .buffer = buffer,
        .position = 0,
        .capacity = (count > 0) ? (count - 1) : 0 // 留一个位置给 '\0'
    };

    my_printf_backend_t backend = {
        .putc_f = string_putc,
        .context = &context
    };

    my_vprintf_core(&backend, format, args);

    // 在末尾添加空终止符
    if (count > 0) {
        buffer[context.position < context.capacity ? context.position : context.capacity] = '\0';
    }

    // 返回理论上应该写入的字符数
    return context.position;
}

int my_sprintf(char* buffer, const char* format, ...) {
    va_list args;
    va_start(args, format);
    int count = my_vsprintf(buffer, format, args);
    va_end(args);
    return count;
}

int my_snprintf(char* buffer, size_t count, const char* format, ...) {
    va_list args;
    va_start(args, format);
    int ret = my_vsnprintf(buffer, count, format, args);
    va_end(args);
    return ret;
}
