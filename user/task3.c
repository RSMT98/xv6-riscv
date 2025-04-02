#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void test_read_write()
{
    int fd = mutex();
    if (fd < 0)
    {
        printf("test_read_write: mutex failed\n");
        exit(1);
    }
    char buf[1];
    if (read(fd, buf, 1) != -1)
    {
        printf("test_read_write failed. Read on mutex succeeded\n");
        exit(1);
    }
    if (write(fd, buf, 1) != -1)
    {
        printf("test_read_write failed. Write on mutex succeeded\n");
        exit(1);
    }
    if (close(fd) < 0)
    {
        fprintf(2, "test_read_write: close failed\n");
        exit(1);
    }
    printf("test_read_write: PASSED\n");
}

void test_close_self()
{
    int fd = mutex();
    if (fd < 0)
    {
        printf("test_close_self: mutex failed\n");
        exit(1);
    }
    if (mutex_lock(fd) < 0)
    {
        printf("test_close_self: lock failed\n");
        exit(1);
    }
    if (close(fd) < 0)
    {
        fprintf(2, "test_close_self: close failed\n");
        exit(1);
    }
    printf("test_close_self: PASSED\n");
}

void test_close_other()
{
    int fd = mutex();
    if (fd < 0)
    {
        printf("test_close_other: mutex failed\n");
        exit(1);
    }
    int pid = fork();
    switch (pid)
    {
    case -1:
        fprintf(2, "test_close_other: fork failed\n");
        exit(1);
    case 0:
        if (close(fd) < 0)
        {
            fprintf(2, "test_close_other: child: close failed\n");
            exit(1);
        }
        exit(0);
    default:
        if (mutex_lock(fd) < 0)
        {
            printf("test_close_other: parent: lock failed\n");
            exit(1);
        }
        wait(0);
        if (close(fd) < 0)
        {
            fprintf(2, "test_close_other: parent: close failed\n");
            exit(1);
        }
    }

    printf("test_close_other: PASSED\n");
}

void test_process_exit()
{
    int pid = fork();
    switch (pid)
    {
    case -1:
        fprintf(2, "test_process_exit: fork failed\n");
        exit(1);
    case 0:
        int fd = mutex();
        if (fd < 0)
        {
            printf("test_process_exit: child: mutex failed\n");
            exit(1);
        }
        if (mutex_lock(fd) < 0)
        {
            printf("test_process_exit: child: lock failed\n");
            exit(1);
        }
        exit(0);
    default:
        wait(0);
    }

    printf("test_process_exit: PASSED\n");
}

void test_unlock_other()
{
    int fd = mutex();
    if (fd < 0)
    {
        printf("test_unlock_other: mutex failed\n");
        exit(1);
    }
    if (mutex_lock(fd) < 0)
    {
        printf("test_unlock_other: lock failed\n");
        exit(1);
    }
    int pid = fork();
    switch (pid)
    {
    case -1:
        fprintf(2, "test_unlock_other: fork failed\n");
        exit(1);
    case 0:
        if (mutex_unlock(fd) == 0)
        {
            printf("test_unlock_other failed. Child unlocked parent's mutex\n");
            exit(1);
        }
        exit(0);
    default:
        wait(0);
        if (mutex_unlock(fd) < 0)
        {
            printf("test_unlock_other: parent: unlock failed\n");
            exit(1);
        }
        if (close(fd) < 0)
        {
            fprintf(2, "test_unlock_other: parent: close failed\n");
            exit(1);
        }
    }

    printf("test_unlock_other: PASSED\n");
}

int main()
{
    printf("Starting tests...\n");

    test_read_write();
    test_close_self();
    test_close_other();
    test_process_exit();
    test_unlock_other();

    printf("ALL TESTS PASSED\n");
    exit(0);
}