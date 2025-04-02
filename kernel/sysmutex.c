#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "stat.h"
#include "spinlock.h"
#include "proc.h"
#include "fs.h"
#include "sleeplock.h"
#include "file.h"
#include "fcntl.h"
#include "mutex.h"

uint64 sys_mutex(void)
{
    struct file *f;
    int fd;

    if (mutexalloc(&f) < 0)
        return -1;

    if ((fd = fdalloc(f)) < 0)
    {
        fileclose(f);
        return -1;
    }

    return fd;
}

uint64 sys_mutex_lock(void)
{
    int fd;
    struct file *f;

    if ((argfd(0, &fd, &f) < 0) || f->type != FD_MUTEX || f->mutex == 0)
        return -1;

    struct mutex *m = f->mutex;
    acquiresleep(&m->sleeplock);
    
    return 0;
}

uint64 sys_mutex_unlock(void) {
    int fd;
    struct file* f;

    if(argfd(0, &fd, &f) < 0 || f->type != FD_MUTEX || f->mutex == 0)
        return -1;
    
    struct mutex* m = f->mutex;
    acquire(&m->sleeplock.lk);
    if(m->sleeplock.pid != myproc()->pid) {
        release(&m->sleeplock.lk);
        return -1;
    }
    release(&m->sleeplock.lk);
    releasesleep(&m->sleeplock);

    return 0;
}