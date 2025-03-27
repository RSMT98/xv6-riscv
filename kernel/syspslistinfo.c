#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "spinlock.h"
#include "param.h"
#include "proc.h"
#include "procinfo.h"

extern struct proc proc[NPROC];
extern struct spinlock wait_lock;

uint64 sys_ps_listinfo(void)
{
    struct procinfo *plist;
    uint64 plist_addr;
    int lim;
    struct proc *p;
    struct procinfo pinfo;
    int total_procs = 0;
    struct proc *cur_proc = myproc();
    if (cur_proc == 0)
    {
        return -1;
    }

    argaddr(0, &plist_addr);
    plist = (struct procinfo *)plist_addr;
    argint(1, &lim);

    for (p = proc; p < &proc[NPROC]; ++p)
    {
        acquire(&p->lock);
        if ((p->state == UNUSED) || (p->state == USED))
        {
            release(&p->lock);
            continue;
        }

        ++total_procs;
        if (plist == 0)
        {
            release(&p->lock);
            continue;
        }

        if (lim < 0)
        {
            release(&p->lock);
            return -2;
        }

        if (total_procs > lim)
        {
            release(&p->lock);
            return -3;
        }

        pinfo.pid = p->pid;
        switch (p->state)
        {
        case SLEEPING:
            pinfo.state = PROCINFO_SLEEPING;
            break;
        case RUNNABLE:
            pinfo.state = PROCINFO_RUNNABLE;
            break;
        case RUNNING:
            pinfo.state = PROCINFO_RUNNING;
            break;
        default:
            pinfo.state = PROCINFO_ZOMBIE;
            break;
        }
        safestrcpy(pinfo.name, p->name, sizeof(pinfo.name));

        pinfo.ppid = -1;
        acquire(&wait_lock);
        if (p->parent)
        {
            acquire(&p->parent->lock);
            pinfo.ppid = p->parent->pid;
            release(&p->parent->lock);
        }
        release(&wait_lock);

        release(&p->lock);

        if (copyout(cur_proc->pagetable, (uint64)(plist + (total_procs - 1)), (char *)&pinfo, sizeof(pinfo)) < 0)
            return -4;
    }

    return total_procs;
}