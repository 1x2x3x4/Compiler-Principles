#include <stdio.h>
#include <string.h>

char prog[20], token[8];
char ch;
int p, k;

void e(), f(), t();

void main3()
{
    p = 0;
    printf("\n please input the source program:\n");
    do
    {
        ch = getchar();
        prog[p++] = ch;
    } while (ch != '#');

    p = 0;
    k = 1;
    ch = prog[p++];

    if ((ch == 'a') || (ch == 'b') || (ch == 'c') || (ch == '('))
        e();
    else
        k = 0;

    if ((ch == '#') && (k == 1))
        printf("success\n");
    else
        printf("error\n");
}

void e()
{
    t();
    while ((ch == '+') || (ch == '-'))
    {
        ch = prog[p++];
        t();
    }
}

void t()
{
    f();
    while ((ch == '*') || (ch == '/'))
    {
        ch = prog[p++];
        f();
    }
}

void f()
{
    if ((ch == 'a') || (ch == 'b') || (ch == 'c'))
    {
        ch = prog[p++];
    }
    else if (ch == '(')
    {
        ch = prog[p++];
        e();
        if (ch == ')')
        {
            ch = prog[p++];
        }
        else
        {
            k = 0;
        }
    }
    else
    {
        k = 0;
    }
}
