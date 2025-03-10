#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define SLEEP_TIME 100

int main(int argc, char *argv[])
{
    int send_kill = 0;
    if ((argc > 1) && ((strcmp(argv[1], "-k") == 0) || (strcmp(argv[1], "--kill") == 0)))
        send_kill = 1;

    int pid = fork();
    switch (pid)
    {
    case -1:
        fprintf(2, "Error creating a child process\n");
        exit(1);
    case 0:
        sleep(SLEEP_TIME);
        exit(1);
    default:
        printf("Parent PID: %d, Child PID: %d\n", getpid(), pid);

        if (send_kill)
        {
            if (kill(pid) < 0)
            {
                fprintf(2, "Error killing a child process\n");
                exit(1);
            }
            printf("The kill signal was sent.\n");
        }

        int status;
        if ((pid = wait(&status)) < 0)
        {
            fprintf(2, "Error waiting for a child process to exit\n");
            exit(1);
        }

        printf("Child PID: %d, Return code: %d\n", pid, status);
        exit(0);
    }
}