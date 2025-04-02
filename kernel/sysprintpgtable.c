#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "spinlock.h"
#include "proc.h"

#define LEVELS_COUNT 3
#define PTEs_COUNT 512
#define BIT_PER_INDEX 9
#define PGTABLE_PRINT_A (1 << 0)
#define PGTABLE_PRINT_D (1 << 1)

static void print_pte_flags(pte_t pte)
{
    consputc((pte & PTE_R) ? 'R' : '_');
    consputc((pte & PTE_W) ? 'W' : '_');
    consputc((pte & PTE_X) ? 'X' : '_');
    consputc((pte & PTE_U) ? 'U' : '_');
    consputc((pte & PTE_G) ? 'G' : '_');
    consputc((pte & PTE_A) ? 'A' : '_');
    consputc((pte & PTE_D) ? 'D' : '_');
    printf("\n");
}

static void print_pte(pte_t pte, int i, int level)
{
    for (int j = 0; j < level; j++)
        printf(".........");
    printf("0x%x -> %p ", i, (void *)PTE2PA(pte));
    print_pte_flags(pte);
}

static void walkpgtableandprint(pagetable_t pgtable, uint64 main_addr, uint64 start_addr, uint64 end_addr, int flags, int level)
{
    if (level > (LEVELS_COUNT - 1))
        return;

    for (uint64 i = 0; i < PTEs_COUNT; ++i)
    {
        uint64 cur_addr = main_addr + (i << (PGSHIFT + ((LEVELS_COUNT - 1) - level) * BIT_PER_INDEX));
        uint64 end_cur_addr = main_addr + ((i + 1) << (PGSHIFT + ((LEVELS_COUNT - 1) - level) * BIT_PER_INDEX));
        if (cur_addr >= end_addr || end_cur_addr <= start_addr)
            continue;

        pte_t pte = pgtable[i];
        if (pte & PTE_V)
        {
            int is_final_table = (pte & (PTE_R | PTE_W | PTE_X));
            if (is_final_table && ((flags == 0) || ((flags & PGTABLE_PRINT_A) && (pte & PTE_A)) || ((flags & PGTABLE_PRINT_D) && (pte & PTE_D))))
                print_pte(pte, i, level);
            else if (!is_final_table)
            {
                print_pte(pte, i, level);
                walkpgtableandprint((pagetable_t)PTE2PA(pte), cur_addr, start_addr, end_addr, flags, level + 1);
            }
        }
    }
}

uint64 sys_print_pgtable(void)
{
    uint64 buf_addr;
    int buf_len;
    int flags;
    argaddr(0, &buf_addr);
    argint(1, &buf_len);
    argint(2, &flags);
    if (flags != 0 && (flags & ~(PGTABLE_PRINT_A | PGTABLE_PRINT_D)))
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

    walkpgtableandprint(cur_proc->pagetable, 0, start_addr, end_addr, flags, 0);
    return 0;
}