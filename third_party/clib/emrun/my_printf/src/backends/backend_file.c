#include "my_printf.h"
#include <stdio.h> // For FILE, fputc, stdout

// 为文件后端定义上下文，包含一个计数器
typedef struct {
    FILE* stream;
    int count;
} file_backend_context_t;

// 文件后端的 putc 实现，现在会更新上下文中的计数器
static void file_putc(char c, void* context) {
    file_backend_context_t* ctx = (file_backend_context_t*)context;
    fputc(c, ctx->stream);
    ctx->count++;
}

// --- 公共 API 函数 ---

int my_vprintf(const char* format, va_list args) {
    file_backend_context_t context = {
        .stream = stdout,
        .count = 0
    };

    my_printf_backend_t backend = {
        .putc_f = file_putc,
        .context = &context
    };
    
    my_vprintf_core(&backend, format, args);
    
    return context.count; // 返回从上下文中获取的计数
}

int my_printf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    int count = my_vprintf(format, args);
    va_end(args);
    return count;
}
