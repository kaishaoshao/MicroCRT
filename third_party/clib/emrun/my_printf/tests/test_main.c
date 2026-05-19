#include "my_printf.h"
#include <stdio.h>
#include <string.h>
#include <limits.h>

// 一个简单的断言宏
#define ASSERT_STR_EQUAL(expected, actual, message) \
    if (strcmp(expected, actual) != 0) { \
        fprintf(stderr, "Assertion failed: %s\n", message); \
        fprintf(stderr, "  Expected: \"%s\"\n", expected); \
        fprintf(stderr, "  Actual:   \"%s\"\n", actual); \
        return 1; \
    }

#define ASSERT_INT_EQUAL(expected, actual, message) \
    if (expected != actual) { \
        fprintf(stderr, "Assertion failed: %s\n", message); \
        fprintf(stderr, "  Expected: %d\n", expected); \
        fprintf(stderr, "  Actual:   %d\n", actual); \
        return 1; \
    }

int main() {
    char buffer[256];
    int ret;

    // --- 基本测试 ---
    my_printf("--- Running tests for my_printf ---\n");

    // 字符串
    my_snprintf(buffer, sizeof(buffer), "Hello, %s!", "world");
    ASSERT_STR_EQUAL("Hello, world!", buffer, "Simple string formatting");

    // 字符
    my_snprintf(buffer, sizeof(buffer), "The character is %c.", 'A');
    ASSERT_STR_EQUAL("The character is A.", buffer, "Simple char formatting");

    // 百分号
    my_snprintf(buffer, sizeof(buffer), "100%% working.");
    ASSERT_STR_EQUAL("100% working.", buffer, "Percent sign");

    // --- 整数测试 ---
    my_snprintf(buffer, sizeof(buffer), "Decimal: %d", 123);
    ASSERT_STR_EQUAL("Decimal: 123", buffer, "Positive decimal");

    my_snprintf(buffer, sizeof(buffer), "Negative Decimal: %d", -456);
    ASSERT_STR_EQUAL("Negative Decimal: -456", buffer, "Negative decimal");
    
    my_snprintf(buffer, sizeof(buffer), "Unsigned: %u", 123);
    ASSERT_STR_EQUAL("Unsigned: 123", buffer, "Unsigned int");

    my_snprintf(buffer, sizeof(buffer), "Octal: %o", 0123); // 83 in decimal
    ASSERT_STR_EQUAL("Octal: 123", buffer, "Octal formatting");

    my_snprintf(buffer, sizeof(buffer), "Hex: %x", 0xABC);
    ASSERT_STR_EQUAL("Hex: abc", buffer, "Lowercase hex");

    my_snprintf(buffer, sizeof(buffer), "Hex Caps: %X", 0xABC);
    ASSERT_STR_EQUAL("Hex Caps: ABC", buffer, "Uppercase hex");

    // --- 填充和对齐测试 ---
    my_snprintf(buffer, sizeof(buffer), "[%5d]", 12);
    ASSERT_STR_EQUAL("[   12]", buffer, "Right padding with spaces");

    my_snprintf(buffer, sizeof(buffer), "[%-5d]", 12);
    ASSERT_STR_EQUAL("[12   ]", buffer, "Left padding with spaces");

    my_snprintf(buffer, sizeof(buffer), "[%05d]", 12);
    ASSERT_STR_EQUAL("[00012]", buffer, "Zero padding");
    
    my_snprintf(buffer, sizeof(buffer), "[%-05d]", 12); // '0' is ignored when '-' is present
    ASSERT_STR_EQUAL("[12   ]", buffer, "Left padding ignores zero pad");

    // --- 精度测试 ---
    my_snprintf(buffer, sizeof(buffer), "String precision: %.5s", "HelloWorld");
    ASSERT_STR_EQUAL("String precision: Hello", buffer, "String precision");
    
    my_snprintf(buffer, sizeof(buffer), "Integer precision: %.5d", 123);
    ASSERT_STR_EQUAL("Integer precision: 00123", buffer, "Integer precision (zero padding)");

    // --- 指针测试 ---
    int x = 10;
    void* p = &x;
    ret = my_snprintf(buffer, sizeof(buffer), "Pointer: %p", p);
    // A more robust check for pointer format. The prefix "Pointer: 0x" is 11 chars.
    if (strncmp(buffer, "Pointer: 0x", 11) != 0 || ret < 12) {
        fprintf(stderr, "Pointer format seems incorrect: %s\n", buffer);
        return 1;
    }
    // Check that the rest are hex digits
    for (int i = 11; i < ret; ++i) {
        char c = buffer[i];
        if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'))) {
            fprintf(stderr, "Pointer format contains non-hex characters: %s\n", buffer);
            return 1;
        }
    }

    // --- snprintf 返回值和边界测试 ---
    ret = my_snprintf(buffer, 6, "Hello, world!"); // Buffer size is 6
    ASSERT_STR_EQUAL("Hello", buffer, "snprintf truncation");
    ASSERT_INT_EQUAL(13, ret, "snprintf return value on truncation");

    ret = my_snprintf(buffer, 20, "Hello, world!"); // Buffer size is 20
    ASSERT_STR_EQUAL("Hello, world!", buffer, "snprintf no truncation");
    ASSERT_INT_EQUAL(13, ret, "snprintf return value on no truncation");
    
    ret = my_snprintf(NULL, 0, "This should not crash");
    ASSERT_INT_EQUAL(23, ret, "snprintf with NULL buffer and 0 size");

    my_printf("--- All tests passed successfully! ---\n");

    return 0;
}