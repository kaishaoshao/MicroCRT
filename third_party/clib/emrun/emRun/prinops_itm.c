/*********************************************************************
*                   (c) SEGGER Microcontroller GmbH                  *
*                        The Embedded Experts                        *
*                           www.segger.com                           *
**********************************************************************

-------------------------- END-OF-HEADER -----------------------------
*/

/*********************************************************************
*
*       #include section
*
**********************************************************************
*/

#include "__SEGGER_RTL_Int.h"
#include "stdio.h"
#include <intrinsics.h>

/*********************************************************************
*
*       Local types
*
**********************************************************************
*/

struct __SEGGER_RTL_FILE_impl {
  int handle;
};

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

static FILE __SEGGER_RTL_stdin_file  = { 0 };  // stdin reads from RTT buffer #0
static FILE __SEGGER_RTL_stdout_file = { 0 };  // stdout writes to RTT buffer #0
static FILE __SEGGER_RTL_stderr_file = { 0 };  // stdout writes to RTT buffer #0
static int  __SEGGER_RTL_stdin_ungot = EOF;

/*********************************************************************
*
*       Public data
*
**********************************************************************
*/

FILE *stdin  = &__SEGGER_RTL_stdin_file;
FILE *stdout = &__SEGGER_RTL_stdout_file;
FILE *stderr = &__SEGGER_RTL_stderr_file;


/*********************************************************************
*
*       Static code
*
**********************************************************************
*/

#define ITM_RXBUFFER_EMPTY ((int32_t)0x5AA55AA5U)
volatile int32_t ITM_RxBuffer = ITM_RXBUFFER_EMPTY;

static char __SEGGER_RTL_stdin_getc(void)
{
  int x;
  while (ITM_RxBuffer == ITM_RXBUFFER_EMPTY);    
  x = ITM_RxBuffer;
  ITM_RxBuffer = ITM_RXBUFFER_EMPTY;
  return x;
}


/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

#if defined(INTERRUPTS_DISABLED)
#define DI() \
  unsigned pm = __get_PRIMASK();\
  if (pm==0)\
    __disable_interrupt()
#define EI() \
  if (pm==0) \
    __enable_interrupt()
#else
#define DI()
#define EI()
#endif

#define WAIT_ITM() while (*(volatile unsigned char *)0xE0000000 == 0)
#define OUTPUT_BYTE(B) *(volatile unsigned char *)0xE0000000 = (B)
#define OUTPUT_HALF(B0,B1) *(volatile unsigned short *)0xE0000000 = ((B0)|(B1)<<8)
#define OUTPUT_WORD(B0,B1,B2,B3) *(volatile unsigned int *)0xE0000000 = ((B0)|(B1)<<8|(B2)<<16|(B3)<<24)


int __SEGGER_RTL_X_file_read(__SEGGER_RTL_FILE *stream, char *s, unsigned len) 
{
  if (__SEGGER_RTL_stdin_ungot != EOF)
    {
      *s = __SEGGER_RTL_stdin_ungot;
      __SEGGER_RTL_stdin_ungot = EOF;
      len--;
    }
  int i;
  for (i=0;i<len;i++)    
    *s++ = __SEGGER_RTL_stdin_getc();
  return len;
}

int __SEGGER_RTL_X_file_flush(__SEGGER_RTL_FILE *stream) {

  __SEGGER_RTL_USE_PARA(stream);

  return 0;
}

int __SEGGER_RTL_X_file_write(__SEGGER_RTL_FILE *stream, const char *pData, unsigned DataLen) 
{
  int r=0;
  //
  DI();
  while (r < DataLen) {
    WAIT_ITM();
    if ((DataLen - r) >= 4)
      {
        OUTPUT_WORD(pData[r], pData[r+1], pData[r+2], pData[r+3]);
        r += 4;
      }
    else if ((DataLen - r) >= 2)
      {
        OUTPUT_HALF(pData[r], pData[r+1]);
        r += 2;
      }
    else
      {
        OUTPUT_BYTE(pData[r]);
        r++;
      }
  }  
  EI();  
  return r;
}

int __SEGGER_RTL_X_file_unget(__SEGGER_RTL_FILE *stream, int c) {
  __SEGGER_RTL_stdin_ungot = c;
  return c;
}

int __SEGGER_RTL_X_file_stat(__SEGGER_RTL_FILE *stream)
{
  return stream == stdin;
}

int __SEGGER_RTL_X_file_bufsize(__SEGGER_RTL_FILE *stream) {
#if defined(UNBUFFERED)
  return 1;
#else
  return 64;
#endif
}

/*************************** End of file ****************************/
