#include "stdio_private.h"

int
__file_str_put(char c, FILE *stream)
{
    struct __file_str *sstream = (struct __file_str *) stream;

    if (sstream->pos != sstream->end)
        *sstream->pos++ = c;

    return (unsigned char) c;
}

int
__file_str_put_alloc(char c, FILE *stream)
{
    struct __file_str *sstream = (struct __file_str *) stream;

    if (sstream->pos == sstream->end)
        return EOF;

    *sstream->pos++ = c;
    return (unsigned char) c;
}

int
fflush(FILE *stream)
{
    int ret = 0;

    __flockfile(stream);
    if (stream->flush)
        ret = stream->flush(stream);
    __funlock_return(stream, ret);
}

int
putc(int c, FILE *stream)
{
    int ret;

    __flockfile(stream);

    if ((stream->flags & __SWR) == 0) {
        stream->flags |= __SERR;
        ret = EOF;
    } else if (stream->put((char) c, stream) < 0) {
        stream->flags |= __SERR;
        ret = EOF;
    } else {
        ret = (unsigned char) c;
    }

    __funlockfile(stream);
    return ret;
}

int
fputc(int c, FILE *stream)
{
    return putc(c, stream);
}

int
putchar(int c)
{
    return fputc(c, stdout);
}

size_t
fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    const unsigned char *p = (const unsigned char *) ptr;
    size_t total = size * nmemb;
    size_t written = 0;

    if (size == 0 || nmemb == 0)
        return 0;

    __flockfile(stream);

    if ((stream->flags & __SWR) == 0) {
        stream->flags |= __SERR;
        __funlock_return(stream, 0);
    }

    while (written < total) {
        if (stream->put((char) p[written], stream) < 0) {
            stream->flags |= __SERR;
            break;
        }
        ++written;
    }

    __funlockfile(stream);
    return written / size;
}

int
fputs(const char *str, FILE *stream)
{
    size_t len = strlen(str);
    return fwrite(str, 1, len, stream) == len ? 0 : EOF;
}

int
puts(const char *str)
{
    if (fputs(str, stdout) == EOF)
        return EOF;
    return putchar('\n');
}
