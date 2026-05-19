#include "__SEGGER_RTL_Int.h"
#include "stdio.h"
#include "SEMIHOST/SEGGER_SEMIHOST.h"

int printf(const char *format, ...) {
  va_list ap;
  va_start(ap, format);
  int n = SEGGER_SEMIHOST_Writef(format, &ap);
  va_end(ap);
  return n;
}

int vprintf(const char *format, va_list ap) { 
  return SEGGER_SEMIHOST_Writef(format, &ap); 
}

int putchar(int c)
{
  return SEGGER_SEMIHOST_WriteC(c);
}

int puts(const char *s)
{
  int res = SEGGER_SEMIHOST_Write0(s);
  if (res == 0)
    res = SEGGER_SEMIHOST_WriteC('\n');
  return res;
}
