#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/procinfo.h"

void test_null_plist()
{
    int total_procs = ps_listinfo(0, 0);
    if (total_procs < 0)
    {
        printf("test_null_plist: FAILED (error %d)\n", total_procs);
        exit(0);
    }
    printf("test_null_plist: PASSED\n");
}

void test_good_buffer()
{
    int total_procs = ps_listinfo(0, 0);
    if (total_procs < 0)
    {
        printf("test_good_buffer: FAILED (error %d)\n", total_procs);
        exit(0);
    }
    if (total_procs == 0)
    {
        printf("test_good_buffer: SKIPPED (no processes)\n");
        return;
    }
    struct procinfo *plist = malloc(total_procs * sizeof(struct procinfo));
    if (!plist)
    {
        printf("test_good_buffer: FAILED (bad malloc)\n");
        exit(0);
    }
    int procs = ps_listinfo(plist, total_procs);
    if (procs != total_procs)
    {
        printf("test_good_buffer: FAILED (expected procs %d, got %d)\n", total_procs, procs);
        free(plist);
        exit(0);
    }
    free(plist);
    printf("test_good_buffer: PASSED\n");
}

void test_bad_buffer()
{
    int total_procs = ps_listinfo(0, 0);
    if (total_procs < 0)
    {
        printf("test_bad_buffer: FAILED (error %d)\n", total_procs);
        exit(0);
    }
    if (total_procs < 2)
    {
        printf("test_bad_buffer: SKIPPED (not enough processes)\n");
        return;
    }
    int error_val = ps_listinfo((struct procinfo *)0xABCDEFFF, total_procs - 1);
    if (error_val != -3)
    {
        printf("test_bad_buffer: FAILED (expected error_val -3, got %d)\n", error_val);
        exit(0);
    }
    printf("test_bad_buffer: PASSED\n");
}

void test_invalid_address()
{
    int error_val = ps_listinfo((struct procinfo *)0xABCDEFFF, 100);
    if (error_val != -4)
    {
        printf("test_invalid_address: FAILED (expected error_val -4, got %d)\n", error_val);
        exit(0);
    }
    printf("test_invalid_address: PASSED\n");
}

void test_negative_lim()
{
    int error_val = ps_listinfo((struct procinfo *)0xABCDEFFF, -1);
    if (error_val != -2)
    {
        printf("test_negative_lim: FAILED (expected error_val -2, got %d)\n", error_val);
        exit(0);
    }
    printf("test_negative_lim: PASSED\n");
}

void test_fork_ppid()
{
    int pid = fork();
    switch (pid)
    {
    case -1:
        printf("test_fork_ppid: FAILED (fork)\n");
        exit(0);
    case 0:
        sleep(50);
        exit(0);
    default:
        sleep(20); // time for the child process to start
        int total_procs = ps_listinfo(0, 0);
        struct procinfo *plist = malloc(total_procs * sizeof(struct procinfo));
        if (!plist)
        {
            printf("test_fork_ppid: FAILED (bad malloc)\n");
            exit(0);
        }
        int procs = ps_listinfo(plist, total_procs);
        int has_child_been_found = 0;
        for (int i = 0; i < procs; ++i)
        {
            if (plist[i].pid == pid)
            {
                has_child_been_found = 1;
                if (plist[i].ppid != getpid())
                {
                    printf("test_fork_ppid: FAILED (expected ppid %d, got %d)\n", getpid(), plist[i].ppid);
                    free(plist);
                    exit(0);
                }
                break;
            }
        }
        free(plist);
        if (!has_child_been_found)
        {
            printf("test_fork_ppid: FAILED (child wasn't found)\n");
            exit(0);
        }
        wait(0);
        printf("test_fork_ppid: PASSED\n");
    }
}

int main(int argc, char *argv[])
{
    printf("Starting tests...\n");

    test_null_plist();
    test_good_buffer();
    test_bad_buffer();
    test_invalid_address();
    test_negative_lim();
    test_fork_ppid();

    printf("ALL TESTS PASSED\n");
    exit(0);
}