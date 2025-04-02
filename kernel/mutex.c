#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "file.h"
#include "proc.h"
#include "mutex.h"

int mutexalloc(struct file **f)
{
    struct file *file = filealloc();
    if (!file)
    {
        printf("mutexalloc: filealloc failed\n");
        return -1;
    }

    struct mutex *m = kalloc();
    if (!m)
    {
        printf("mutexalloc: kalloc failed\n");
        fileclose(file);
        return -1;
    }
    printf("mutexalloc: mutex successfully allocated at %p\n", m);

    initsleeplock(&m->sleeplock, "mutex_sleep");
    file->type = FD_MUTEX;
    file->mutex = m;
    file->readable = file->writable = 0;
    *f = file;
    return 0;
}

int mutexclose(struct file *f)
{
    if (!f)
    {
        printf("mutexclose: invalid file descriptor\n");
        return -1;
    }
    if (f->type != FD_MUTEX)
    {
        printf("mutexclose: invalid file type\n");
        return -1;
    }
    if (!f->mutex)
    {
        printf("mutexclose: mutex already closed\n");
        return -1;
    }
    
    kfree(f->mutex);
    printf("mutexclose: mutex successfully freed at %p\n", f->mutex);
    f->mutex = 0;

    return 0;
}