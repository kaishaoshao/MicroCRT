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
#include "string.h"
#include "errno.h"

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/

/*********************************************************************
*
*       __SEGGER_RTL_Read()
*
*  Function description
*    Read file.
*
*  Parameters
*    stream - Pointer to file to read from.
*    ptr    - Pointer to object to write to.
*    n      - Number of bytes to read.
*
*  Return value
*    < 0  - Error.
*    >= 0 - Number of bytes read.
*/
static int __SEGGER_RTL_Read(FILE *stream, void *ptr, size_t n) {
  return __SEGGER_RTL_X_file_read(stream, ptr, n);
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

/*********************************************************************
*
*       ungetc()
*
*  Function description
*    Push character back to file.
*
*  Parameters
*    c      - Character to push back to file.
*    stream - File to push character to.
*
*  Additional information
*    This function pushes the character c back to the file stream so
*    that it can be read again.  If c is EOF, the function fails and
*    EOF is returned.  One character of pushback is guaranteed; if more
*    than one character is pushed back without an intervening read,
*    the pushback may fail.
*
*  Return value
*    == EOF - Failed to push character back.
*    != EOF - The character pushed back to the file.
*
*  Thread safety
*    Unsafe.
*/
int ungetc(int c, FILE *stream) {
  return __SEGGER_RTL_X_file_unget(stream, c);
}

/*********************************************************************
*
*       fgetc()
*
*  Function description
*    Read character from file.
*
*  Parameters
*    stream - Pointer to file to read from.
*
*  Additional information
*   If the end-of-file indicator for the input stream pointed to by
*   stream is not set and a next character is present, obtain that
*   character as an unsigned char converted to an int and advance the
*   associated  file  position.
*
*  Return value
*    If the end-of-file indicator for the stream is set, or if the
*    stream is at end of file, the end-of-file indicator for the file
*    is set and the fgetc function returns EOF. Otherwise, return the
*    next character from the file pointed to by stream. If a read
*    error occurs, the error indicator for the stream is set and
*    return EOF.
*
*  Thread safety
*    Unsafe.
*/
int __SEGGER_RTL_PUBLIC_API fgetc(FILE *stream) {
  unsigned char c;
  int           stat;
  //
  stat = __SEGGER_RTL_Read(stream, &c, 1);
  if (stat <= 0) {
    return EOF;
  } else {
    return c;
  }
}
  
/*********************************************************************
*
*       getchar()
*
*  Function description
*    Read character from standard input.
*
*  Additional information
*    Reads a single character from the standard input stream.
*
*  Return value
*    If the stream is at end-of-file or a read error occurs, returns EOF,
*    otherwise a nonnegative value.
*
*  Thread safety
*    Unsafe.
*/
int __SEGGER_RTL_PUBLIC_API getchar(void) {
  return fgetc(stdin);
}

/*********************************************************************
*
*       getc()
*
*  Function description
*    Read character from stream.
*
*  Additional information
*    Reads a single character from a stream.
*
*  Parameters
*    stream - Pointer to file to read from.
*
*  Return value
*    If the stream is at end-of-file or a read error occurs, returns EOF,
*    otherwise a nonnegative value.
*
*  Thread safety
*    Unsafe.
*/
int __SEGGER_RTL_PUBLIC_API getc(FILE *stream) {
  return fgetc(stream);
}

/*********************************************************************
*
*       fgets()
*
*  Function description
*    Read string from stream.
*
*  Parameters
*    s      - Pointer to object to write to.
*    n      - Number of bytes to read.
*    stream - Pointer to file to read from.
*
*  Additional information
*    Reads at most one less than the number of characters specified by n
*    from the file pointed to by stream into the array pointed to
*    by s.  No additional characters are read after a newline character
*    (which is retained) or after end of file.  A null character is
*    written immediately after the last character read into the array.
*
*  Return value
*    Returns s if successful. If end-of-file is encountered and no
*    characters have been read into the array, the contents of the
*    array remain unchanged and a null pointer is returned. If a read
*    error occurs during the operation, the array contents are
*    indeterminate and a null pointer is returned.
*
*  Thread safety
*    Unsafe.
*/
char * __SEGGER_RTL_PUBLIC_API fgets(char *s, int n, FILE *stream) {
  char * pOut;
  int    c;
  //
  if (__SEGGER_RTL_X_file_stat(stream) < 0) {
    return NULL;
  }
  if (n == 0) {
    return NULL;
  }
  pOut = s;
  while (n > 1) {
    c = fgetc(stream);
    if (c < 0) {
      return NULL;
    }
    *pOut++ = c;
    --n;
    if (c == '\n') {
      break;
    }
  }
  *pOut = 0;
  return s;
}

/*********************************************************************
*
*       gets()
*
*  Function description
*    Read string from standard input.
*
*  Parameters
*    s - Pointer to object that receives the string.
*
*  Additional information
*    This function reads characters from standard input into the
*    array pointed to by s until end-of-file is encountered or a
*    newline character is read. Any newline character is discarded,
*    and a null character is written immediately after the last
*    character read into the array.
*
*  Return value
*    Returns s if successful. If end-of-file is encountered and no
*    characters have been read into the array, the contents of the
*    array remain unchanged and a null pointer is returned. If a read
*    error occurs during the operation, the array contents are
*    indeterminate and a null null pointer is return.
*
*  Thread safety
*    Unsafe.
*/
char * __SEGGER_RTL_PUBLIC_API gets(char *s) {
  char *a = s;
  //
  for (;;) {
    int ch = getchar();
    if (ch == EOF) {
      if (s == a) {
        return NULL;
      }
      a = NULL;
      break;
    }
    if (ch == '\n') {
      break;
    }
    *s++ = (char)ch;
  }
  *s = 0;
  return a;
}

/*********************************************************************
*
*       putchar()
*
*  Function description
*    Write character to standard output.
*
*  Parameters
*    c - Character to write.
*
*  Additional information
*    Writes the character c to the standard output stream. 
*
*  Return value
*    If no error, the character written. If a write error occurs,
*    returns EOF.
*
*  Thread safety
*    Unsafe.
*/
int __SEGGER_RTL_PUBLIC_API putchar(int c) {
  return fputc(c, stdout);
}

/*********************************************************************
*
*       putc()
*
*  Function description
*    Write character to file.
*
*  Parameters
*    c      - Character to write.
*    stream - Pointer to stream to write to.
*
*  Additional information
*    Writes the character c to stream. 
*
*  Return value
*    If no error, the character written. If a write error occurs,
*    returns EOF.
*
*  Thread safety
*    Unsafe.
*/
int __SEGGER_RTL_PUBLIC_API putc(int c, FILE *stream) {
  return fputc(c, stream);
}

/*********************************************************************
*
*       puts()
*
*  Function description
*    Write string to standard output.
*
*  Parameters
*    s - Pointer to zero-terminated string.
*
*  Additional information
*    Writes the string pointed to by s to the standard output 
*    stream using putchar() and appends a newline character to
*    the output.  The terminating null character is not written.
*
*  Return value
*    Returns EOF if a write error occurs; otherwise it returns 
*    a nonnegative value.
*
*  Thread safety
*    Unsafe.
*/
int __SEGGER_RTL_PUBLIC_API puts(const char *s) {
  if (__SEGGER_RTL_X_file_write(stdout, s, (strlen)(s)) == EOF) {
    return EOF;
  } else {
    return putchar('\n');
  }
}

/*********************************************************************
*
*       fputs()
*
*  Function description
*    Write string to standard output.
*
*  Parameters
*    s      - Pointer to zero-terminated string.
*    stream - Pointer to file to write to.
*
*  Additional information
*    Write the string  pointed to by s to the file pointed to by
*    stream.  The terminating null character is not written.
*
*  Return value
*    Returns EOF if a write error occurs; otherwise returns
*    a nonnegative value.
*
*  Thread safety
*    Unsafe.
*/
int __SEGGER_RTL_PUBLIC_API fputs(const char *s, FILE *stream) {
  size_t len;
  //
  len = (strlen)(s);
  if (fwrite(s, sizeof(char), len, stream) == len) {
    return 0;
  } else {
    return EOF;
  }
}

/*********************************************************************
*
*       fputc()
*
*  Function description
*    Write character to file.
*
*  Parameters
*    c      - Character to write.
*    stream - Pointer to file to write to.
*
*  Additional information
*    Writes the character c to stream. 
*
*  Return value
*    If no error, the character written. If a write error occurs,
*    returns EOF.
*
*  Thread safety
*    Unsafe.
*/
int __SEGGER_RTL_PUBLIC_API fputc(int c, FILE * stream) {
  unsigned char uc;
  //
  uc = (unsigned char)c;
  if (fwrite(&uc, sizeof(char), 1, stream) == 0) {
    return EOF;
  } else {
    return uc;
  }
}

/*********************************************************************
*
*       clearerr()
*
*  Function description
*    Clear error and end-of-file indicator on file.
*
*  Parameters
*    stream - Pointer to file to clear indicators on.
*
*  Thread safety
*    Unsafe.
*/
void __SEGGER_RTL_PUBLIC_API clearerr(FILE *stream) {
  __SEGGER_RTL_X_file_clrerr(stream);
}

/*********************************************************************
*
*       fopen()
*
*  Function description
*    Open file.
*
*  Parameters
*    filename - Pointer to zero-terminated file name.
*    mode     - Pointer to zero-terminated file mode.
*
*  Return value
*    == NULL - File not opened.
*    != NULL - File opened.
*
*  Thread safety
*    Unsafe.
*/
FILE * __SEGGER_RTL_PUBLIC_API fopen(const char *filename, const char *mode) {
  return __SEGGER_RTL_X_file_open(filename, mode);
}

/*********************************************************************
*
*       fdopen()
*
*  Function description
*    Open file using descriptor.
*
*  Parameters
*    descriptor - Underlying OS descriptor to associate with file.
*    mode       - Pointer to zero-terminated file mode.
*
*  Return value
*    == NULL - File not opened.
*    != NULL - File opened.
*    
*  Conformance
*    POSIX.1-2008.
*
*  Thread safety
*    Unsafe.
*/
FILE * __SEGGER_RTL_PUBLIC_API fdopen(int descriptor, const char *mode) {
  return __SEGGER_RTL_X_file_dopen(descriptor, mode);
}

/*********************************************************************
*
*       fclose()
*
*  Function description
*    Close file.
*
*  Parameters
*    stream - Pointer to file to close.
*
*  Return value
*    == 0   - File successfully closed.
*    == EOF - File did not successfully close.
*
*  Thread safety
*    Unsafe.
*/
int __SEGGER_RTL_PUBLIC_API fclose(FILE *stream) {
  return __SEGGER_RTL_X_file_close(stream);
}

/*********************************************************************
*
*       ferror()
*
*  Function description
*    Test error indicator.
*
*  Parameters
*    stream - Pointer to file to test.
*
*  Return value
*    == 0 - No error on file.
*    != 0 - Error on file.
*
*  Thread safety
*    Unsafe.
*/
int __SEGGER_RTL_PUBLIC_API ferror(FILE *stream) {
  return __SEGGER_RTL_X_file_error(stream);
}

/*********************************************************************
*
*       feof()
*
*  Function description
*    Test end-of-file indicator.
*
*  Parameters
*    stream - Pointer to file to test.
*
*  Return value
*    == 0 - No end-of-file on file.
*    != 0 - End-of-file on file.
*
*  Thread safety
*    Unsafe.
*/
int __SEGGER_RTL_PUBLIC_API feof(FILE *stream) {
  return __SEGGER_RTL_X_file_eof(stream);
}

/*********************************************************************
*
*       fflush()
*
*  Function description
*    Flush file.
*
*  Parameters
*    stream - Pointer to file to flush, or NULL, indicating all files.
*
*  Additional information
*    If stream points to file in write or update mode where the most-recent
*    operation was not input, any unwritten data for that file is delivered
*    to the host environment to be written; otherwise, the behavior is
*    undefined.
*
*  Return value
*    == 0   - File (or all files) successfully flushed.
*    != EOF - Error flushing one or more files.
*
*  Thread safety
*    Unsafe.
*/
int __SEGGER_RTL_PUBLIC_API fflush(FILE *stream) {
  return __SEGGER_RTL_X_file_flush(stream);
}

/*********************************************************************
*
*       rewind()
*
*  Function description
*    Rewind file.
*
*  Parameters
*    stream - Pointer to file to rewind.
*
*  Additional information
*    Sets the file position to start of file.
*
*  Thread safety
*    Unsafe.
*/
void __SEGGER_RTL_PUBLIC_API rewind(FILE *stream) {
  fseek(stream, 0, SEEK_SET);
}

/*********************************************************************
*
*       fread()
*
*  Function description
*    Read from file.
*
*  Parameters
*    ptr    - Pointer to object to write to.
*    size   - Size of each element to read.
*    nmemb  - Number of elements to read.
*    stream - Pointer to file to read from.
*
*  Return value
*    The number of elements successfully read, which may be less than
*    nmemb if a read error or end-of-file is encountered.
*
*  Additional information
*    If size or nmemb is zero, fread() returns zero and the contents
*    of the array and the state of the stream  remain unchanged.
*
*  Thread safety
*    Unsafe.
*/
size_t __SEGGER_RTL_PUBLIC_API fread(void *ptr, size_t size, size_t nmemb, FILE *stream) {
  size_t n;
  //
  // If file is closed, don't try to read from it.
  //
  if (__SEGGER_RTL_X_file_stat(stream) < 0) {
    return NULL;
  }
  //
  // Calculate number of objects to read.  On overflow, clamp to maximum.
  //
  n = size * nmemb;
  if (n == 0) {
    return 0;
  } else if (n < size) {
    n = (size_t)~0 / size;
  }
  //
  return __SEGGER_RTL_X_file_read(stream, ptr, n) / size;
}

/*********************************************************************
*
*       fwrite()
*
*  Function description
*    Write to file.
*
*  Parameters
*    ptr    - Pointer to data to write.
*    size   - Size of each element to write.
*    nmemb  - Number of elements to write.
*    stream - Pointer to file to write to.
*
*  Return value
*    The number of elements successfully written, which may be less than
*    nmemb if a read error or end-of-file is encountered.
*
*  Additional information
*    If size or nmemb is zero, fwrite() returns zero and the contents
*    of the array and the state of the stream  remain unchanged.
*
*  Thread safety
*    Unsafe.
*/
size_t __SEGGER_RTL_PUBLIC_API fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream) {
  size_t n;
  //
  if (__SEGGER_RTL_X_file_stat(stream) < 0) {
    return NULL;
  }
  n = size*nmemb;
  if (n < size) {
    return 0;
  } else {
    return __SEGGER_RTL_X_file_write(stream, ptr, n) / size;
  }
}

/*********************************************************************
*
*       fseek()
*
*  Function description
*    Set file position.
*
*  Parameters
*    stream - Pointer to file to position.
*    offset - Offset relative to anchor specified by whence.
*    whence - Where offset is relative to.
*
*  Return value
*    == 0 - Position is set.
*    != 0 - Position is not set.
*
*  Thread safety
*    Unsafe.
*/
int __SEGGER_RTL_PUBLIC_API fseek(FILE *stream, long offset, int whence) {
  return __SEGGER_RTL_X_file_seek(stream, offset, whence);
}
  
/*********************************************************************
*
*       fgetpos()
*
*  Function description
*    Get file position.
*
*  Parameters
*    stream - Pointer to file to position.
*    pos    - Pointer to object that receives the position.
*
*  Return value
*    == 0 - Position retrieved successfully.
*    != 0 - Position not retrieved successfully; errno set to ESPIPE.
*
*  Thread safety
*    Unsafe.
*/
int __SEGGER_RTL_PUBLIC_API fgetpos(FILE *stream, fpos_t *pos) {
  int stat;
  //
  stat = __SEGGER_RTL_X_file_getpos(stream, pos);
  if (stat != 0) {
    errno = ESPIPE;
  }
  return stat;
}

/*********************************************************************
*
*       fsetpos()
*
*  Function description
*    Set file position.
*
*  Parameters
*    stream - Pointer to file to position.
*    pos    - Pointer to position.
*
*  Additional information
*    Sets the file position to pos which was previously retrieved
*    using fgetpos().
*
*  Return value
*    == 0 - Position set successfully.
*    != 0 - Position not set successfully; errno set to ESPIPE.
*
*  Thread safety
*    Unsafe.
*/
int __SEGGER_RTL_PUBLIC_API fsetpos(FILE *stream, const fpos_t *pos) {
  int stat;
  //
  stat = __SEGGER_RTL_X_file_seek(stream, *pos, SEEK_SET);
  if (stat != 0) {
    errno = ESPIPE;
  }
  return stat;
}

/*********************************************************************
*
*       ftell()
*
*  Function description
*    Get file position.
*
*  Parameters
*    stream - Pointer to file.
*
*  Additional information
*    Sets the file position to pos which was previously retrieved
*    using fgetpos().
*
*  Return value
*    == 0 - Position set successfully.
*    != 0 - Position not set successfully; errno set to ESPIPE.
*
*  Thread safety
*    Unsafe.
*/
long __SEGGER_RTL_PUBLIC_API ftell(FILE *stream) {
  long pos;
  //
  if (__SEGGER_RTL_X_file_getpos(stream, &pos) < 0) {
    errno = ESPIPE;
    return -1L;
  } else {
    return pos;
  }
}

/*********************************************************************
*
*       freopen()
*
*  Function description
*    Reopen file.
*
*  Parameters
*    filename - Pointer to zero-terminated file name.
*    mode     - Pointer to zero-terminated file mode.
*    stream   - Pointer to file to reopen.
*
*  Return value
*    == NULL - File not reopened.
*    != NULL - File reopened.
*
*  Thread safety
*    Unsafe.
*/
FILE * __SEGGER_RTL_PUBLIC_API freopen(const char *filename, const char *mode, FILE *stream) {
  __SEGGER_RTL_USE_PARA(filename);
  __SEGGER_RTL_USE_PARA(mode);
  __SEGGER_RTL_USE_PARA(stream);
  //
  return NULL;
}

/*********************************************************************
*
*       tmpnam()
*
*  Function description
*    Generate name for temporary file.
*
*  Parameters
*    s - Pointer to object that receives the temporary file name,
*        or NULL indicating that a (shared) internal buffer is used
*        for the temporary name.
*
*  Return value
*    == NULL - Cannot generate a unique temporary name.
*    != NULL - Pointer to temporary name generated.
*
*  Thread safety
*    Unsafe.
*/
char * __SEGGER_RTL_PUBLIC_API tmpnam(char *s) {
  return __SEGGER_RTL_X_file_tmpnam(s, L_tmpnam);
}

/*********************************************************************
*
*       tmpfile()
*
*  Function description
*    Generate temporary file.
*
*  Return value
*    == NULL - Cannot generate a unique temporary file.
*    != NULL - Pointer to temporary file.
*
*  Thread safety
*    Unsafe.
*/
FILE * __SEGGER_RTL_PUBLIC_API tmpfile(void) {
  return __SEGGER_RTL_X_file_tmpfile();
}

/*********************************************************************
*
*       rename()
*
*  Function description
*    Rename file.
*
*  Parameters
*    oldname - Pointer to string denoting old file name.
*    newname - Pointer to string denoting new file name.
*
*  Return value
*    == 0 - Rename succeeded.
*    != 0 - Rename failed.
*
*  Thread safety
*    Unsafe.
*/
int __SEGGER_RTL_PUBLIC_API rename(const char *oldname, const char *newname) {
  return __SEGGER_RTL_X_file_rename(oldname, newname);
}

/*********************************************************************
*
*       remove()
*
*  Function description
*    Remove file.
*
*  Parameters
*    filename - Pointer to string denoting file name to remove.
*
*  Return value
*    == 0 - Remove succeeded.
*    != 0 - Remove failed.
*
*  Thread safety
*    Unsafe.
*/
int __SEGGER_RTL_PUBLIC_API remove(const char *filename) {
  return __SEGGER_RTL_X_file_remove(filename);
}

/*********************************************************************
*
*       perror()
*
*  Function description
*    Print error message to standard error stream.
*
*  Parameters
*    s - Pointer to string to prefix error with.  May be NULL.
*
*  Thread safety
*    Unsafe.
*/
void __SEGGER_RTL_PUBLIC_API perror(const char *s) {
  if (s && s[0]) {
    fputs(s, stderr);
    fputs(": ", stderr);
  }
  fputs(strerror(errno), stderr);
  fputs("\n", stderr);
}

int __SEGGER_RTL_PUBLIC_API setvbuf(FILE *stream, char *buf, int mod, size_t size) {
  __SEGGER_RTL_USE_PARA(stream);
  __SEGGER_RTL_USE_PARA(buf);
  __SEGGER_RTL_USE_PARA(mod);
  __SEGGER_RTL_USE_PARA(size);
  //
  return 0;
}

void __SEGGER_RTL_PUBLIC_API setbuf(FILE *stream, char *buf) {
  __SEGGER_RTL_USE_PARA(stream);
  __SEGGER_RTL_USE_PARA(buf);
  //
  /* Nothing to do */
}

/*************************** End of file ****************************/
