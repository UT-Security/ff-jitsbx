#define _GNU_SOURCE
#include <dirent.h>
#include <stdio.h>
#include <syscall.h>
#include <sys/stat.h>
#include <unistd.h>

#include "types.h"
#include "host/posix/file.h"
#include "host/error.h"

static inline ssize_t getdents64(int fd, void* dirp, size_t count) {
    return syscall(__NR_getdents64, fd, dirp, count);
}

ssize_t
host_getdents64(struct HostFile* file, void* dirp, size_t count)
{
    ssize_t r = getdents64(file->fd, dirp, count);
    if (r < 0)
        return tuxerr(errno);
    return r;
}
