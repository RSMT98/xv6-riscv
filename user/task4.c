#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include "kernel/riscv.h"

int hexdump(const char *path, int len)
{
    int fd = open(path, O_RDONLY);
    if (fd < 0)
    {
        fprintf(2, "hexdump: open %s failed\n", path);
        return -1;
    }
    uint8 *buf = malloc(len);
    if (buf == 0)
    {
        fprintf(2, "hexdump: malloc failed\n");
        close(fd);
        return -1;
    }
    int total_read = 0;
    while (total_read < len)
    {
        int n = read(fd, buf + total_read, len - total_read);
        if (n < 0)
        {
            fprintf(2, "hexdump: read failed\n");
            free(buf);
            close(fd);
            return -1;
        }
        if (n == 0)
        {
            fprintf(2, "hexdump: EOF reached before reading all bytes\n");
            free(buf);
            close(fd);
            return -1;
        }
        total_read += n;
    }
    for (int i = 0; i < len; ++i)
        printf("%x ", buf[i]);
    printf("\n");
    free(buf);
    close(fd);
    return 0;
}

int main(int argc, char *argv[])
{
    // possible usage: task4 read zero 32
    if (argc == 4 && strcmp(argv[1], "read") == 0)
        return hexdump(argv[2], atoi(argv[3]));

    printf("=== Testing pseudo-devices ===\n");

    printf("null write %d bytes -> ", 5);
    int fd = open("null", O_WRONLY);
    if (fd < 0)
    {
        fprintf(2, "main: open null failed\n");
        exit(1);
    }
    int ret;
    printf("wrote %d\n", ret = write(fd, "Hello", 5));
    if (ret != 5)
    {
        fprintf(2, "Error: it should\'ve been \"wrote 5\"\n");
        close(fd);
        exit(1);
    }
    close(fd);

    printf("zero read 16 bytes -> ");
    hexdump("zero", 16);

    printf("urandom read 8 bytes -> ");
    hexdump("urandom", sizeof(uint64));

    uint64 seed = 12345678;
    fd = open("urandom", O_WRONLY);
    if (fd < 0)
    {
        fprintf(2, "main: open urandom failed\n");
        exit(1);
    }
    write(fd, &seed, sizeof(seed));
    close(fd);
    printf("urandom seeded read 8 bytes -> ");
    hexdump("urandom", sizeof(seed));

    printf("nullstat write 20 bytes -> ");
    fd = open("nullstat", O_WRONLY);
    if (fd < 0)
    {
        fprintf(2, "main: open nullstat failed\n");
        exit(1);
    }
    printf("wrote %d\n", ret = write(fd, "abcdefghijklmnopqrst", 20));
    if (ret != 20)
    {
        fprintf(2, "Error: it should\'ve been \"wrote 20\"\n");
        close(fd);
        exit(1);
    }
    close(fd);
    printf("nullstat read count (if you ran \'task4\' once, should be \"14 0 0 0 0 0 0 0\" (because 0x14 = 20)) -> ");
    hexdump("nullstat", sizeof(uint64));

    fd = open("zero", O_WRONLY);
    if (fd < 0)
    {
        fprintf(2, "main: open zero failed\n");
        exit(1);
    }
    ret = write(fd, "abcd", 4);
    if (ret != -1)
    {
        fprintf(2, "Error: write to zero should fail, returned %d\n", ret);
        close(fd);
        exit(1);
    }
    printf("Write to zero correctly failed\n");
    close(fd);

    fd = open("nullstat", O_RDONLY);
    if (fd < 0)
    {
        fprintf(2, "main: open nullstat failed\n");
        exit(1);
    }
    uint8 buf[4];
    ret = read(fd, buf, 4);
    if (ret != -1)
    {
        fprintf(2, "Error: read nullstat with 4 bytes should fail, returned %d\n", ret);
        close(fd);
        exit(1);
    }
    printf("Read nullstat with wrong size correctly failed\n");
    close(fd);

    fd = open("urandom", O_WRONLY);
    if (fd < 0)
    {
        fprintf(2, "main: open urandom failed\n");
        exit(1);
    }
    uint32 bad_seed = 1234;
    ret = write(fd, &bad_seed, sizeof(bad_seed));
    if (ret != -1)
    {
        fprintf(2, "Error: write to urandom with 4 bytes should fail, returned %d\n", ret);
        close(fd);
        exit(1);
    }
    printf("Write to urandom with wrong size correctly failed\n");
    close(fd);

    fd = open("null", O_RDONLY);
    if (fd < 0)
    {
        fprintf(2, "main: open null failed\n");
        exit(1);
    }
    uint8 another_buf[10];
    ret = read(fd, another_buf, 10);
    if (ret != 0)
    {
        fprintf(2, "Error: read from null should return 0, got %d\n", ret);
        close(fd);
        exit(1);
    }
    printf("Read from null correctly returned 0 bytes\n");
    close(fd);

    printf("=== ALL TESTS PASSED ===\n");
    exit(0);
}
