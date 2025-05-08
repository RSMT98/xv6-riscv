#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

static struct spinlock rtc_lock;

void
rtcinit(void)
{
    initlock(&rtc_lock, "rtc");
}

static inline uint32 read_rtc_reg(uint64 addr)
{
    return *(volatile uint32 *)addr;
}

uint64
sys_get_time(void)
{
    acquire(&rtc_lock);
    uint32 low = read_rtc_reg(RTC_LOW);
    uint32 high = read_rtc_reg(RTC_HIGH);
    release(&rtc_lock);
    return ((uint64)high << 32) | low;
}
