#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "proc.h"
#include "ps.h"
#include "file.h"

static uint64 urandom_seed = 1;
static struct spinlock urandom_lock;
static uint64 nullstat_count = 0;
static struct spinlock nullstat_lock;

int ps_read(int minor, int user_dst, uint64 dst, int n)
{
    switch (minor)
    {
    case NULL:
        return 0;
    case ZERO:
        char *zero_buf = kalloc();
        if (!zero_buf)
            return -1;
        memset(zero_buf, 0, PGSIZE);
        int remaining = n;
        while (remaining > 0)
        {
            int chunk = remaining > PGSIZE ? PGSIZE : remaining;
            if (copyout(myproc()->pagetable, dst + n - remaining, zero_buf, chunk) < 0)
            {
                kfree(zero_buf);
                return -1;
            }
            remaining -= chunk;
        }
        kfree(zero_buf);
        return n;
    case URANDOM:
        acquire(&urandom_lock);
        for (int i = 0; i < n; ++i)
        {
            urandom_seed = urandom_seed * 1664525 + 1013904223;
            char byte = (urandom_seed >> 24) & 0xFF;
            if (copyout(myproc()->pagetable, dst + i, &byte, 1) < 0)
            {
                release(&urandom_lock);
                return -1;
            }
        }
        release(&urandom_lock);
        return n;
    case NULLSTAT:
        if (n != sizeof(uint64))
            return -1;
        acquire(&nullstat_lock);
        uint64 val = nullstat_count;
        release(&nullstat_lock);
        if (copyout(myproc()->pagetable, dst, (char *)&val, sizeof(val)) < 0)
            return -1;
        return sizeof(val);
    default:
        return -1;
    }
}

int ps_write(int minor, int user_src, uint64 src, int n)
{
    switch (minor)
    {
    case NULL:
        return n;
    case ZERO:
        return -1;
    case URANDOM:
        if (n != sizeof(uint64))
            return -1;
        uint64 input_seed;
        if (copyin(myproc()->pagetable, (char *)&input_seed, src, sizeof(input_seed)) < 0)
            return -1;
        acquire(&urandom_lock);
        urandom_seed = input_seed;
        release(&urandom_lock);
        return n;
    case NULLSTAT:
        acquire(&nullstat_lock);
        nullstat_count += n;
        release(&nullstat_lock);
        return n;
    default:
        return -1;
    }
}

void ps_init(void)
{
    initlock(&urandom_lock, "urand");
    initlock(&nullstat_lock, "nstat");
    devsw[PS_MAJOR].read = ps_read;
    devsw[PS_MAJOR].write = ps_write;
}
