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
    int copied_procs = 0;
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
        if ((p->state != UNUSED) && (p->state != USED))
            ++total_procs;
        release(&p->lock);
    }

    if (plist == 0)
    {
        return total_procs;
    }

    if (lim < 0)
    {
        return -2;
    }

    if (total_procs > lim)
    {
        return -3;
    }

    for (p = proc; p < &proc[NPROC]; ++p)
    {
        acquire(&p->lock);
        if ((p->state == UNUSED) || (p->state == USED))
        {
            release(&p->lock);
            continue;
        }

        pinfo.pid = p->pid;
        pinfo.state = p->state;
        safestrcpy(pinfo.name, p->name, sizeof(pinfo.name));

        acquire(&wait_lock);
        pinfo.ppid = p->parent ? p->parent->pid : -1;
        release(&wait_lock);

        release(&p->lock);

        if (copyout(cur_proc->pagetable, (uint64)(plist + copied_procs), (char *)&pinfo, sizeof(pinfo)) < 0)
            return -4;

        ++copied_procs;
    }

    return copied_procs;
}