#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <dlfcn.h>
#include <setjmp.h>

#define LFI_SYS_pause 1024

#pragma GCC visibility push(default)

extern void _lfi_ret(void);

__attribute__((noreturn)) static void *
lfi_pause(void *arg)
{
    (void) arg;
    syscall(LFI_SYS_pause);
    while (1) {
    }
}

void *
_lfi_thread_create(void)
{
    pthread_t *t = malloc(sizeof(pthread_t));
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_attr_setstacksize(&attr, 2 * 1024 * 1024);
    pthread_create(t, &attr, &lfi_pause, NULL);
    pthread_attr_destroy(&attr);
    return t;
}

void
_lfi_thread_destroy(void *arg)
{
    (void) arg; // arg is pthread_t *
    pthread_exit(NULL);
}

int
_lfi_errno(void)
{
    return errno;
}

void *
_lfi_dlopen(const char *filename, int flags)
{
    return dlopen(filename, flags);
}

void *
_lfi_dlsym(void *handle, const char *symbol)
{
    return dlsym(handle, symbol);
}

void *
_lfi_malloc(size_t size)
{
    return malloc(size);
}

void *
_lfi_realloc(void *ptr, size_t size)
{
    return realloc(ptr, size);
}

void *
_lfi_calloc(size_t nmemb, size_t size)
{
    return calloc(nmemb, size);
}

void
_lfi_free(void *ptr)
{
    free(ptr);
}

void
_lfi_setjmp(jmp_buf env, void *host_env, void (*callback)(void *, int))
{
    int r = setjmp(env);
    if (r != 0) {
        callback(host_env, r);
        __builtin_unreachable();
    }
}

void *symbols[] = {
    &_lfi_thread_create,
    &_lfi_thread_destroy,
    &_lfi_errno,
    &_lfi_dlopen,
    &_lfi_dlsym,
    &_lfi_malloc,
    &_lfi_realloc,
    &_lfi_calloc,
    &_lfi_free,
    &_lfi_ret,
    &_lfi_setjmp,
};

int
main(void)
{
    lfi_pause(NULL);
}

#pragma GCC visibility pop
