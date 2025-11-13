#include <stdio.h>
#include <string.h>

char input[20];
char ch;
int p, k;

void E(), A(), B();

void main()
{
    p = 0;
    k = 1;

    printf("请输入要分析的字符串（以#结束）:\n");

    // 读取输入
    do {
        ch = getchar();
        input[p++] = ch;
    } while (ch != '#');

    p = 0;
    ch = input[p++];

    // 开始分析
    E();

    // 检查是否成功分析到结束且没有错误
    if ((ch == '#') && (k == 1))
        printf("success\n");
    else
        printf("error\n");
}

// E → Aa | Bb
void E()
{
    if (ch == 'c' || ch == 'e')  // A 的开始符号
    {
        A();
        if (ch == 'a')
        {
            ch = input[p++];
        }
        else
        {
            k = 0;
        }
    }
    else if (ch == 'b')  // B 的开始符号
    {
        B();
        if (ch == 'b')
        {
            ch = input[p++];
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

// A → cA | eB
void A()
{
    if (ch == 'c')
    {
        ch = input[p++];
        A();
    }
    else if (ch == 'e')
    {
        ch = input[p++];
        B();
    }
    else
    {
        k = 0;
    }
}

// B → bd
void B()
{
    if (ch == 'b')
    {
        ch = input[p++];
        if (ch == 'd')
        {
            ch = input[p++];
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
