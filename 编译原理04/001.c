#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

#define MAX_VARS 100
#define MAX_CODE_LENGTH 10000
#define MAX_LINE_LENGTH 256
#define MAX_FILENAME_LENGTH 256
#define MAX_TOKENS 1000


/* 词法分析器标记类型 */
typedef enum {
    TOKEN_INT,          // int
    TOKEN_IDENTIFIER,   // 标识符
    TOKEN_NUMBER,       // 数字
    TOKEN_INPUT,        // input
    TOKEN_OUTPUT,       // output
    TOKEN_LBRACE,       // {
    TOKEN_RBRACE,       // }
    TOKEN_LPAREN,       // (
    TOKEN_RPAREN,       // )
    TOKEN_SEMICOLON,    // ;
    TOKEN_ASSIGN,       // =
    TOKEN_PLUS,         // +
    TOKEN_MINUS,        // -
    TOKEN_MULTIPLY,     // *
    TOKEN_DIVIDE,       // /
    TOKEN_EOF           // 文件结束
} TokenType;

/* 词法分析器标记结构 */
typedef struct {
    TokenType type;
    char value[32];
    int line;
} Token;

/* 语法树节点类型 */
typedef enum {
    NODE_PROGRAM,
    NODE_DECLARATION,
    NODE_INPUT,
    NODE_OUTPUT,
    NODE_ASSIGNMENT,
    NODE_EXPRESSION,
    NODE_BINARY_OP,
    NODE_VARIABLE,
    NODE_CONSTANT
} NodeType;

/* 语法树节点结构 */
typedef struct ASTNode {
    NodeType type;
    char value[32];
    struct ASTNode* left;
    struct ASTNode* right;
} ASTNode;

/* 中间代码指令类型 */
typedef enum {
    IR_INPUT,       // input dest
    IR_OUTPUT,      // output src
    IR_ASSIGN,      // dest = src
    IR_ADD,         // dest = src1 + src2
    IR_SUB,         // dest = src1 - src2  
    IR_MUL,         // dest = src1 * src2
    IR_DIV,         // dest = src1 / src2
    IR_ASSIGN_CONST // dest = const
} IRType;

/* 中间代码指令结构 */
typedef struct {
    IRType type;
    char op[8];         // 操作符
    char dest[32];      // 目标操作数
    char src1[32];      // 源操作数1
    char src2[32];      // 源操作数2
    int constant;       // 常量值
} IRInstruction;

/* 变量结构 */
typedef struct {
    char name[32];
    int offset;
} Variable;

/* 编译器主结构 */
typedef struct {
    Variable vars[MAX_VARS];
    int var_count;
    int label_count;

    /* 词法分析结果 */
    Token tokens[MAX_TOKENS];
    int token_count;

    /* 语法分析结果 */
    ASTNode* ast_root;

    /* 中间代码结果 */
    IRInstruction ir_code[MAX_TOKENS];
    int ir_count;
    int temp_var_counter;  // 临时变量计数器

    /* 最终汇编代码 */
    char output[MAX_CODE_LENGTH];
    int output_pos;
} Compiler;

/* 函数声明 */
void compiler_init(Compiler* compiler);
void clear_input_buffer(void);
char* my_strtok_r(char* str, const char* delim, char** saveptr);
int is_temp_var(const char* name);
int get_temp_var_offset(Compiler* compiler, const char* temp_name);

/* 词法分析函数 */
void lexer(Compiler* compiler, const char* source);
void print_tokens(Compiler* compiler, FILE* output_file);

/* 语法分析函数 */
void parser(Compiler* compiler);
ASTNode* parse_statement(Compiler* compiler, int* pos);
ASTNode* parse_expression(Compiler* compiler, int* pos);
void print_ast(ASTNode* node, int depth, FILE* output_file);
void free_ast(ASTNode* node);

/* 中间代码生成函数 */
void generate_ir(Compiler* compiler);
char* generate_expression_ir(Compiler* compiler, ASTNode* node);
char* new_temp_var(Compiler* compiler);
void print_ir(Compiler* compiler, FILE* output_file);

/* 代码生成函数 */
void generate_assembly(Compiler* compiler);

/* 工具函数 */
int find_variable(Compiler* compiler, const char* name);
int add_variable(Compiler* compiler, const char* name);
void emit_code(Compiler* compiler, const char* format, ...);
char* new_label(Compiler* compiler);

/* 同时输出到屏幕和文件的函数 */
void print_to_both(FILE* file, const char* format, ...);

/* 清空输入缓冲区 */
void clear_input_buffer(void)
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

/* 自定义 strtok_r 实现 */
char* my_strtok_r(char* str, const char* delim, char** saveptr)
{
    char* token;
    char* end;

    if (str != NULL) {
        *saveptr = str;
    }

    if (*saveptr == NULL || **saveptr == '\0') {
        return NULL;
    }

    *saveptr += strspn(*saveptr, delim);
    if (**saveptr == '\0') {
        return NULL;
    }

    token = *saveptr;
    end = token + strcspn(token, delim);

    if (*end != '\0') {
        *end = '\0';
        *saveptr = end + 1;
    }
    else {
        *saveptr = end;
    }

    return token;
}

/* 判断是否为临时变量 */
int is_temp_var(const char* name)
{
    return (name[0] == 't' && isdigit((unsigned char)name[1]));
}

/* 获取临时变量的栈偏移量 */
int get_temp_var_offset(Compiler* compiler, const char* temp_name)
{
    /* 临时变量从普通变量之后开始分配 */
    int temp_id;
    if (sscanf_s(temp_name, "t%d", &temp_id) == 1) {
        return (compiler->var_count + temp_id + 1) * 4;
    }
    return -1;
}

/* 编译器初始化 */
void compiler_init(Compiler* compiler)
{
    compiler->var_count = 0;
    compiler->label_count = 0;
    compiler->token_count = 0;
    compiler->ast_root = NULL;
    compiler->ir_count = 0;
    compiler->temp_var_counter = 0;
    compiler->output_pos = 0;
    memset(compiler->output, 0, sizeof(compiler->output));
}

/* 查找变量 */
int find_variable(Compiler* compiler, const char* name)
{
    int i;
    for (i = 0; i < compiler->var_count; i++) {
        if (strcmp(compiler->vars[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

/* 添加变量 */
int add_variable(Compiler* compiler, const char* name)
{
    Variable* var;

    if (find_variable(compiler, name) != -1) {
        return 0;
    }

    if (compiler->var_count >= MAX_VARS) {
        return -1;
    }

    var = &compiler->vars[compiler->var_count];
    strncpy(var->name, name, sizeof(var->name) - 1);
    var->name[sizeof(var->name) - 1] = '\0';
    var->offset = (compiler->var_count + 1) * 4;
    compiler->var_count++;
    return 1;
}

/* 生成代码 */
void emit_code(Compiler* compiler, const char* format, ...)
{
    va_list args;
    int written;
    char buffer[1024];

    va_start(args, format);
    written = vsprintf_s(buffer, sizeof(buffer), format, args);
    va_end(args);

    if (written > 0 && compiler->output_pos + written < MAX_CODE_LENGTH) {
        strcpy(compiler->output + compiler->output_pos, buffer);
        compiler->output_pos += written;
    }
}

/* 生成标签 */
char* new_label(Compiler* compiler)
{
    static char label[32];
    sprintf_s(label, sizeof(label), ".L%d", compiler->label_count++);
    return label;
}

/* 生成临时变量名 */
char* new_temp_var(Compiler* compiler)
{
    static char temp_name[32];
    sprintf_s(temp_name, sizeof(temp_name), "t%d", compiler->temp_var_counter++);
    return temp_name;
}

/* 同时输出到屏幕和文件的函数 */
void print_to_both(FILE* file, const char* format, ...)
{
    va_list args;
    char buffer[1024];

    va_start(args, format);
    vsprintf_s(buffer, sizeof(buffer), format, args);
    va_end(args);

    /* 输出到屏幕 */
    printf("%s", buffer);

    /* 输出到文件 */
    if (file != NULL) {
        fprintf(file, "%s", buffer);
    }
}

/* 词法分析器 */
void lexer(Compiler* compiler, const char* source)
{
    char* source_copy;
    char* line;
    char* saveptr = NULL;
    int line_num = 1;
    char* pos;
    Token* current_token;

    source_copy = _strdup(source);
    if (source_copy == NULL) {
        fprintf(stderr, "内存分配失败：无法复制源代码\n");
        return;
    }

    line = my_strtok_r(source_copy, "\n", &saveptr);

    while (line != NULL) {
        pos = line;

        /* 跳过前导空格 */
        while (*pos == ' ' || *pos == '\t') pos++;

        /* 处理每一行的标记 */
        while (*pos != '\0') {
            if (compiler->token_count >= MAX_TOKENS) {
                break;
            }

            current_token = &compiler->tokens[compiler->token_count];
            current_token->line = line_num;

            /* 跳过空格 */
            while (*pos == ' ' || *pos == '\t') pos++;
            if (*pos == '\0') break;

            /* 识别标记 */
            if (strncmp(pos, "int", 3) == 0 && !isalnum((unsigned char)pos[3])) {
                current_token->type = TOKEN_INT;
                strcpy(current_token->value, "int");
                pos += 3;
                compiler->token_count++;
            }
            else if (strncmp(pos, "input", 5) == 0 && !isalnum((unsigned char)pos[5])) {
                current_token->type = TOKEN_INPUT;
                strcpy(current_token->value, "input");
                pos += 5;
                compiler->token_count++;
            }
            else if (strncmp(pos, "output", 6) == 0 && !isalnum((unsigned char)pos[6])) {
                current_token->type = TOKEN_OUTPUT;
                strcpy(current_token->value, "output");
                pos += 6;
                compiler->token_count++;
            }
            else if (*pos == '{') {
                current_token->type = TOKEN_LBRACE;
                strcpy(current_token->value, "{");
                pos++;
                compiler->token_count++;
            }
            else if (*pos == '}') {
                current_token->type = TOKEN_RBRACE;
                strcpy(current_token->value, "}");
                pos++;
                compiler->token_count++;
            }
            else if (*pos == '(') {
                current_token->type = TOKEN_LPAREN;
                strcpy(current_token->value, "(");
                pos++;
                compiler->token_count++;
            }
            else if (*pos == ')') {
                current_token->type = TOKEN_RPAREN;
                strcpy(current_token->value, ")");
                pos++;
                compiler->token_count++;
            }
            else if (*pos == ';') {
                current_token->type = TOKEN_SEMICOLON;
                strcpy(current_token->value, ";");
                pos++;
                compiler->token_count++;
            }
            else if (*pos == '=') {
                current_token->type = TOKEN_ASSIGN;
                strcpy(current_token->value, "=");
                pos++;
                compiler->token_count++;
            }
            else if (*pos == '+') {
                current_token->type = TOKEN_PLUS;
                strcpy(current_token->value, "+");
                pos++;
                compiler->token_count++;
            }
            else if (*pos == '-') {
                current_token->type = TOKEN_MINUS;
                strcpy(current_token->value, "-");
                pos++;
                compiler->token_count++;
            }
            else if (*pos == '*') {
                current_token->type = TOKEN_MULTIPLY;
                strcpy(current_token->value, "*");
                pos++;
                compiler->token_count++;
            }
            else if (*pos == '/') {
                current_token->type = TOKEN_DIVIDE;
                strcpy(current_token->value, "/");
                pos++;
                compiler->token_count++;
            }
            else if (isalpha((unsigned char)*pos)) {
                /* 标识符 */
                int i = 0;
                current_token->type = TOKEN_IDENTIFIER;
                while (isalnum((unsigned char)*pos) && i < 31) {
                    current_token->value[i++] = *pos++;
                }
                current_token->value[i] = '\0';
                compiler->token_count++;
            }
            else if (isdigit((unsigned char)*pos)) {
                /* 数字 */
                int i = 0;
                current_token->type = TOKEN_NUMBER;
                while (isdigit((unsigned char)*pos) && i < 31) {
                    current_token->value[i++] = *pos++;
                }
                current_token->value[i] = '\0';
                compiler->token_count++;
            }
            else {
                /* 未知字符，跳过 */
                pos++;
            }
        }

        line = my_strtok_r(NULL, "\n", &saveptr);
        line_num++;
    }

    /* 添加EOF标记 */
    if (compiler->token_count < MAX_TOKENS) {
        compiler->tokens[compiler->token_count].type = TOKEN_EOF;
        strcpy(compiler->tokens[compiler->token_count].value, "EOF");
        compiler->tokens[compiler->token_count].line = line_num;
        compiler->token_count++;
    }

    free(source_copy);
}

/* 打印词法分析结果 */
void print_tokens(Compiler* compiler, FILE* output_file)
{
    int i;
    const char* type_str;

    print_to_both(output_file, "=== 词法分析结果 ===\n");
    print_to_both(output_file, "%-12s %-15s %s\n", "行号", "标记类型", "值");
    print_to_both(output_file, "----------------------------------------\n");

    for (i = 0; i < compiler->token_count; i++) {
        switch (compiler->tokens[i].type) {
        case TOKEN_INT: type_str = "TOKEN_INT"; break;
        case TOKEN_IDENTIFIER: type_str = "TOKEN_IDENTIFIER"; break;
        case TOKEN_NUMBER: type_str = "TOKEN_NUMBER"; break;
        case TOKEN_INPUT: type_str = "TOKEN_INPUT"; break;
        case TOKEN_OUTPUT: type_str = "TOKEN_OUTPUT"; break;
        case TOKEN_LBRACE: type_str = "TOKEN_LBRACE"; break;
        case TOKEN_RBRACE: type_str = "TOKEN_RBRACE"; break;
        case TOKEN_LPAREN: type_str = "TOKEN_LPAREN"; break;
        case TOKEN_RPAREN: type_str = "TOKEN_RPAREN"; break;
        case TOKEN_SEMICOLON: type_str = "TOKEN_SEMICOLON"; break;
        case TOKEN_ASSIGN: type_str = "TOKEN_ASSIGN"; break;
        case TOKEN_PLUS: type_str = "TOKEN_PLUS"; break;
        case TOKEN_MINUS: type_str = "TOKEN_MINUS"; break;
        case TOKEN_MULTIPLY: type_str = "TOKEN_MULTIPLY"; break;
        case TOKEN_DIVIDE: type_str = "TOKEN_DIVIDE"; break;
        case TOKEN_EOF: type_str = "TOKEN_EOF"; break;
        default: type_str = "UNKNOWN"; break;
        }
        print_to_both(output_file, "%-12d %-15s %s\n",
            compiler->tokens[i].line, type_str, compiler->tokens[i].value);
    }
    print_to_both(output_file, "\n");
}

/* 语法分析器 */
void parser(Compiler* compiler)
{
    int pos = 0;
    ASTNode* last_stmt = NULL;
    ASTNode* current_stmt;

    /* 跳过开头的 { */
    if (pos < compiler->token_count && compiler->tokens[pos].type == TOKEN_LBRACE) {
        pos++;
    }

    /* 创建程序节点 */
    compiler->ast_root = (ASTNode*)malloc(sizeof(ASTNode));
    if (compiler->ast_root == NULL) {
        fprintf(stderr, "内存分配失败：无法创建 AST 根节点\n");
        return;
    }

    compiler->ast_root->type = NODE_PROGRAM;
    strcpy(compiler->ast_root->value, "program");
    compiler->ast_root->left = NULL;
    compiler->ast_root->right = NULL;

    /* 解析所有语句 */
    while (pos < compiler->token_count && compiler->tokens[pos].type != TOKEN_RBRACE) {
        current_stmt = parse_statement(compiler, &pos);
        if (current_stmt != NULL) {
            if (last_stmt == NULL) {
                compiler->ast_root->left = current_stmt;
            }
            else {
                last_stmt->right = current_stmt;
            }
            last_stmt = current_stmt;
        }
    }
}

/* 解析语句 */
ASTNode* parse_statement(Compiler* compiler, int* pos)
{
    ASTNode* node = NULL;

    if (*pos >= compiler->token_count) return NULL;

    if (compiler->tokens[*pos].type == TOKEN_INT) {
        /* 变量声明 */
        node = (ASTNode*)malloc(sizeof(ASTNode));
        if (node == NULL) {
            fprintf(stderr, "内存分配失败：DECLARATION 节点\n");
            return NULL;
        }
        node->type = NODE_DECLARATION;
        strcpy(node->value, compiler->tokens[*pos + 1].value);
        node->left = NULL;
        node->right = NULL;
        *pos += 3; /* int + identifier + ; */
        add_variable(compiler, node->value);
    }
    else if (compiler->tokens[*pos].type == TOKEN_INPUT) {
        /* input语句 */
        node = (ASTNode*)malloc(sizeof(ASTNode));
        if (node == NULL) {
            fprintf(stderr, "内存分配失败：INPUT 节点\n");
            return NULL;
        }
        node->type = NODE_INPUT;
        strcpy(node->value, compiler->tokens[*pos + 2].value); /* input ( identifier ) */
        node->left = NULL;
        node->right = NULL;
        *pos += 5; /* input + ( + identifier + ) + ; */
    }
    else if (compiler->tokens[*pos].type == TOKEN_OUTPUT) {
        /* output语句 */
        node = (ASTNode*)malloc(sizeof(ASTNode));
        if (node == NULL) {
            fprintf(stderr, "内存分配失败：OUTPUT 节点\n");
            return NULL;
        }
        node->type = NODE_OUTPUT;
        strcpy(node->value, compiler->tokens[*pos + 2].value); /* output ( identifier ) */
        node->left = NULL;
        node->right = NULL;
        *pos += 5; /* output + ( + identifier + ) + ; */
    }
    else if (compiler->tokens[*pos].type == TOKEN_IDENTIFIER &&
        *pos + 1 < compiler->token_count &&
        compiler->tokens[*pos + 1].type == TOKEN_ASSIGN) {
        /* 赋值语句 */
        node = (ASTNode*)malloc(sizeof(ASTNode));
        if (node == NULL) {
            fprintf(stderr, "内存分配失败：ASSIGNMENT 节点\n");
            return NULL;
        }
        node->type = NODE_ASSIGNMENT;
        strcpy(node->value, compiler->tokens[*pos].value);
        *pos += 2; /* identifier + = */
        node->left = parse_expression(compiler, pos);
        node->right = NULL;
        (*pos)++; /* 跳过 ; */
    }
    else {
        (*pos)++; /* 跳过无法识别的标记 */
    }

    return node;
}

/* 解析表达式 */
ASTNode* parse_expression(Compiler* compiler, int* pos)
{
    ASTNode* node = NULL;

    if (*pos >= compiler->token_count) return NULL;

    /* 简单的表达式解析 */
    if (compiler->tokens[*pos].type == TOKEN_IDENTIFIER) {
        if (*pos + 2 < compiler->token_count &&
            (compiler->tokens[*pos + 1].type == TOKEN_PLUS ||
                compiler->tokens[*pos + 1].type == TOKEN_MINUS ||
                compiler->tokens[*pos + 1].type == TOKEN_MULTIPLY ||
                compiler->tokens[*pos + 1].type == TOKEN_DIVIDE)) {
            /* 二元运算表达式 */
            node = (ASTNode*)malloc(sizeof(ASTNode));
            if (node == NULL) {
                fprintf(stderr, "内存分配失败：BINARY_OP 节点\n");
                return NULL;
            }
            node->type = NODE_BINARY_OP;
            strcpy(node->value, compiler->tokens[*pos + 1].value);

            node->left = (ASTNode*)malloc(sizeof(ASTNode));
            if (node->left == NULL) {
                fprintf(stderr, "内存分配失败：BINARY_OP 左子树\n");
                free(node);
                return NULL;
            }
            node->left->type = NODE_VARIABLE;
            strcpy(node->left->value, compiler->tokens[*pos].value);
            node->left->left = NULL;
            node->left->right = NULL;

            *pos += 2;
            node->right = parse_expression(compiler, pos);
        }
        else {
            /* 单个变量 */
            node = (ASTNode*)malloc(sizeof(ASTNode));
            if (node == NULL) {
                fprintf(stderr, "内存分配失败：VARIABLE 节点\n");
                return NULL;
            }
            node->type = NODE_VARIABLE;
            strcpy(node->value, compiler->tokens[*pos].value);
            node->left = NULL;
            node->right = NULL;
            (*pos)++;
        }
    }
    else if (compiler->tokens[*pos].type == TOKEN_NUMBER) {
        /* 常量 */
        node = (ASTNode*)malloc(sizeof(ASTNode));
        if (node == NULL) {
            fprintf(stderr, "内存分配失败：CONSTANT 节点\n");
            return NULL;
        }
        node->type = NODE_CONSTANT;
        strcpy(node->value, compiler->tokens[*pos].value);
        node->left = NULL;
        node->right = NULL;
        (*pos)++;
    }

    return node;
}

/* 打印语法树 */
void print_ast(ASTNode* node, int depth, FILE* output_file)
{
    int i;

    if (node == NULL) return;

    for (i = 0; i < depth; i++) {
        print_to_both(output_file, "  ");
    }

    switch (node->type) {
    case NODE_PROGRAM: print_to_both(output_file, "PROGRAM\n"); break;
    case NODE_DECLARATION: print_to_both(output_file, "DECLARATION: %s\n", node->value); break;
    case NODE_INPUT: print_to_both(output_file, "INPUT: %s\n", node->value); break;
    case NODE_OUTPUT: print_to_both(output_file, "OUTPUT: %s\n", node->value); break;
    case NODE_ASSIGNMENT: print_to_both(output_file, "ASSIGNMENT: %s\n", node->value); break;
    case NODE_BINARY_OP: print_to_both(output_file, "BINARY_OP: %s\n", node->value); break;
    case NODE_VARIABLE: print_to_both(output_file, "VARIABLE: %s\n", node->value); break;
    case NODE_CONSTANT: print_to_both(output_file, "CONSTANT: %s\n", node->value); break;
    default: print_to_both(output_file, "UNKNOWN\n"); break;
    }

    print_ast(node->left, depth + 1, output_file);
    print_ast(node->right, depth + 1, output_file);
}

/* 释放语法树 */
void free_ast(ASTNode* node)
{
    if (node == NULL) return;
    free_ast(node->left);
    free_ast(node->right);
    free(node);
}

/* 递归生成表达式中间代码 */
char* generate_expression_ir(Compiler* compiler, ASTNode* node)
{
    IRInstruction* ir;
    char* temp1;
    char* temp2;
    char* result_temp;

    if (node == NULL) return NULL;

    switch (node->type) {
    case NODE_VARIABLE:
        /* 变量直接返回其名称 */
        return node->value;

    case NODE_CONSTANT:
        /* 常量生成加载指令 */
        if (compiler->ir_count >= MAX_TOKENS) return NULL;

        ir = &compiler->ir_code[compiler->ir_count];
        result_temp = new_temp_var(compiler);

        ir->type = IR_ASSIGN_CONST;
        strcpy(ir->op, "=");
        strcpy(ir->dest, result_temp);
        strcpy(ir->src1, node->value);
        ir->constant = atoi(node->value);
        compiler->ir_count++;

        return result_temp;

    case NODE_BINARY_OP:
        /* 递归处理左右子树 */
        temp1 = generate_expression_ir(compiler, node->left);
        temp2 = generate_expression_ir(compiler, node->right);

        if (temp1 == NULL || temp2 == NULL) {
            return NULL;
        }

        /* 生成二元运算指令 */
        if (compiler->ir_count >= MAX_TOKENS) return NULL;

        ir = &compiler->ir_code[compiler->ir_count];
        result_temp = new_temp_var(compiler);

        if (strcmp(node->value, "+") == 0) {
            ir->type = IR_ADD;
            strcpy(ir->op, "+");
        }
        else if (strcmp(node->value, "-") == 0) {
            ir->type = IR_SUB;
            strcpy(ir->op, "-");
        }
        else if (strcmp(node->value, "*") == 0) {
            ir->type = IR_MUL;
            strcpy(ir->op, "*");
        }
        else if (strcmp(node->value, "/") == 0) {
            ir->type = IR_DIV;
            strcpy(ir->op, "/");
        }
        else {
            return NULL;
        }

        strcpy(ir->dest, result_temp);
        strcpy(ir->src1, temp1);
        strcpy(ir->src2, temp2);
        compiler->ir_count++;

        return result_temp;

    default:
        return NULL;
    }
}

/* 生成中间代码 */
void generate_ir(Compiler* compiler)
{
    ASTNode* current = compiler->ast_root ? compiler->ast_root->left : NULL;
    IRInstruction* ir;
    char* temp_var;

    compiler->temp_var_counter = 0;  // 重置临时变量计数器

    while (current != NULL) {
        if (compiler->ir_count >= MAX_TOKENS) break;

        switch (current->type) {
        case NODE_INPUT:
            if (compiler->ir_count >= MAX_TOKENS) break;
            ir = &compiler->ir_code[compiler->ir_count];
            ir->type = IR_INPUT;
            strcpy(ir->op, "input");
            strcpy(ir->dest, current->value);
            compiler->ir_count++;
            break;

        case NODE_OUTPUT:
            if (compiler->ir_count >= MAX_TOKENS) break;
            ir = &compiler->ir_code[compiler->ir_count];
            ir->type = IR_OUTPUT;
            strcpy(ir->op, "output");
            strcpy(ir->src1, current->value);
            compiler->ir_count++;
            break;

        case NODE_ASSIGNMENT:
            /* 递归生成表达式的中间代码 */
            temp_var = generate_expression_ir(compiler, current->left);
            if (temp_var != NULL && compiler->ir_count < MAX_TOKENS) {
                /* 生成赋值指令 */
                ir = &compiler->ir_code[compiler->ir_count];
                ir->type = IR_ASSIGN;
                strcpy(ir->op, "=");
                strcpy(ir->dest, current->value);
                strcpy(ir->src1, temp_var);
                compiler->ir_count++;
            }
            break;

        default:
            break;
        }

        current = current->right;
    }
}

/* 改进的中间代码打印函数 */
void print_ir(Compiler* compiler, FILE* output_file)
{
    int i;
    print_to_both(output_file, "=== 中间代码生成结果 (三地址码) ===\n");
    print_to_both(output_file, "%-6s %-8s %-10s %-10s %-10s\n", "序号", "操作", "目标", "源1", "源2");
    print_to_both(output_file, "-------------------------------------------------\n");

    for (i = 0; i < compiler->ir_count; i++) {
        IRInstruction* ir = &compiler->ir_code[i];

        switch (ir->type) {
        case IR_INPUT:
            print_to_both(output_file, "%-6d %-8s %-10s %-10s %-10s\n",
                i, "input", ir->dest, "", "");
            break;

        case IR_OUTPUT:
            print_to_both(output_file, "%-6d %-8s %-10s %-10s %-10s\n",
                i, "output", "", ir->src1, "");
            break;

        case IR_ASSIGN:
            print_to_both(output_file, "%-6d %-8s %-10s %-10s %-10s\n",
                i, "=", ir->dest, ir->src1, "");
            break;

        case IR_ASSIGN_CONST:
            print_to_both(output_file, "%-6d %-8s %-10s %-10s %-10s\n",
                i, "= const", ir->dest, ir->src1, "");
            break;

        case IR_ADD:
            print_to_both(output_file, "%-6d %-8s %-10s %-10s %-10s\n",
                i, "+", ir->dest, ir->src1, ir->src2);
            break;

        case IR_SUB:
            print_to_both(output_file, "%-6d %-8s %-10s %-10s %-10s\n",
                i, "-", ir->dest, ir->src1, ir->src2);
            break;

        case IR_MUL:
            print_to_both(output_file, "%-6d %-8s %-10s %-10s %-10s\n",
                i, "*", ir->dest, ir->src1, ir->src2);
            break;

        case IR_DIV:
            print_to_both(output_file, "%-6d %-8s %-10s %-10s %-10s\n",
                i, "/", ir->dest, ir->src1, ir->src2);
            break;

        default:
            print_to_both(output_file, "%-6d %-8s %-10s %-10s %-10s\n",
                i, "unknown", "", "", "");
            break;
        }
    }
    print_to_both(output_file, "\n");

    /* 添加中间代码的文本表示 */
    print_to_both(output_file, "=== 中间代码文本表示 ===\n");
    for (i = 0; i < compiler->ir_count; i++) {
        IRInstruction* ir = &compiler->ir_code[i];

        switch (ir->type) {
        case IR_INPUT:
            print_to_both(output_file, "%-4d: input %s\n", i, ir->dest);
            break;

        case IR_OUTPUT:
            print_to_both(output_file, "%-4d: output %s\n", i, ir->src1);
            break;

        case IR_ASSIGN:
            print_to_both(output_file, "%-4d: %s = %s\n", i, ir->dest, ir->src1);
            break;

        case IR_ASSIGN_CONST:
            print_to_both(output_file, "%-4d: %s = %s\n", i, ir->dest, ir->src1);
            break;

        case IR_ADD:
            print_to_both(output_file, "%-4d: %s = %s + %s\n", i, ir->dest, ir->src1, ir->src2);
            break;

        case IR_SUB:
            print_to_both(output_file, "%-4d: %s = %s - %s\n", i, ir->dest, ir->src1, ir->src2);
            break;

        case IR_MUL:
            print_to_both(output_file, "%-4d: %s = %s * %s\n", i, ir->dest, ir->src1, ir->src2);
            break;

        case IR_DIV:
            print_to_both(output_file, "%-4d: %s = %s / %s\n", i, ir->dest, ir->src1, ir->src2);
            break;

        default:
            print_to_both(output_file, "%-4d: unknown\n", i);
            break;
        }
    }
    print_to_both(output_file, "\n");
}

/* 生成汇编代码 */
void generate_assembly(Compiler* compiler)
{
    int i;
    int stack_size;
    IRInstruction* ir;
    int dest_var;
    int src1_var;
    int src2_var;

    /* 添加汇编头部 */
    emit_code(compiler, "=== 最终汇编代码 ===\n");
    emit_code(compiler, ".section .rodata\n");
    emit_code(compiler, ".LC0:\n");
    emit_code(compiler, "    .string \"%%d\"\n");
    emit_code(compiler, ".LC1:\n");
    emit_code(compiler, "    .string \"%%d\\\\n\"\n");
    emit_code(compiler, "\n");
    emit_code(compiler, ".section .text\n");
    emit_code(compiler, ".globl main\n");
    emit_code(compiler, "main:\n");
    emit_code(compiler, "    pushq   %%rbp\n");
    emit_code(compiler, "    movq    %%rsp, %%rbp\n");

    /* 分配栈空间 - 包括临时变量 */
    stack_size = (compiler->var_count + compiler->temp_var_counter) * 4 + 16;
    emit_code(compiler, "    subq    $%d, %%rsp  # 为局部变量分配空间\n\n", stack_size);

    /* 根据中间代码生成汇编 */
    for (i = 0; i < compiler->ir_count; i++) {
        ir = &compiler->ir_code[i];

        switch (ir->type) {
        case IR_INPUT:
            dest_var = find_variable(compiler, ir->dest);
            if (dest_var != -1) {
                emit_code(compiler, "    # input(%s)\n", ir->dest);
                emit_code(compiler, "    leaq    -%d(%%rbp), %%rsi\n", compiler->vars[dest_var].offset);
                emit_code(compiler, "    movl    $.LC0, %%edi\n");
                emit_code(compiler, "    movl    $0, %%eax\n");
                emit_code(compiler, "    call    scanf\n");
            }
            break;

        case IR_OUTPUT:
            src1_var = find_variable(compiler, ir->src1);
            if (src1_var != -1) {
                emit_code(compiler, "    # output(%s)\n", ir->src1);
                emit_code(compiler, "    movl    -%d(%%rbp), %%esi\n", compiler->vars[src1_var].offset);
                emit_code(compiler, "    movl    $.LC1, %%edi\n");
                emit_code(compiler, "    movl    $0, %%eax\n");
                emit_code(compiler, "    call    printf\n");
            }
            break;

        case IR_ASSIGN:
            /* 处理赋值指令：dest = src1 */
            if (is_temp_var(ir->dest)) {
                /* 目标为临时变量，需要找到其存储位置 */
                dest_var = get_temp_var_offset(compiler, ir->dest);
                src1_var = find_variable(compiler, ir->src1);
                if (src1_var != -1) {
                    emit_code(compiler, "    # %s = %s\n", ir->dest, ir->src1);
                    emit_code(compiler, "    movl    -%d(%%rbp), %%eax\n", compiler->vars[src1_var].offset);
                    emit_code(compiler, "    movl    %%eax, -%d(%%rbp)\n", dest_var);
                }
            }
            else {
                /* 目标为普通变量 */
                dest_var = find_variable(compiler, ir->dest);
                if (is_temp_var(ir->src1)) {
                    /* 源为临时变量 */
                    src1_var = get_temp_var_offset(compiler, ir->src1);
                    if (dest_var != -1) {
                        emit_code(compiler, "    # %s = %s\n", ir->dest, ir->src1);
                        emit_code(compiler, "    movl    -%d(%%rbp), %%eax\n", src1_var);
                        emit_code(compiler, "    movl    %%eax, -%d(%%rbp)\n", compiler->vars[dest_var].offset);
                    }
                }
                else {
                    /* 源为普通变量 */
                    src1_var = find_variable(compiler, ir->src1);
                    if (dest_var != -1 && src1_var != -1) {
                        emit_code(compiler, "    # %s = %s\n", ir->dest, ir->src1);
                        emit_code(compiler, "    movl    -%d(%%rbp), %%eax\n", compiler->vars[src1_var].offset);
                        emit_code(compiler, "    movl    %%eax, -%d(%%rbp)\n", compiler->vars[dest_var].offset);
                    }
                }
            }
            break;

        case IR_ASSIGN_CONST:
            /* 处理常量赋值：dest = const */
            if (is_temp_var(ir->dest)) {
                dest_var = get_temp_var_offset(compiler, ir->dest);
                emit_code(compiler, "    # %s = %s\n", ir->dest, ir->src1);
                emit_code(compiler, "    movl    $%s, %%eax\n", ir->src1);
                emit_code(compiler, "    movl    %%eax, -%d(%%rbp)\n", dest_var);
            }
            else {
                dest_var = find_variable(compiler, ir->dest);
                if (dest_var != -1) {
                    emit_code(compiler, "    # %s = %s\n", ir->dest, ir->src1);
                    emit_code(compiler, "    movl    $%s, %%eax\n", ir->src1);
                    emit_code(compiler, "    movl    %%eax, -%d(%%rbp)\n", compiler->vars[dest_var].offset);
                }
            }
            break;

        case IR_ADD:
            /* 处理加法：dest = src1 + src2 */
            if (is_temp_var(ir->dest)) {
                dest_var = get_temp_var_offset(compiler, ir->dest);
                /* 处理源操作数1 */
                if (is_temp_var(ir->src1)) {
                    src1_var = get_temp_var_offset(compiler, ir->src1);
                    emit_code(compiler, "    movl    -%d(%%rbp), %%eax\n", src1_var);
                }
                else {
                    src1_var = find_variable(compiler, ir->src1);
                    if (src1_var != -1) {
                        emit_code(compiler, "    movl    -%d(%%rbp), %%eax\n", compiler->vars[src1_var].offset);
                    }
                    else {
                        /* 可能是常量 */
                        emit_code(compiler, "    movl    $%s, %%eax\n", ir->src1);
                    }
                }

                /* 处理源操作数2 */
                if (is_temp_var(ir->src2)) {
                    src2_var = get_temp_var_offset(compiler, ir->src2);
                    emit_code(compiler, "    addl    -%d(%%rbp), %%eax\n", src2_var);
                }
                else {
                    src2_var = find_variable(compiler, ir->src2);
                    if (src2_var != -1) {
                        emit_code(compiler, "    addl    -%d(%%rbp), %%eax\n", compiler->vars[src2_var].offset);
                    }
                    else {
                        /* 可能是常量 */
                        emit_code(compiler, "    addl    $%s, %%eax\n", ir->src2);
                    }
                }

                emit_code(compiler, "    movl    %%eax, -%d(%%rbp)  # %s = %s + %s\n",
                    dest_var, ir->dest, ir->src1, ir->src2);
            }
            break;

        case IR_MUL:
            /* 处理乘法：dest = src1 * src2 */
            if (is_temp_var(ir->dest)) {
                dest_var = get_temp_var_offset(compiler, ir->dest);
                /* 处理源操作数1 */
                if (is_temp_var(ir->src1)) {
                    src1_var = get_temp_var_offset(compiler, ir->src1);
                    emit_code(compiler, "    movl    -%d(%%rbp), %%eax\n", src1_var);
                }
                else {
                    src1_var = find_variable(compiler, ir->src1);
                    if (src1_var != -1) {
                        emit_code(compiler, "    movl    -%d(%%rbp), %%eax\n", compiler->vars[src1_var].offset);
                    }
                    else {
                        /* 可能是常量 */
                        emit_code(compiler, "    movl    $%s, %%eax\n", ir->src1);
                    }
                }

                /* 处理源操作数2 */
                if (is_temp_var(ir->src2)) {
                    src2_var = get_temp_var_offset(compiler, ir->src2);
                    emit_code(compiler, "    imull   -%d(%%rbp), %%eax\n", src2_var);
                }
                else {
                    src2_var = find_variable(compiler, ir->src2);
                    if (src2_var != -1) {
                        emit_code(compiler, "    imull   -%d(%%rbp), %%eax\n", compiler->vars[src2_var].offset);
                    }
                    else {
                        /* 可能是常量 */
                        emit_code(compiler, "    imull   $%s, %%eax\n", ir->src2);
                    }
                }

                emit_code(compiler, "    movl    %%eax, -%d(%%rbp)  # %s = %s * %s\n",
                    dest_var, ir->dest, ir->src1, ir->src2);
            }
            break;
        }
        emit_code(compiler, "\n");
    }

    /* 添加程序结束部分 */
    emit_code(compiler, "    # return 0\n");
    emit_code(compiler, "    movl    $0, %%eax\n");
    emit_code(compiler, "    leave\n");
    emit_code(compiler, "    ret\n");
}

/* 主函数 */
int main(void)
{
    FILE* input_file;
    FILE* output_file;
    char source[MAX_CODE_LENGTH];
    char buffer[MAX_LINE_LENGTH];
    char input_filename[MAX_FILENAME_LENGTH];
    char output_filename[MAX_FILENAME_LENGTH];
    static Compiler compiler;  /* 静态分配，减小栈使用 */
    int choice;

    printf("=========================================\n");
    printf("       简单C语言编译器 (完整版)\n");
    printf("=========================================\n");

    while (1) {
        printf("\n请选择操作:\n");
        printf("1. 编译C源文件\n");
        printf("2. 退出程序\n");
        printf("请选择 (1-2): ");

        if (scanf("%d", &choice) != 1) {
            printf("输入错误，请重新输入！\n");
            clear_input_buffer();
            continue;
        }
        clear_input_buffer();

        if (choice == 2) {
            printf("程序退出，再见！\n");
            break;
        }
        else if (choice != 1) {
            printf("无效选择，请重新输入！\n");
            continue;
        }

        /* 获取输入文件名 */
        printf("\n请输入C源文件名 (例如: source.c): ");
        if (fgets(input_filename, sizeof(input_filename), stdin) == NULL) {
            printf("读取文件名失败！\n");
            continue;
        }
        input_filename[strcspn(input_filename, "\n")] = '\0';

        if (strlen(input_filename) == 0) {
            printf("文件名不能为空！\n");
            continue;
        }

        /* 获取输出文件名 */
        printf("请输入输出文件名 (例如: output.s): ");
        if (fgets(output_filename, sizeof(output_filename), stdin) == NULL) {
            printf("读取文件名失败！\n");
            continue;
        }
        output_filename[strcspn(output_filename, "\n")] = '\0';

        if (strlen(output_filename) == 0) {
            printf("输出文件名不能为空！\n");
            continue;
        }

        /* 读取源文件 */
        input_file = fopen(input_filename, "r");
        if (!input_file) {
            printf("错误: 无法打开输入文件 '%s'\n", input_filename);
            continue;
        }

        memset(source, 0, sizeof(source));
        while (fgets(buffer, sizeof(buffer), input_file)) {
            size_t curr_len = strlen(source);
            size_t buf_len = strlen(buffer);

            if (curr_len + buf_len < MAX_CODE_LENGTH) {
                strncat(source, buffer, MAX_CODE_LENGTH - curr_len - 1);
            }
            else {
                /* 只拼接还能容纳的部分，防止越界 */
                strncat(source, buffer, MAX_CODE_LENGTH - curr_len - 1);
                source[MAX_CODE_LENGTH - 1] = '\0';
                printf("警告: 源文件过大，部分内容被截断\n");
                break;
            }
        }
        source[MAX_CODE_LENGTH - 1] = '\0';
        fclose(input_file);

        printf("\n正在编译文件: %s\n", input_filename);

        /* 初始化编译器 */
        compiler_init(&compiler);

        /* 词法分析 */
        printf("1. 进行词法分析...\n");
        lexer(&compiler, source);

        /* 输出到文件 */
        output_file = fopen(output_filename, "w");
        if (!output_file) {
            printf("错误: 无法创建输出文件 '%s'\n", output_filename);
            continue;
        }

        /* 写入完整的编译结果 */
        print_to_both(output_file, "编译结果文件: %s -> %s\n\n", input_filename, output_filename);
        print_to_both(output_file, "源程序:\n%s\n", source);
        print_to_both(output_file, "----------------------------------------\n\n");

        /* 显示词法分析结果 */
        printf("\n词法分析结果:\n");
        print_tokens(&compiler, output_file);

        /* 语法分析 */
        printf("2. 进行语法分析...\n");
        parser(&compiler);

        /* 显示语法分析结果 */
        printf("\n语法分析结果:\n");
        print_to_both(output_file, "=== 语法分析结果 ===\n");
        print_ast(compiler.ast_root, 0, output_file);
        print_to_both(output_file, "\n");

        /* 中间代码生成 */
        printf("3. 生成中间代码...\n");
        generate_ir(&compiler);

        /* 显示中间代码结果 */
        printf("\n中间代码生成结果:\n");
        print_ir(&compiler, output_file);

        /* 目标代码生成 */
        printf("4. 生成汇编代码...\n");
        generate_assembly(&compiler);

        /* 显示汇编代码结果 */
        printf("\n汇编代码生成结果:\n");
        print_to_both(output_file, "%s", compiler.output);

        fclose(output_file);

        printf("\n编译成功！\n");
        printf("输入文件: %s\n", input_filename);
        printf("输出文件: %s\n", output_filename);
        printf("编译过程完成！\n");

        /* 释放语法树 */
        if (compiler.ast_root) {
            free_ast(compiler.ast_root);
            compiler.ast_root = NULL;
        }

        printf("\n按回车键继续...");
        clear_input_buffer();
    }

    return 0;
}
