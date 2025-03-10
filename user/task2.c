#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
    int pipefd[2];
    if (pipe(pipefd) < 0)
    {
        fprintf(2, "Error creating a pipe\n");
        exit(1);
    }

    int pid = fork();
    switch (pid)
    {
    case -1:
        fprintf(2, "Error creating a child process\n");
        close(pipefd[0]);
        close(pipefd[1]);
        exit(1);
    case 0:
        if (close(pipefd[1]) < 0)
        {
            fprintf(2, "Child: Error closing the write end of the pipe\n");
            close(pipefd[0]);
            exit(1);
        }

        if (close(0) < 0)
        {
            fprintf(2, "Child: Error closing stdin\n");
            close(pipefd[0]);
            exit(1);
        }
        if (dup(pipefd[0]) < 0)
        {
            fprintf(2, "Child: Error replacing stdin with the read end of the pipe\n");
            close(pipefd[0]);
            exit(1);
        }
        if (close(pipefd[0]) < 0)
        {
            fprintf(2, "Child: Error closing the read end of the pipe\n");
            exit(1);
        }

        char *wc_argv[] = {"/wc", 0};
        if (exec(wc_argv[0], wc_argv) < 1)
        {
            fprintf(2, "Child: Error calling the exec\n");
            exit(1);
        }

        fprintf(2, "Child: exec failed\n");
        exit(1);
    default:
        close(pipefd[0]);

        for (int i = 0; i < argc; ++i)
        {
            char *arg = argv[i];
            int arg_length = strlen(arg);
            int bytes_written = 0;

            while (bytes_written < arg_length)
            {
                int bytes_transfered = write(pipefd[1], arg + bytes_written, arg_length - bytes_written);
                if (bytes_transfered <= 0)
                {
                    fprintf(2, "Parent: Write error\n");
                    close(pipefd[1]);
                    exit(1);
                }
                bytes_written += bytes_transfered;
            }
            if (write(pipefd[1], "\n", 1) <= 0)
            {
                fprintf(2, "Parent: Error writing the '\\n'\n");
                close(pipefd[1]);
                exit(1);
            }
        }

        int status;
        if (close(pipefd[1]) < 0)
        {
            fprintf(2, "Parent: Error closing the write end of the pipe\n");
            exit(1);
        }
        if (wait(&status) < 0)
        {
            fprintf(2, "Parent: Error waiting for a child process to exit\n");
            exit(1);
        }
        exit(0);
    }
}