#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void print_args(int argc, char *argv[], int use_mutex, int mutex_fd)
{
    char output[2];
    output[1] = '\0';
    for (int i = 0; i < argc; i++)
    {
        char *arg = argv[i];
        for (int j = 0; arg[j] != '\0'; ++j)
        {
            if (use_mutex)
            {
                if (mutex_lock(mutex_fd) < 0)
                {
                    fprintf(2, "mutex_lock failed\n");
                    exit(1);
                }
            }
            output[0] = arg[j];
            printf("pid: %d, arg %d, char '%s'\n", getpid(), i, output);
            if (use_mutex)
            {
                if (mutex_unlock(mutex_fd) < 0)
                {
                    fprintf(2, "mutex_unlock failed\n");
                    exit(1);
                }
            }
        }
    }
}

void run_test(int argc, char *argv[], int use_mutex)
{
    int mutex_fd = -1;
    if (use_mutex)
    {
        if ((mutex_fd = mutex()) < 0)
        {
            fprintf(2, "mutex failed\n");
            exit(1);
        }
    }

    int pid1 = fork();
    if (pid1 == 0)
    {
        print_args(argc, argv, use_mutex, mutex_fd);
        exit(0);
    }
    else if (pid1 == -1)
    {
        fprintf(2, "Error creating first child process\n");
        if (use_mutex)
        {
            close(mutex_fd);
        }
        exit(1);
    }

    int pid2 = fork();
    if (pid2 == 0)
    {
        print_args(argc, argv, use_mutex, mutex_fd);
        exit(0);
    }
    else if (pid2 == -1)
    {
        fprintf(2, "Error creating second child process\n");
        kill(pid1);
        if (use_mutex)
        {
            close(mutex_fd);
        }
        exit(1);
    }

    wait(0);
    wait(0);

    if (use_mutex)
    {
        if (close(mutex_fd) < 0)
        {
            fprintf(2, "Error closing mutex\n");
            exit(1);
        }
    }
}

int main(int argc, char *argv[])
{
    printf("Running without mutex...\n");
    run_test(argc, argv, 0);

    printf("\nRunning with mutex...\n");
    run_test(argc, argv, 1);

    exit(0);
}