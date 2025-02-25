#include "kernel/types.h"
#include "user/user.h"

#define BUFFER_SIZE 256

int is_number_valid(const char *str_num)
{
    while (*str_num)
    {
        if ((*str_num < '0') || (*str_num > '9'))
            return 0;
        ++str_num;
    }
    return 1;
}

int main(int argc, char *argv[])
{
    char buf[BUFFER_SIZE];
    char symbol;
    int cur_ind = 0;
    int read_return;

    while (cur_ind < (BUFFER_SIZE - 1))
    {
        if ((read_return = read(0, &symbol, 1)) > 0)
        {
            if (symbol == '\n')
            {
                buf[cur_ind] = '\0';
                break;
            }
            else
                buf[cur_ind++] = symbol;
        }
        else if (read_return == 0)
        {
            buf[cur_ind] = '\0';
            break;
        }
        else
        {
            fprintf(2, "Read error\n");
            exit(1);
        }
    }

    if (cur_ind == (BUFFER_SIZE - 1))
    {
        
        if ((read_return = read(0, &symbol, 1)) < 0)
        {
            fprintf(2, "Read error\n");
            exit(1);
        }
        if ((read_return > 0) && (symbol != '\n')) {
            fprintf(2, "Buffer overflow\n");
            exit(1);
        }
        buf[cur_ind] = '\0';
    }

    printf("|%s|\n", buf);

    if (strlen(buf) == 0)
    {
        fprintf(2, "An empty string was entered\n");
        exit(1);
    }

    char *space = strchr(buf, ' ');
    if (space == 0)
    {
        fprintf(2, "There is no space\n");
        exit(1);
    }

    if (strchr(space + 1, ' '))
    {
        fprintf(2, "There are multiple spaces\n");
        exit(1);
    }

    *space = '\0';
    char *str_num1 = buf;
    char *str_num2 = space + 1;

    if (strlen(str_num1) == 0)
    {
        fprintf(2, "There is no first number\n");
        exit(1);
    }

    if (strlen(str_num2) == 0)
    {
        fprintf(2, "There is no second number\n");
        exit(1);
    }

    if (!is_number_valid(str_num1))
    {
        fprintf(2, "Incorrect format of the first number\n");
        exit(1);
    }

    if (!is_number_valid(str_num2))
    {
        fprintf(2, "Incorrect format of the second number\n");
        exit(1);
    }

    printf("%d\n", add(atoi(str_num1), atoi(str_num2)));
    exit(0);
}
