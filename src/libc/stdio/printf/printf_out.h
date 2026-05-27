#ifndef _MICROCRT_PRINTF_OUT_H_
#define _MICROCRT_PRINTF_OUT_H_

#include "printf_bridge.h"

struct __printf_out {
    void *cookie;
    int (*put)(int ch, void *cookie);
    int (*write)(const char *buf, size_t len, void *cookie);
    int (*flush)(void *cookie);
    int (*finalize)(void *cookie);
    void (*lock)(void *cookie);
    void (*unlock)(void *cookie);
    int (*writable)(void *cookie);
    void (*mark_error)(void *cookie);
};

#ifndef MICROCRT_PRINTF_ENABLE_AUTO_FLUSH
#define MICROCRT_PRINTF_ENABLE_AUTO_FLUSH 0
#endif

struct __printf_cstr_out {
    char *pos;
    char *end;
    size_t cap;
};

static inline int
__printf_file_put(int ch, void *cookie)
{
    FILE *stream = (FILE *) cookie;
    return stream->put((char) ch, stream);
}

static inline int
__printf_file_write(const char *buf, size_t len, void *cookie)
{
    FILE *stream = (FILE *) cookie;
    size_t written = 0;

    while (written < len) {
        if (stream->put(buf[written], stream) < 0)
            return -1;
        written++;
    }
    return (int) written;
}

static inline int
__printf_file_flush(void *cookie)
{
    FILE *stream = (FILE *) cookie;
    if (stream->flush == NULL)
        return 0;
    return stream->flush(stream);
}

static inline int
__printf_file_finalize(void *cookie)
{
#if MICROCRT_PRINTF_ENABLE_AUTO_FLUSH
    return __printf_file_flush(cookie);
#else
    (void) cookie;
    return 0;
#endif
}

static inline void
__printf_file_lock(void *cookie)
{
    __flockfile((FILE *) cookie);
}

static inline void
__printf_file_unlock(void *cookie)
{
    __funlockfile((FILE *) cookie);
}

static inline int
__printf_file_writable(void *cookie)
{
    FILE *stream = (FILE *) cookie;
    return (stream->flags & __SWR) != 0;
}

static inline void
__printf_file_mark_error(void *cookie)
{
    FILE *stream = (FILE *) cookie;
    stream->flags |= __SERR;
}

static inline void
__printf_out_init_file(struct __printf_out *out, FILE *stream)
{
    out->cookie = stream;
    out->put = __printf_file_put;
    out->write = __printf_file_write;
    out->flush = __printf_file_flush;
    out->finalize = __printf_file_finalize;
    out->lock = __printf_file_lock;
    out->unlock = __printf_file_unlock;
    out->writable = __printf_file_writable;
    out->mark_error = __printf_file_mark_error;
}

static inline int
__printf_cstr_put(int ch, void *cookie)
{
    struct __printf_cstr_out *buf = (struct __printf_cstr_out *) cookie;

    if (buf->end == NULL || buf->pos < buf->end)
        *buf->pos++ = (char) ch;

    return (unsigned char) ch;
}

static inline int
__printf_cstr_write(const char *src, size_t len, void *cookie)
{
    struct __printf_cstr_out *buf = (struct __printf_cstr_out *) cookie;
    size_t writable = len;

    if (buf->end != NULL && buf->pos + writable > buf->end)
        writable = (size_t) (buf->end - buf->pos);

    if (writable != 0) {
        memcpy(buf->pos, src, writable);
        buf->pos += writable;
    }

    return (int) len;
}

static inline int
__printf_cstr_flush(void *cookie)
{
    (void) cookie;
    return 0;
}

static inline int
__printf_cstr_finalize(void *cookie)
{
    struct __printf_cstr_out *buf = (struct __printf_cstr_out *) cookie;

    if (buf->cap != 0 && buf->pos != NULL)
        *buf->pos = '\0';
    return 0;
}

static inline void
__printf_cstr_lock(void *cookie)
{
    (void) cookie;
}

static inline void
__printf_cstr_unlock(void *cookie)
{
    (void) cookie;
}

static inline int
__printf_cstr_writable(void *cookie)
{
    (void) cookie;
    return 1;
}

static inline void
__printf_cstr_mark_error(void *cookie)
{
    (void) cookie;
}

static inline void
__printf_out_init_cstr(struct __printf_out *out, struct __printf_cstr_out *buf)
{
    out->cookie = buf;
    out->put = __printf_cstr_put;
    out->write = __printf_cstr_write;
    out->flush = __printf_cstr_flush;
    out->finalize = __printf_cstr_finalize;
    out->lock = __printf_cstr_lock;
    out->unlock = __printf_cstr_unlock;
    out->writable = __printf_cstr_writable;
    out->mark_error = __printf_cstr_mark_error;
}

static inline int
__printf_out_write(struct __printf_out *out, int *stream_len, const char *buf, size_t len)
{
    if (len == 0)
        return 0;

    *stream_len += (int) len;

    if (out->write != NULL)
        return out->write(buf, len, out->cookie);

    while (len--) {
        if (out->put((unsigned char) *buf++, out->cookie) < 0)
            return -1;
    }
    return 0;
}

static inline int
__printf_out_begin(struct __printf_out *out)
{
    out->lock(out->cookie);

    if (!out->writable(out->cookie)) {
        out->mark_error(out->cookie);
        out->unlock(out->cookie);
        return EOF;
    }
    return 0;
}

static inline void
__printf_out_fail(struct __printf_out *out)
{
    out->mark_error(out->cookie);
}

static inline int
__printf_out_finish(struct __printf_out *out, int stream_len)
{
    if (stream_len >= 0 && out->finalize != NULL) {
        if (out->finalize(out->cookie) < 0) {
            out->mark_error(out->cookie);
            stream_len = -1;
        }
    }
    out->unlock(out->cookie);
    return stream_len;
}

static inline int
__printf_out_finish_failed(struct __printf_out *out)
{
    __printf_out_fail(out);
    return __printf_out_finish(out, -1);
}

#endif
