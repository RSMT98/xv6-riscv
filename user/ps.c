#include "kernel/types.h"
#include "user/user.h"
#include "kernel/procinfo.h"
#include "kernel/param.h"

int main()
{
    int buffer_size = NPROC;
    struct procinfo *plist = 0;
    int procs;

    while (1)
    {
        if (plist)
        {
            free(plist);
        }
        plist = malloc(buffer_size * sizeof(struct procinfo));

        if (!plist)
        {
            fprintf(2, "Bad malloc\n");
            exit(1);
        }

        procs = ps_listinfo(plist, buffer_size);
        if (procs >= 0)
        {
            break;
        }
        else if (procs == -3)
        {
            buffer_size *= 2;
        }
        else
        {
            switch (procs)
            {
            case -1:
                fprintf(2, "Error getting the current process\n");
                break;
            case -2:
                fprintf(2, "Incorrect lim value (it cannot be negative)\n");
                break;
            case -4:
                fprintf(2, "Error copying from kernel to user\n");
                break;
            default:
                fprintf(2, "Some untraceable error happened while receiving processes info\n");
                break;
            }
            free(plist);
            exit(1);
        }
    }

    printf("id\tname\tstate\tppid\tpname\n");
    for (int i = 0; i < procs; ++i)
    {
        char *pname = "";
        if (plist[i].ppid != -1)
        {
            for (int j = 0; j < procs; ++j)
            {
                if (plist[j].pid == plist[i].ppid)
                {
                    pname = plist[j].name;
                    break;
                }
            }
        }

        char *state_name = "";
        switch (plist[i].state)
        {
        case 2:
            state_name = "SLEEPING";
            break;
        case 3:
            state_name = "RUNNABLE";
            break;
        case 4:
            state_name = "RUNNING";
            break;
        case 5:
            state_name = "ZOMBIE";
            break;
        }

        if (state_name[0] == '\0')
        {
            fprintf(2, "Process %d has an unexpected state %d\n", plist[i].pid, plist[i].state);
            free(plist);
            exit(1);
        }

        printf("%d\t%s\t%s\t%d\t%s\n", plist[i].pid, plist[i].name, state_name, plist[i].ppid, pname);
    }

    free(plist);
    exit(0);
}