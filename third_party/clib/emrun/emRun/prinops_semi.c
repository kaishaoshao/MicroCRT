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
#include "SEMIHOST/SEGGER_SEMIHOST.h"

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

static FILE __SEGGER_RTL_stdin_file  = { SEGGER_SEMIHOST_STDIN  };
static FILE __SEGGER_RTL_stdout_file = { SEGGER_SEMIHOST_STDOUT };
static FILE __SEGGER_RTL_stderr_file = { SEGGER_SEMIHOST_ERROUT };
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

/*********************************************************************
*
*       __SEGGER_RTL_X_file_stat()
*
*  Function description
*    Get file status.
*
*  Parameters
*    stream - Pointer to file.
*
*  Additional information
*    Low-overhead test to determine if stream is valid.  If stream
*    is a valid pointer and the stream is open, this function must
*    succeed.  If stream is a valid pointer and the stream is closed,
*    this function must fail.
*
*    The implementation may optionally determine whether stream is
*    a valid pointer: this may not always be possible and is not
*    required, but may assist debugging when clients provide wild
*    pointers.
*
*  Return value
*    <  0 - Failure, stream is not a valid file.
*    >= 0 - Success, stream is a valid file.
*/
int __SEGGER_RTL_X_file_stat(__SEGGER_RTL_FILE *stream) {
  if (stream == stdin || stream == stdout || stream == stderr) {
    return 0;
  } else {
    return EOF;
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_X_file_bufsize()
*
*  Function description
*    Get stream buffer size.
*
*  Parameters
*    stream - Pointer to file.
*
*  Additional information
*    Returns the number of characters to use for buffered I/O on
*    the file stream.  The I/O buffer is allocated on the stack
*    for the duration of the I/O call, therefore this value should
*    not be set arbitrarily large.
*
*    For unbuffered I/O, return 1.
*
*  Return value
*    Nonzero number of characters to use for buffered I/O; for
*    unbuffered I/O, return 1.
*/
int __SEGGER_RTL_X_file_bufsize(__SEGGER_RTL_FILE *stream) {
#if defined(UNBUFFERED)
  return 1;
#else
  return 80;
#endif
}

/*********************************************************************
*
*       __SEGGER_RTL_stdin_getc()
*
*  Function description
*    Get character from standard input.
*
*  Return value
*    >= 0   - Character read.
*    == EOF - End of stream or error reading.
*
*  Additional information
*    This function never fails to deliver a character.
*/
static int __SEGGER_RTL_stdin_getc(void) {
  int  r;
  char c;
  //
  if (__SEGGER_RTL_stdin_ungot != EOF) {
    c = __SEGGER_RTL_stdin_ungot;
    __SEGGER_RTL_stdin_ungot = EOF;
    r = 0;
  } else {
    r = SEGGER_SEMIHOST_ReadC();
  }
  //
  return r < 0 ? EOF : c;
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

/*********************************************************************
*
*       __SEGGER_RTL_X_file_read()
*
*  Function description
*    Read data from file.
*
*  Parameters
*    stream - Pointer to file to read from.
*    s      - Pointer to object that receives the input.
*    len    - Number of characters to read from file.
*
*  Return value
*    >= 0 - Success.
*    <  0 - Failure.
*
*  Additional information
*    Reading from any stream other than stdin results in an error.
*/
int __SEGGER_RTL_X_file_read(__SEGGER_RTL_FILE * stream,
                             char              * s,
                             unsigned            len) {
  int c;
  //
  if (stream == stdin) {
    c = 0;
    while (len > 0) {
      *s++ = __SEGGER_RTL_stdin_getc();
      --len;
    }
  } else {
    c = SEGGER_SEMIHOST_Read(stream->handle, s, len);
  }
  //
  return c;
}

/*********************************************************************
*
*       __SEGGER_RTL_X_file_write()
*
*  Function description
*    Write data to file.
*
*  Parameters
*    stream - Pointer to file to write to.
*    s      - Pointer to object to write to file.
*    len    - Number of characters to write to the file.
*
*  Return value
*    >= 0 - Success.
*    <  0 - Failure.
*/
int __SEGGER_RTL_X_file_write(__SEGGER_RTL_FILE *stream, const char *s, unsigned len) {
  int r;
  //
  r = SEGGER_SEMIHOST_Write(stream->handle, s, len);
  if (r < 0) {
    r = EOF;
  }
  //
  return r;
}

/*********************************************************************
*
*       __SEGGER_RTL_X_file_unget()
*
*  Function description
*    Push character back to stream.
*
*  Parameters
*    stream - Pointer to stream to push back to.
*    c      - Character to push back.
*
*  Return value
*    >= 0 - Success.
*    <  0 - Failure.
*
*  Additional information
*    Push-back is only supported for standard input, and
*    only a single-character pushback buffer is implemented.
*/
int __SEGGER_RTL_X_file_unget(__SEGGER_RTL_FILE *stream, int c) {
  if (stream == stdin) {
    if (c != EOF && __SEGGER_RTL_stdin_ungot == EOF) {
      __SEGGER_RTL_stdin_ungot = c;
    } else {
      c = EOF;
    }
  } else {
    c = EOF;
  }
  //
  return c;
}

/*********************************************************************
*
*       __SEGGER_RTL_X_file_flush()
*
*  Function description
*    Flush unwritten data to file.
*
*  Parameters
*    stream - Pointer to file.
*
*  Return value
*    <  0 - Failure, file cannot be flushed or was not successfully flushed.
*    == 0 - Success, unwritten data is flushed.
*/
int __SEGGER_RTL_X_file_flush(__SEGGER_RTL_FILE *stream) {
  return 0;
}

/*************************** End of file ****************************/
