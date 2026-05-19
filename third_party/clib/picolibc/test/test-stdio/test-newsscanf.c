/*
 * Test suite for the standalone sscanf implementation.
 */

#include <stdio.h>
#include <string.h>
#include <limits.h>

// If running this test standalone, we need to include the C file directly.
#ifdef STANDALONE_TEST
#include "../../libc/stdio/newsscanf.c"
#endif

int total_tests = 0;
int failed_tests = 0;

#define TEST_ASSERT(condition) \
    do { \
        total_tests++; \
        if (!(condition)) { \
            failed_tests++; \
            printf("FAIL: %s:%d: Assertion failed: %s\n", __FILE__, __LINE__, #condition); \
        } \
    } while (0)

#define RUN_TEST(test_func) \
    do { \
        printf("--- Running %s ---\n", #test_func); \
        test_func(); \
    } while (0)

void test_simple_integers() {
    int a, b;
    int ret = sscanf("123 456", "%d %d", &a, &b);
    TEST_ASSERT(ret == 2);
    TEST_ASSERT(a == 123);
    TEST_ASSERT(b == 456);

    int c;
    ret = sscanf("-987", "%d", &c);
    TEST_ASSERT(ret == 1);
    TEST_ASSERT(c == -987);

    unsigned int u;
    ret = sscanf("123", "%u", &u);
    TEST_ASSERT(ret == 1);
    TEST_ASSERT(u == 123);
}

void test_hex_oct_integers() {
    unsigned int x, o;
    int i;

    int ret = sscanf("0x1A 077 0x1a", "%x %o %i", &x, &o, &i);
    TEST_ASSERT(ret == 3);
    TEST_ASSERT(x == 0x1A);
    TEST_ASSERT(o == 077);
    TEST_ASSERT(i == 0x1a);

    ret = sscanf("123", "%i", &i);
    TEST_ASSERT(ret == 1);
    TEST_ASSERT(i == 123);

    ret = sscanf("0123", "%i", &i);
    TEST_ASSERT(ret == 1);
    TEST_ASSERT(i == 0123);
}

void test_strings() {
    char s1[20], s2[20];
    int ret = sscanf("hello world", "%s %s", s1, s2);
    TEST_ASSERT(ret == 2);
    TEST_ASSERT(strcmp(s1, "hello") == 0);
    TEST_ASSERT(strcmp(s2, "world") == 0);

    ret = sscanf("  leading_space", "%s", s1);
    TEST_ASSERT(ret == 1);
    TEST_ASSERT(strcmp(s1, "leading_space") == 0);
}

void test_width_limit() {
    int d;
    char s[10];

    int ret = sscanf("1234567", "%3d", &d);
    TEST_ASSERT(ret == 1);
    TEST_ASSERT(d == 123);

    ret = sscanf("longstring", "%5s", s);
    TEST_ASSERT(ret == 1);
    TEST_ASSERT(strcmp(s, "longs") == 0);
}

void test_assignment_suppression() {
    int d;
    int ret = sscanf("123 456", "%*d %d", &d);
    TEST_ASSERT(ret == 1);
    TEST_ASSERT(d == 456);

    char s[10];
    ret = sscanf("abc def", "%*s %s", s);
    TEST_ASSERT(ret == 1);
    TEST_ASSERT(strcmp(s, "def") == 0);
}

void test_return_value_and_eof() {
    int d;
    int ret = sscanf("abc", "%d", &d);
    TEST_ASSERT(ret == 0);

    ret = sscanf("", "%d", &d);
    TEST_ASSERT(ret == -1); // EOF

    ret = sscanf("123 abc", "%d %d", &d, &d);
    TEST_ASSERT(ret == 1);
}

void test_literal_matching() {
    int d1, d2;
    int ret = sscanf("value: 123, next: 456", "value: %d, next: %d", &d1, &d2);
    TEST_ASSERT(ret == 2);
    TEST_ASSERT(d1 == 123);
    TEST_ASSERT(d2 == 456);

    ret = sscanf("value: 123, WRONG: 456", "value: %d, next: %d", &d1, &d2);
    TEST_ASSERT(ret == 1);
    TEST_ASSERT(d1 == 123);
}

int main() {
    RUN_TEST(test_simple_integers);
    RUN_TEST(test_hex_oct_integers);
    RUN_TEST(test_strings);
    RUN_TEST(test_width_limit);
    RUN_TEST(test_assignment_suppression);
    RUN_TEST(test_return_value_and_eof);
    RUN_TEST(test_literal_matching);

    printf("\n-----------------------------------\n");
    if (failed_tests == 0) {
        printf("All %d tests passed!\n", total_tests);
        return 0;
    } else {
        printf("%d out of %d tests failed.\n", failed_tests, total_tests);
        return 1;
    }
}
