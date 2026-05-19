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
#include "SEMIHOST/SEGGER_SEMIHOST.h"

/*********************************************************************
*
*       Defines, fixed
*
**********************************************************************
*/

#define MAX_FILES   10
#define MAX_TMPNAM  L_tmpnam                    // Maximum number of characters in a temporary file name

/*********************************************************************
*
*       Local types
*
**********************************************************************
*/

struct __SEGGER_RTL_FILE_impl {
  int    Handle;
  int    Mode;
  int    Eof;
  int    Temp;     // Nonzero -> delete this index on close
  int    Len;      // Should be fpos_t, but semihosting punts to int
  char   Ungot;
  char   UngotCh;
  fpos_t Pos;
};

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

static __SEGGER_RTL_FILE _aFileTable[MAX_FILES] = {
  { SEGGER_SEMIHOST_STDIN,  SYS_FILE_MODE_READBINARY,  0, 0, 0, 0, 0, 0 },
  { SEGGER_SEMIHOST_STDOUT, SYS_FILE_MODE_WRITEBINARY, 0, 0, 0, 0, 0, 0 },
  { SEGGER_SEMIHOST_ERROUT, SYS_FILE_MODE_WRITEBINARY, 0, 0, 0, 0, 0, 0 },
};

static unsigned char __SEGGER_RTL_TmpNamIndex;

/*********************************************************************
*
*       Public data
*
**********************************************************************
*/

__SEGGER_RTL_FILE *stdin  = &_aFileTable[SEGGER_SEMIHOST_STDIN ];
__SEGGER_RTL_FILE *stdout = &_aFileTable[SEGGER_SEMIHOST_STDOUT];
__SEGGER_RTL_FILE *stderr = &_aFileTable[SEGGER_SEMIHOST_ERROUT];

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/

/*********************************************************************
*
*       _SetErrno()
*
*  Function description
*    Set C library errno.
*
*  Parameters
*    err - Error indication (0 is no error, nonzero is error).
*/
static void _SetErrno(int err) {
  if (err) {
    errno = SEGGER_SEMIHOST_Errno();
  }
}

/*********************************************************************
*
*       _GetIndex()
*
*  Function description
*    Get file table index.
*
*  Parameters
*    stream - Pointer to file.
*
*  Return value
*    Index for stream, -1 on invalid.
*/
static int _GetIndex(__SEGGER_RTL_FILE *stream) {
  if (&_aFileTable[0] <= stream && stream < &_aFileTable[MAX_FILES]) {
    return stream - &_aFileTable[0];
  } else {
    return EOF;
  }
}

/*********************************************************************
*
*       _GetNewIndex()
*
*  Function description
*    Get new file table index.
*
*  Return value
*    Index for new stream, -1 if none.
*/
static int _GetNewIndex(void) {
  int i;
  //
  for (i = 3; i < MAX_FILES; ++i) {
    if (_aFileTable[i].Handle <= 0) {
      return i;
    }
  }
  return EOF;
}

/*********************************************************************
*
*       _Advance()
*
*  Function description
*    Advance file position.
*
*  Parameters
*    stream - Pointer to file to advance.
*    chars  - Number of characters to advance by.
*/
static void _Advance(__SEGGER_RTL_FILE *stream, int chars) {
  //
  // Advance file position.
  //
  stream->Pos += chars;
  //
  // Record changes in file size.
  //
  if (stream->Mode == SYS_FILE_MODE_WRITEBINARY) {
    if (stream->Pos > stream->Len) {
      stream->Len = stream->Pos;
    }
  }
}

/*********************************************************************
*
*       Public code
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
  if (_GetIndex(stream) < 0) {
    return EOF;
  } else {
    return stream->Handle;
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
  __SEGGER_RTL_USE_PARA(stream);
  //
  return 80;
}

/*********************************************************************
*
*       __SEGGER_RTL_X_file_close()
*
*  Function description
*    Close file.
*
*  Parameters
*    stream - Pointer to file.
*
*  Additional information
*    Close the file stream.  If the stream is connected to a temporary
*    file (by use of tmpfile()), the temporary file is deleted.
*
*  Return value
*    <  0 - Failure, stream is already closed.
*    >= 0 - Success, stream is closed.
*/
int __SEGGER_RTL_X_file_close(__SEGGER_RTL_FILE *stream) {
  char aTmpNam[MAX_TMPNAM];
  int  r;
  //
  if (__SEGGER_RTL_X_file_stat(stream) < 0) {
    return EOF;
  }
  //
  r = SEGGER_SEMIHOST_Close(stream->Handle);
  _SetErrno(r);
  stream->Handle = -1;
  //
  if (stream->Temp) {
    if (SEGGER_SEMIHOST_TmpName(aTmpNam, stream->Temp, sizeof(aTmpNam)) == 0) {
      remove(aTmpNam);
    }
  }
  //
  return r;
}

/*********************************************************************
*
*       __SEGGER_RTL_X_file_end()
*
*  Function description
*    Test for end-of-file condition.
*
*  Parameters
*    stream - Pointer to file.
*
*  Return value
*    <  0 - Failure, stream is closed.
*    == 0 - Success, stream is not at end of file.
*    >  0 - Success, stream is at end of file.
*/
int __SEGGER_RTL_X_file_end(__SEGGER_RTL_FILE *stream) {
  if (__SEGGER_RTL_X_file_stat(stream) < 0) {
    return EOF;
  } else {
    return stream->Eof;
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_X_file_error()
*
*  Function description
*    Test for file-error condition.
*
*  Parameters
*    stream - Pointer to file.
*
*  Return value
*    <  0 - Failure, stream is closed.
*    == 0 - Success, stream is not in error.
*    >  0 - Success, stream is in error.
*/
int __SEGGER_RTL_X_file_error(__SEGGER_RTL_FILE *stream) {
  if (__SEGGER_RTL_X_file_stat(stream) < 0) {
    return EOF;
  } else {
    return 0;
  }
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
  if (stream == NULL) {
    return 0;
  } else if (__SEGGER_RTL_X_file_stat(stream) < 0) {
    return EOF;
  } else {
    return 0;
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_X_file_getpos()
*
*  Function description
*    Get file position.
*
*  Parameters
*    stream - Pointer to file.
*    pos    - Pointer to object that receives the position.
*
*  Return value
*    == 0 - Position retrieved successfully.
*    <  0 - Position not retrieved successfully.
*/
int __SEGGER_RTL_X_file_getpos(__SEGGER_RTL_FILE *stream, fpos_t *pos) {
  if (__SEGGER_RTL_X_file_stat(stream) < 0) {
    return EOF;
  } else {
    *pos = stream->Pos;
    return 0;
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_X_file_clrerr()
*
*  Function description
*    Clear file-error status.
*
*  Parameters
*    stream - Pointer to file.
*/
void __SEGGER_RTL_X_file_clrerr(__SEGGER_RTL_FILE *stream) {
  __SEGGER_RTL_USE_PARA(stream);
}

/*********************************************************************
*
*       __SEGGER_RTL_X_file_open()
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
*/
__SEGGER_RTL_FILE *__SEGGER_RTL_X_file_open(const char *filename, const char *mode) {
  int                 Index;
  __SEGGER_RTL_FILE * stream;
  //
  Index = _GetNewIndex();
  if (Index < 0) {
    return NULL;
  }
  //
  stream = &_aFileTable[Index];
  stream->Mode = (strchr)(mode, 'r') == NULL
                   ? SYS_FILE_MODE_WRITEBINARY
                   : SYS_FILE_MODE_READBINARY;
  stream->Handle = SEGGER_SEMIHOST_Open(filename, stream->Mode, (strlen)(filename));
  if (stream->Handle >= 0) {
    stream->Pos   = 0;
    stream->Eof   = 0;
    stream->Temp  = 0;
    stream->Len   = SEGGER_SEMIHOST_FLen(stream->Handle);
    stream->Ungot = 0;
    return stream;
  } else {
    return NULL;
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_X_file_dopen()
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
*/
__SEGGER_RTL_FILE *__SEGGER_RTL_X_file_dopen(int descriptor, const char *mode) {
  int                 Index;
  __SEGGER_RTL_FILE * stream;
  switch (descriptor) {
  case SEGGER_SEMIHOST_STDIN:
  case SEGGER_SEMIHOST_STDOUT:
  case SEGGER_SEMIHOST_ERROUT:
    Index = _GetNewIndex();
    break;
  default:
    Index = -1;
  }
  //
  if (Index < 0) {
    return NULL;
  }
  //
  stream = &_aFileTable[Index];
  stream->Mode = (strchr)(mode, 'r') == NULL
                   ? SYS_FILE_MODE_WRITEBINARY
                   : SYS_FILE_MODE_READBINARY;
  stream->Handle = descriptor;
  if (stream->Handle >= 0) {
    stream->Pos   = 0;
    stream->Eof   = 0;
    stream->Temp  = 0;
    stream->Len   = 0;
    stream->Ungot = 0;
    return stream;
  } else {
    return NULL;
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_X_file_seek()
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
*/
int __SEGGER_RTL_X_file_seek(__SEGGER_RTL_FILE *stream, long offset, int whence) {
  int r;
  //
  if (__SEGGER_RTL_X_file_stat(stream) < 0) {
    return EOF;
  }
  //
  switch (whence) {
  case SEEK_END:  offset = stream->Len + offset; break;
  case SEEK_CUR:  offset = stream->Pos + offset; break;
  case SEEK_SET:                                 break;
  default:        return EOF;
  }
  //
  stream->Ungot = 0;
  //
  if (offset < 0) {
    offset = 0;
  } else if (offset > stream->Len) {
    offset = stream->Len;
  }
  //
  r = SEGGER_SEMIHOST_Seek(stream->Handle, offset);
  if (r < 0) {
    _SetErrno(r);
  } else {
    stream->Pos = offset;
  }
  return r;
}

/*********************************************************************
*
*       __SEGGER_RTL_X_file_remove()
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
*/
int __SEGGER_RTL_X_file_remove(const char *filename) {
  return SEGGER_SEMIHOST_Remove(filename, (strlen)(filename));
}

/*********************************************************************
*
*       __SEGGER_RTL_X_file_rename()
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
*/
int __SEGGER_RTL_X_file_rename(const char *oldname, const char *newname) {
  return SEGGER_SEMIHOST_Rename(oldname, (strlen)(oldname), newname, (strlen)(newname));
}

/*********************************************************************
*
*       __SEGGER_RTL_X_file_tmpnam()
*
*  Function description
*    Generate name for temporary file.
*
*  Parameters
*    s   - Pointer to object that receives the temporary file name,
*          or NULL indicating that a (shared) internal buffer is used
*          for the temporary name.
*    max - Maxumum number of characters acceptable in the object s.
*
*  Return value
*    == NULL - Cannot generate a unique temporary name.
*    != NULL - Pointer to temporary name generated.
*/
char *__SEGGER_RTL_X_file_tmpnam(char *s, unsigned max) {
  //
  // Try to construct a temporary name.
  //
  ++__SEGGER_RTL_TmpNamIndex;
  if (__SEGGER_RTL_TmpNamIndex == 0) {
    __SEGGER_RTL_TmpNamIndex = 1;
  }
  if (SEGGER_SEMIHOST_TmpName(s, __SEGGER_RTL_TmpNamIndex, max) == 0) {
    return s;
  } else {
    return NULL;
  }
}

/*********************************************************************
*
*       __SEGGER_RTL_X_file_tmpfile()
*
*  Function description
*    Generate temporary file.
*
*  Return value
*    == NULL - Cannot generate a unique temporary file.
*    != NULL - Pointer to temporary file.
*/
__SEGGER_RTL_FILE *__SEGGER_RTL_X_file_tmpfile(void) {
  __SEGGER_RTL_FILE * stream;
  char   aTmpNam[MAX_TMPNAM];
  //
  if (tmpnam(aTmpNam) == NULL) {
    return NULL;
  }
  stream = fopen(aTmpNam, "wb");
  if (stream != NULL) {
    stream->Temp = __SEGGER_RTL_TmpNamIndex;
  }
  //
  return stream;
}

/*********************************************************************
*
*       __SEGGER_RTL_X_file_read()
*
*  Function description
*    Read from file.
*
*  Parameters
*    stream - Pointer to file to read from.
*    s      - Pointer to object to write to.
*    len    - Number of characters to read.
*
*  Return value
*    The number of characters successfully read, which may be less than
*    len if a read error or end-of-file is encountered.
*/
int __SEGGER_RTL_X_file_read(__SEGGER_RTL_FILE *stream, char *s, unsigned len) {
  int n;
  int chars;
  //
  if (__SEGGER_RTL_X_file_stat(stream) < 0) {
    return EOF;
  }
  if (len == 0) {
    return 0;
  }
  //
  chars = 0;
  if (len > 0) {
    if (stream->Ungot) {
      *s = stream->UngotCh;
      stream->Ungot = 0;
      stream->Pos  += 1;
      ++s;
      --len;
      ++chars;
    }
  }
  n = SEGGER_SEMIHOST_Read(stream->Handle, s, len);
  if (n < 0) {
    n = 0;
  } else {
    n      = len - n;
    chars += n;
  }
  _Advance(stream, n);
  return chars;
}

/*********************************************************************
*
*       __SEGGER_RTL_X_file_write()
*
*  Function description
*    Read from file.
*
*  Parameters
*    stream - Pointer to file to read from.
*    s      - Pointer to object to write.
*    len    - Number of characters to write.
*
*  Return value
*    The number of characters successfully written, which may be less than
*    len if an error occurs.
*/
int __SEGGER_RTL_X_file_write(__SEGGER_RTL_FILE *stream, const char *s, unsigned len) {
  int n;
  //
  if (__SEGGER_RTL_X_file_stat(stream) < 0) {
    return EOF;
  }
  if (len == 0) {
    return 0;
  }
  n = SEGGER_SEMIHOST_Write(stream->Handle, s, len);
  if (n != 0) {
    return 0;
  }
  _Advance(stream, len);
  return len;
}

/*********************************************************************
*
*       __SEGGER_RTL_X_file_eof()
*
*  Function description
*    Is end-of-file flag set?
*
*  Parameters
*    stream - Pointer to file.
*
*  Return value
*    == 0 - End-of-file flag not set.
*    != 0 - End-of-file flag set.
*/
int __SEGGER_RTL_X_file_eof(__SEGGER_RTL_FILE *stream) {
  return stream->Pos >= stream->Len;
}

/*********************************************************************
*
*       __SEGGER_RTL_X_file_unget()
*
*  Function description
*    Push character back to file.
*
*  Parameters
*    stream - File to push character to.
*    c      - Character to push back to file.
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
*/
int __SEGGER_RTL_X_file_unget(__SEGGER_RTL_FILE *stream, int c) {
  if (__SEGGER_RTL_X_file_stat(stream) < 0) {
    return EOF;
  }
  if (stream->Ungot || c == EOF) {
    return EOF;
  }
  stream->Ungot   = 1;
  stream->UngotCh = c;
  stream->Pos    -= 1;
  return c & 0xFF;
}

/*************************** End of file ****************************/

int fileno(FILE *stream)
{
  return _GetIndex(stream);
}

#include <sys/stat.h>

#define	S_IRWXU (S_IRUSR | S_IWUSR | S_IXUSR)
#define	S_IRUSR	0000400
#define	S_IWUSR	0000200
#define	S_IXUSR 0000100
#define	S_IRWXG	(S_IRGRP | S_IWGRP | S_IXGRP)
#define	S_IRGRP	0000040
#define	S_IWGRP	0000020
#define	S_IXGRP 0000010
#define	S_IRWXO	(S_IROTH | S_IWOTH | S_IXOTH)
#define	S_IROTH	0000004
#define	S_IWOTH	0000002
#define	S_IXOTH 0000001

#define	S_IREAD	S_IRUSR
#define	S_IWRITE S_IWUSR
int fstat(int fildes, struct stat *buf)
{
  buf->st_mode = S_IREAD | S_IWRITE | S_IRWXU | S_IRWXG | S_IRWXO; // fixme
  buf->st_size = SEGGER_SEMIHOST_FLen(fildes);
  return 0;
}
