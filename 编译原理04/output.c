编译结果文件: source.c -> output.c

源程序:
{
    int a;
    int b;
    int c;
    input(a);
    input(b);
    c = a + b * 2;
    output(c);
}

----------------------------------------

=== 词法分析结果 ===
行号         标记类型        值
----------------------------------------
1            TOKEN_LBRACE    {
2            TOKEN_INT       int
2            TOKEN_IDENTIFIER a
2            TOKEN_SEMICOLON ;
3            TOKEN_INT       int
3            TOKEN_IDENTIFIER b
3            TOKEN_SEMICOLON ;
4            TOKEN_INT       int
4            TOKEN_IDENTIFIER c
4            TOKEN_SEMICOLON ;
5            TOKEN_INPUT     input
5            TOKEN_LPAREN    (
5            TOKEN_IDENTIFIER a
5            TOKEN_RPAREN    )
5            TOKEN_SEMICOLON ;
6            TOKEN_INPUT     input
6            TOKEN_LPAREN    (
6            TOKEN_IDENTIFIER b
6            TOKEN_RPAREN    )
6            TOKEN_SEMICOLON ;
7            TOKEN_IDENTIFIER c
7            TOKEN_ASSIGN    =
7            TOKEN_IDENTIFIER a
7            TOKEN_PLUS      +
7            TOKEN_IDENTIFIER b
7            TOKEN_MULTIPLY  *
7            TOKEN_NUMBER    2
7            TOKEN_SEMICOLON ;
8            TOKEN_OUTPUT    output
8            TOKEN_LPAREN    (
8            TOKEN_IDENTIFIER c
8            TOKEN_RPAREN    )
8            TOKEN_SEMICOLON ;
9            TOKEN_RBRACE    }
10           TOKEN_EOF       EOF

=== 语法分析结果 ===
PROGRAM
  DECLARATION: a
    DECLARATION: b
      DECLARATION: c
        INPUT: a
          INPUT: b
            ASSIGNMENT: c
              BINARY_OP: +
                VARIABLE: a
                BINARY_OP: *
                  VARIABLE: b
                  CONSTANT: 2
              OUTPUT: c

=== 中间代码生成结果 (三地址码) ===
序号   操作     目标       源1        源2       
-------------------------------------------------
0      input    a                               
1      input    b                               
2      = const  t0         2                    
3      *        t1         b          t1        
4      +        t2         a          t2        
5      =        c          t2                   
6      output              c                    

=== 中间代码文本表示 ===
0   : input a
1   : input b
2   : t0 = 2
3   : t1 = b * t1
4   : t2 = a + t2
5   : c = t2
6   : output c

=== 最终汇编代码 ===
.section .rodata
.LC0:
    .string "%d"
.LC1:
    .string "%d\\n"

.section .text
.globl main
main:
    pushq   %rbp
    movq    %rsp, %rbp
    subq    $40, %rsp  # 为局部变量分配空间

    # input(a)
    leaq    -4(%rbp), %rsi
    movl    $.LC0, %edi
    movl    $0, %eax
    call    scanf

    # input(b)
    leaq    -8(%rbp), %rsi
    movl    $.LC0, %edi
    movl    $0, %eax
    call    scanf

    # t0 = 2
    movl    $2, %eax
    movl    %eax, -16(%rbp)

    movl    -8(%rbp), %eax
    imull   -20(%rbp), %eax
    movl    %eax, -20(%rbp)  # t1 = b * t1

    movl    -4(%rbp), %eax
    addl    -24(%rbp), %eax
    movl    %eax, -24(%rbp)  # t2 = a + t2

    # c = t2
    movl    -24(%rbp), %eax
    movl    %eax, -12(%rbp)

    # output(c)
    movl    -12(%rbp), %esi
    movl    $.LC1, %edi
    movl    $0, %eax
    call    printf

    # return 0
    movl    $0, %eax
    leave
    ret
