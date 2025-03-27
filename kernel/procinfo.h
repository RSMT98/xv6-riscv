struct procinfo
{
    int pid;
    char name[16];
    enum { PROCINFO_SLEEPING, PROCINFO_RUNNABLE, PROCINFO_RUNNING, PROCINFO_ZOMBIE } state;
    int ppid;
};