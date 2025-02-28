#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#define BUFFER_SIZE 5120

int main(int argc, char *argv[])
{
    int pipefd[2];
    if (pipe(pipefd) < 0)
    {
        perror("Error creating a pipe");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    switch (pid)
    {
    case -1:
        perror("Error creating a child process");
        close(pipefd[0]);
        close(pipefd[1]);
        exit(EXIT_FAILURE);
    case 0:
        close(pipefd[1]);
        
        char buffer[BUFFER_SIZE];
        ssize_t bytes_read = 0;

        while ((bytes_read = read(pipefd[0], buffer, sizeof(buffer))) > 0)
        {
            ssize_t bytes_written = 0;

            while (bytes_written < bytes_read)
            {
                ssize_t bytes_written_out = write(STDOUT_FILENO, buffer + bytes_written, bytes_read - bytes_written);
                if (bytes_written_out <= 0)
                {
                    perror("Child: Write error");
                    close(pipefd[0]);
                    exit(EXIT_FAILURE);
                }
                bytes_written += bytes_written_out;
            }
        }

        if (bytes_read == -1)
        {
            perror("Child: Read error");
            close(pipefd[0]);
            exit(EXIT_FAILURE);
        }

        if (close(pipefd[0]) < 0)
        {
            perror("Child: Error closing the read end of the pipe");
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);
    default:
        close(pipefd[0]);

        for (int i = 0; i < argc; ++i)
        {
            const char *arg = argv[i];
            size_t arg_length = strlen(arg);
            size_t bytes_written = 0;

            while (bytes_written < arg_length)
            {
                size_t bytes_transfered = write(pipefd[1], arg + bytes_written, arg_length - bytes_written);
                if (bytes_transfered <= 0)
                {
                    perror("Parent: Write error");
                    close(pipefd[1]);
                    exit(EXIT_FAILURE);
                }
                bytes_written += bytes_transfered;
            }
            if (write(pipefd[1], "\n", 1) <= 0)
            {
                perror("Parent: Error writing the '\\n'");
                close(pipefd[1]);
                exit(EXIT_FAILURE);
            }
        }

        int status;
        if (close(pipefd[1]) < 0)
        {
            perror("Parent: Error closing the write end of the pipe");
            exit(EXIT_FAILURE);
        }
        if (waitpid(pid, &status, 0) < 0)
        {
            perror("Parent: Error waiting for a child process to exit");
            exit(EXIT_FAILURE);
        }

        if (WIFEXITED(status))
            exit(WEXITSTATUS(status));
        else
        {
            perror("Some untraceable error occurred in a child process");
            exit(EXIT_FAILURE);
        }
    }
}