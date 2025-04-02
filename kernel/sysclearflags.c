#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "spinlock.h"
#include "proc.h"

#define LEVELS_COUNT 3
#define PTEs_COUNT 512
#define BIT_PER_INDEX 9
#define PGTABLE_CLEAR_A (1 << 0)
#define PGTABLE_CLEAR_D (1 << 1)

static void walkpgtableandclear(pagetable_t pgtable, uint64 main_addr, uint64 start_addr, uint64 end_addr, int flags, int level)
{
    if (level > (LEVELS_COUNT - 1))
        return;

    for (uint64 i = 0; i < PTEs_COUNT; ++i)
    {
        uint64 cur_addr = main_addr + (i << (PGSHIFT + ((LEVELS_COUNT - 1) - level) * BIT_PER_INDEX));
        uint64 end_cur_addr = main_addr + ((i + 1) << (PGSHIFT + ((LEVELS_COUNT - 1) - level) * BIT_PER_INDEX));
        if (cur_addr >= end_addr || end_cur_addr <= start_addr)
            continue;

        pte_t *ptep = &pgtable[i];
        pte_t pte = *ptep;
        if (pte & PTE_V)
        {
            if (pte & (PTE_R | PTE_W | PTE_X))
            {
                pte_t flags_to_clear = 0;
                if (flags & PGTABLE_CLEAR_A)
                    flags_to_clear |= PTE_A;
                if (flags & PGTABLE_CLEAR_D)
                    flags_to_clear |= PTE_D;

                *ptep &= ~flags_to_clear;
            }
            else
                walkpgtableandclear((pagetable_t)PTE2PA(pte), cur_addr, start_addr, end_addr, flags, level + 1);
        }
    }
}

uint64 sys_clear_flags(void)
{
    uint64 buf_addr;
    int buf_len;
    int flags;
    argaddr(0, &buf_addr);
    argint(1, &buf_len);
    argint(2, &flags);
    if (flags & ~(PGTABLE_CLEAR_A | PGTABLE_CLEAR_D))
        return -1;

    struct proc *cur_proc = myproc();
    if (cur_proc == 0)
        return -1;

    uint64 start_addr;
    uint64 end_addr;
    if (buf_addr != 0 && buf_len > 0)
    {
        if (buf_addr >= cur_proc->sz || buf_addr + buf_len > cur_proc->sz || buf_addr + buf_len < buf_addr)
            return -1;

        start_addr = PGROUNDDOWN(buf_addr);
        end_addr = PGROUNDUP(buf_addr + buf_len);
    }
    else if (buf_addr == 0 && buf_len == 0)
    {
        start_addr = 0;
        end_addr = MAXVA;
    }
    else
        return -1;

    walkpgtableandclear(cur_proc->pagetable, 0, start_addr, end_addr, flags, 0);
    sfence_vma();
    return 0;
}