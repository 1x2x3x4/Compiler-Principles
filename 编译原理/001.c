#include <stdio.h>
#include <string.h>
#include <ctype.h>

// --- 全局变量与关键字表 ---
char prog[80], token[8], ch;
int syn, p, m, n, sum;
char* rwtab[6] = { "begin", "if", "then", "while", "do", "end" };
// --- 函数原型声明 ---
void scaner();
// --- 主函数 ---
int main2() {
    p = 0;
    printf("\n please input string : \n");
    // 读取源程序直到 '#'
    do {
        ch = getchar();
        prog[p++] = ch;
    } while (ch != '#');
    prog[p] = '\0';
    p = 0; // 重置指针，准备扫描
    do {
        scaner(); // 调用词法分析器
        switch (syn) {
        case 11: printf("(%2d, %8d)\n", syn, sum); break; // 输出整数
        case -1: printf("Input error\n"); break;          // 输出错误
        default: printf("(%2d, %8s)\n", syn, token); break; // 输出其他单词
        }
    } while (syn != 0);
    return 0;
}
// --- 词法分析器函数 (Scanner) ---
void scaner() {
    sum = 0; m = 0; // 重置 sum 和 token 索引
    for (int i = 0; i < 8; i++) token[i] = '\0'; // 清空 token
    ch = prog[p++];
    while (ch == ' ') ch = prog[p++]; // 跳过空格
    if (isalpha(ch)) { // --- 识别标识符或关键字 ---
        while (isalpha(ch) || isdigit(ch)) {
            token[m++] = ch;
            ch = prog[p++];
        }
        p--;      // 指针回退
        syn = 10; // 默认为标识符
        for (n = 0; n < 6; n++) { // 查关键字表
            if (strcmp(token, rwtab[n]) == 0) {
                syn = n + 1;
                break;
            }
        }
    }
    else if (isdigit(ch)) { // --- 识别无符号整数 ---
        while (isdigit(ch)) {
            sum = sum * 10 + (ch - '0');
            ch = prog[p++];
        }
        p--;
        syn = 11;
    }
    else { // --- 识别运算符或界符 ---
        switch (ch) {
        case '+': syn = 13; token[0] = ch; break;
        case '-': syn = 14; token[0] = ch; break;
        case '*': syn = 15; token[0] = ch; break;
        case '/': syn = 16; token[0] = ch; break;
        case '=': syn = 25; token[0] = ch; break;
        case ';': syn = 26; token[0] = ch; break;
        case '(': syn = 27; token[0] = ch; break;
        case ')': syn = 28; token[0] = ch; break;
        case '#': syn = 0;  token[0] = ch; break;
        case ':': // 可能为 ':' 或 ':='
            token[0] = ch;
            ch = prog[p++];
            if (ch == '=') { syn = 18; token[1] = ch; }
            else { syn = 17; p--; }
            break;
        case '<': // 可能为 '<', '<>', '<='
            token[0] = ch;
            ch = prog[p++];
            if (ch == '>') { syn = 21; token[1] = ch; }
            else if (ch == '=') { syn = 22; token[1] = ch; }
            else { syn = 20; p--; }
            break;
        case '>': // 可能为 '>' 或 '>='
            token[0] = ch;
            ch = prog[p++];
            if (ch == '=') { syn = 24; token[1] = ch; }
            else { syn = 23; p--; }
            break;
        default: syn = -1; break; // 非法字符
        }
    }
}