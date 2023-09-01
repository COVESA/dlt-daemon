#include <unistd.h>
#include <stdio.h>
#include <dlfcn.h>

typedef int (*orig_fsync_t)(int);

extern "C" int fsync(int fd)
{
    static orig_fsync_t orig_fsync = (orig_fsync_t)dlsym(RTLD_NEXT, "fsync");

    // Could also track fd-name mappings by wrapping open/close
    // Maybe also print current filesize to know exactly when the sync occured
    fprintf(stderr, "%s: fd=%d\n", __func__, fd);

    return orig_fsync(fd);
}