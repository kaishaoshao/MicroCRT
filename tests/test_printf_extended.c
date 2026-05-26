#include <stdio.h>
#include <string.h>
#include <wchar.h>

static int failures;

struct test_file {
    FILE file;
    char buf[64];
    char *pos;
    int flush_count;
};

static void
check_str(const char *name, const char *expect, const char *actual)
{
    if (strcmp(expect, actual) != 0) {
        fprintf(stderr, "FAIL %s\n", name);
        fprintf(stderr, "  expect: \"%s\"\n", expect);
        fprintf(stderr, "  actual: \"%s\"\n", actual);
        failures++;
    }
}

static void
check_int(const char *name, int expect, int actual)
{
    if (expect != actual) {
        fprintf(stderr, "FAIL %s\n", name);
        fprintf(stderr, "  expect: %d\n", expect);
        fprintf(stderr, "  actual: %d\n", actual);
        failures++;
    }
}

static int
test_file_put(char c, FILE *stream)
{
    struct test_file *tf = (struct test_file *) stream;

    if ((size_t) (tf->pos - tf->buf) >= sizeof(tf->buf) - 1)
        return EOF;
    *tf->pos++ = c;
    *tf->pos = '\0';
    return (unsigned char) c;
}

static int
test_file_flush(FILE *stream)
{
    struct test_file *tf = (struct test_file *) stream;
    tf->flush_count++;
    return 0;
}

static int
format_to_buf(char *buf, size_t size, const char *fmt, ...)
{
    int ret;
    va_list ap;

    va_start(ap, fmt);
    ret = vsnprintf(buf, size, fmt, ap);
    va_end(ap);
    return ret;
}

static int
iformat_to_buf(char *buf, const char *fmt, ...)
{
    int ret;
    va_list ap;

    va_start(ap, fmt);
    ret = vsiprintf(buf, fmt, ap);
    va_end(ap);
    return ret;
}

static int
vformat_file(FILE *stream, const char *fmt, ...)
{
    int ret;
    va_list ap;

    va_start(ap, fmt);
    ret = vfprintf(stream, fmt, ap);
    va_end(ap);
    return ret;
}

static int
full_format_to_buf(char *buf, size_t size, const char *fmt, ...)
{
    int ret;
    va_list ap;

    va_start(ap, fmt);
    ret = vsnprintf_full(buf, size, fmt, ap);
    va_end(ap);
    return ret;
}

int
main(void)
{
    char buf[128];
    static const wchar_t wide_word[] = L"Hi";
    long l = 1234567890L;
    long long ll = -1234567890123LL;
    struct test_file tf = {
        .file = { .flags = __SWR, .put = test_file_put, .flush = test_file_flush },
        .buf = { 0 },
        .pos = NULL,
        .flush_count = 0,
    };
    int ret;

    tf.pos = tf.buf;

    ret = snprintf(buf, sizeof(buf), "%ld %lld", l, ll);
    check_str("long-long-longlong", "1234567890 -1234567890123", buf);
    check_int("long-long-longlong ret", 25, ret);

    ret = snprintf(buf, sizeof(buf), "[%*.*d]", 6, 4, 12);
    check_str("dynamic width precision", "[  0012]", buf);
    check_int("dynamic width precision ret", 8, ret);

    ret = snprintf(buf, sizeof(buf), "%2$d/%1$d", 7, 12);
    check_str("positional reorder", "12/7", buf);
    check_int("positional reorder ret", 4, ret);

    ret = snprintf(buf, sizeof(buf), "[%2$*1$d]", 6, 12);
    check_str("positional width", "[    12]", buf);
    check_int("positional width ret", 8, ret);

    ret = snprintf(buf, sizeof(buf), "[%+d][% d][%#x][%#o]", 12, 12, 0x2a, 012);
    check_str("flags", "[+12][ 12][0x2a][012]", buf);
    check_int("flags ret", 21, ret);

    memset(buf, 'Z', sizeof(buf));
    ret = snprintf(buf, 5, "abcdef");
    check_str("truncate", "abcd", buf);
    check_int("truncate ret", 6, ret);
    check_int("truncate nul", 1, buf[4] == '\0');

    buf[0] = 'Q';
    ret = snprintf(buf, 0, "abcdef");
    check_int("size0 ret", 6, ret);
    check_int("size0 untouched", 1, buf[0] == 'Q');

    ret = format_to_buf(buf, sizeof(buf), "%s:%d:%04x", "tag", -7, 0x2a);
    check_str("vsnprintf helper", "tag:-7:002a", buf);
    check_int("vsnprintf helper ret", 11, ret);

    ret = format_to_buf(buf, sizeof(buf), "<%s>", (char *) NULL);
    check_str("vsnprintf null string", "<(null)>", buf);
    check_int("vsnprintf null string ret", 8, ret);

    ret = snprintf(buf, sizeof(buf), "%lc:%ls", L'A', wide_word);
    check_str("wide char string", "A:Hi", buf);
    check_int("wide char string ret", 4, ret);

    ret = snprintf(buf, sizeof(buf), "<%ls>", (wchar_t *) NULL);
    check_str("wide null string", "<(null)>", buf);
    check_int("wide null string ret", 8, ret);

    ret = sprintf(buf, "%-6s/%04d", "xy", 9);
    check_str("sprintf", "xy    /0009", buf);
    check_int("sprintf ret", 11, ret);

    ret = siprintf(buf, "%d %u %x %lld", -7, 9u, 0x2a, 1234567890123LL);
    check_str("siprintf", "-7 9 2a 1234567890123", buf);
    check_int("siprintf ret", 21, ret);

    ret = iformat_to_buf(buf, "%08d/%#x", 12, 0x2a);
    check_str("vsiprintf helper", "00000012/0x2a", buf);
    check_int("vsiprintf helper ret", 13, ret);

    ret = iformat_to_buf(buf, "[%f]", 1.5);
    check_str("vsiprintf float placeholder", "[*float*]", buf);
    check_int("vsiprintf float placeholder ret", 9, ret);

    ret = snprintf_full(buf, sizeof(buf), "%.2f/%lld", 12.345, 1234567890123LL);
    check_str("snprintf_full", "12.35/1234567890123", buf);
    check_int("snprintf_full ret", 19, ret);

    ret = full_format_to_buf(buf, sizeof(buf), "%#.3a", 12.0);
    check_str("vsnprintf_full helper", "0x1.800p+3", buf);
    check_int("vsnprintf_full helper ret", 10, ret);

    ret = fprintf(&tf.file, "xy:%d", 7);
    check_str("fprintf custom stream", "xy:7", tf.buf);
    check_int("fprintf custom stream ret", 4, ret);

    tf.pos = tf.buf;
    tf.buf[0] = '\0';
    ret = vformat_file(&tf.file, "[%04x]", 0x2a);
    check_str("vfprintf custom stream", "[002a]", tf.buf);
    check_int("vfprintf custom stream ret", 6, ret);
    check_int("auto flush default off", 0, tf.flush_count);

    ret = fflush(&tf.file);
    check_int("explicit fflush ret", 0, ret);
    check_int("explicit fflush count", 1, tf.flush_count);

    if (failures) {
        fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }

    puts("microcrt printf extended passed");
    return 0;
}
