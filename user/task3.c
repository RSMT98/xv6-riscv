#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/riscv.h"

#define LEVELS_COUNT 3
#define PGTABLE_PRINT_A (1 << 0)
#define PGTABLE_PRINT_D (1 << 1)
#define PGTABLE_CLEAR_A (1 << 0)
#define PGTABLE_CLEAR_D (1 << 1)

int global_var;
char global_array[15];

int main(int argc, char *argv[])
{
    volatile int stack_var = 20;
    volatile char stack_array[30] = {1};

    char *heap_small;
    int *heap_large;

    volatile char temp_char;
    volatile int temp_int;

    printf("\n=== Starting Tests ===\n");

    printf("\n--- 1. Starting Page Table ---\n");
    if (print_pgtable(0, 0, 0) < 0)
    {
        fprintf(2, "print_pgtable failed\n");
        exit(1);
    }

    int heap_small_size = 500;
    heap_small = malloc(heap_small_size);
    if (!heap_small)
    {
        fprintf(2, "heap_small: bad malloc\n");
        exit(1);
    }
    int heap_large_size = PGSIZE * LEVELS_COUNT + 128;
    heap_large = malloc(heap_large_size);
    if (!heap_large)
    {
        fprintf(2, "heap_large: bad malloc\n");
        free(heap_small);
        exit(1);
    }

    printf("\n--- 2. Page Table After Malloc ---\n");
    if (print_pgtable(0, 0, 0) < 0)
    {
        fprintf(2, "print_pgtable failed\n");
        free(heap_small);
        free(heap_large);
        exit(1);
    }

    if (clear_flags(0, 0, PGTABLE_CLEAR_A | PGTABLE_CLEAR_D) < 0)
    {
        fprintf(2, "clear_flags failed\n");
        free(heap_small);
        free(heap_large);
        exit(1);
    }

    printf("\n--- 3. Page Table After Clearing A and D From All Pages ---\n");
    if (print_pgtable(0, 0, PGTABLE_PRINT_A | PGTABLE_PRINT_D) < 0)
    {
        fprintf(2, "print_pgtable failed\n");
        free(heap_small);
        free(heap_large);
        exit(1);
    }
    
    temp_int = global_var; temp_int;
    temp_int = stack_var;
    temp_char = global_array[0]; temp_char;
    temp_char = stack_array[5];
    temp_char = heap_small[10];
    temp_int = heap_large[0];
    temp_int = heap_large[PGSIZE / sizeof(int)];
    temp_int = heap_large[2 * PGSIZE / sizeof(int)];
    temp_int = heap_large[3 * PGSIZE / sizeof(int)];

    printf("\n--- 4. Page Table After Reading ---\n");
    if (print_pgtable(0, 0, PGTABLE_PRINT_A) < 0)
    {
        fprintf(2, "print_pgtable failed\n");
        free(heap_small);
        free(heap_large);
        exit(1);
    }

    if (clear_flags(0, 0, PGTABLE_CLEAR_A | PGTABLE_CLEAR_D) < 0)
    {
        fprintf(2, "clear_flags failed\n");
        free(heap_small);
        free(heap_large);
        exit(1);
    }

    heap_small[0] = 'a';
    heap_large[1] = 1;
    heap_large[PGSIZE / sizeof(int) + 1] = 2;
    heap_large[2 * PGSIZE / sizeof(int) + 1] = 3;
    heap_large[3 * PGSIZE / sizeof(int) + 1] = 4;
    global_var = 10;
    ++stack_var;
    global_array[3] = 'b';
    stack_array[6] = 'c';

    printf("\n--- 5. Page Table After Writing ---\n");
    if (print_pgtable(0, 0, PGTABLE_PRINT_D) < 0)
    {
        fprintf(2, "print_pgtable failed\n");
        free(heap_small);
        free(heap_large);
        exit(1);
    }

    if (clear_flags(heap_small, heap_small_size, PGTABLE_CLEAR_A | PGTABLE_CLEAR_D) < 0)
    {
        fprintf(2, "clear_flags failed\n");
        free(heap_small);
        free(heap_large);
        exit(1);
    }

    printf("\n--- 6. Page Table (only 'heap_small' pages) After Clearing A and D From 'heap_small' Pages ---\n");
    if (print_pgtable(heap_small, heap_small_size, 0) < 0)
    {
        fprintf(2, "print_pgtable failed\n");
        free(heap_small);
        free(heap_large);
        exit(1);
    }

    free(heap_small);
    free(heap_large);

    printf("\n--- 7. Page Table After Freeing Memory ---\n");
    if (print_pgtable(0, 0, 0) < 0)
    {
        fprintf(2, "print_pgtable failed\n");
        exit(1);
    }

    printf("\n=== ALL TESTS PASSED ===\n");
    exit(0);
}