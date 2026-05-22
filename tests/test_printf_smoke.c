#include <stdio.h>
#include <string.h>

static int failures;

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

int
main(void)
{
    char buf[128];
    int ret;
    int count = -1;

    ret = snprintf(buf, sizeof(buf), "%s %c %%", "world", 'A');
    check_str("basic", "world A %", buf);
    check_int("basic ret", 9, ret);

    ret = snprintf(buf, sizeof(buf), "%d %u %x %o", -12, 12u, 0x2a, 012);
    check_str("integer", "-12 12 2a 12", buf);
    check_int("integer ret", 12, ret);

    ret = snprintf(buf, sizeof(buf), "[%5d][%-5d][%05d]", 12, 12, 12);
    check_str("width", "[   12][12   ][00012]", buf);
    check_int("width ret", 21, ret);

    ret = snprintf(buf, sizeof(buf), "%.3s %.2f %.2e %.4g", "abcdef", 12.345, 12.345, 12.345);
    check_str("float", "abc 12.35 1.23e+01 12.35", buf);
    check_int("float ret", 24, ret);

    ret = snprintf(buf, sizeof(buf), "%p", (void *)0x1234);
    check_int("pointer prefix", 1, buf[0] == '0');

    ret = snprintf(buf, sizeof(buf), "abc%nXYZ", &count);
    check_str("percent n", "abcXYZ", buf);
    check_int("percent n ret", 6, ret);
    check_int("percent n count", 3, count);

    ret = printf("microcrt printf smoke: %d %s %.1f\n", 7, "ok", 1.5);
    if (ret <= 0)
        check_int("printf ret positive", 1, 0);

    if (failures) {
        fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }

    puts("microcrt printf smoke passed");
    return 0;
}
