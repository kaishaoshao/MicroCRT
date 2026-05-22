#include <stddef.h>
#include <unistd.h>

typedef long ssize_t;

ssize_t
_write(int fd, const void *buf, size_t nbyte)
{
    return write(fd, buf, nbyte);
}

ssize_t
_read(int fd, void *buf, size_t nbyte)
{
    return read(fd, buf, nbyte);
}
