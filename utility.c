//
// Created by zhangyukun on 2022/4/11.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "globals.h"
#include "utility.h"

static char* sourcePtr = NULL;             // 源代码打印指针

static void printType(int type);
static void printOperator(int op);
static void printTab(int n);

/**
 * @brief   载入源代码
 *
 * 通过文件指针打开指定源代码文件, 并将文件中内容读入到源代码缓冲区.
 *
 * @param   fileName    源代码文件名
 * @return  void
 * */
void loadSrc(const char* fileName){
    FILE* filePtr = NULL;           // 文件指针.
    long size = 0;                 // 存储源代码长度.

    // 打开源代码文件.
    filePtr = fopen(fileName,"r");
    // 判断文件是否成功打开.
    if (filePtr == NULL) {
        // 打开文件失败, 打印错误信息.
        printErrorInformation("Could Not Open Source Code:", fileName);
        exit(1);
    }
    // 分配缓冲区读取源代码.
    source = (char*)malloc(MAX_SIZE);
    // 判断源代码区是否分配成功.
    if (source == NULL) {
        // 分配源代码区失败, 打印错误信息.
        printErrorInformation("Could Not Malloc for Source Code", NULL);
        exit(1);
    }
    // 移动指针到文件末尾 , 获取文件长度.
    fseek(filePtr,0,SEEK_END);
    size = ftell(filePtr);
    // 恢复指针到文件首.
    fseek(filePtr,0,SEEK_SET);
    // 读取源代码到文件缓冲区.
    size = (long)fread(source,sizeof(char),size,filePtr);
    // 判断读取源代码是否成功.
    if (size == 0) {
        // 读取源代码失败, 打印错误信息.
        printErrorInformation("Could Not Read Source Code:", fileName);
        exit(1);
    }
    // 初始化源代码指针, 源代码备份指针.
    sourcePtr = sourceDump = source;
    // 关闭源代码文件.
    fclose(filePtr);
}

/**
 * @brief 初始化编译器
 *
 * 为编译器中用到的符号表, 节点栈, 代码段, 数据段, 堆栈段的分配存储区; 并初始化存储区空间与相关指针
 *
 * @param   void
 * @return  void
 * */
void init() {
    // 分配符号表存储区
    symbolTable = (sSymbol*)malloc(MAX_SIZE * sizeof(sSymbol));
    // 分配代码段存储区
    code = (long long*)malloc(MAX_SIZE * sizeof(long long));
    // 分配数据段存储区
    data = (long long*)malloc(MAX_SIZE * sizeof(long long));
    // 分配堆栈段存储区
    stack = (long long*)malloc(MAX_SIZE * sizeof(long long));
    // 判断初始化是否成功
    if (symbolTable == NULL || code == NULL || data == NULL || stack == NULL) {
        // 初始化失败, 打印错误信息
        printErrorInformation("Fail to Init", NULL);
        exit(1);
    }
    // 初始化存储区
    memset(symbolTable, 0, MAX_SIZE * sizeof(sSymbol));
    memset(code, 0, MAX_SIZE * sizeof(long long));
    memset(data, 0, MAX_SIZE * sizeof(long long));
    memset(stack, 0, MAX_SIZE * sizeof(long long));
    // 初始化符号表指针
    symbolPtr = symbolTable;
    // 初始化代码备份指针
    codeDump = code;
}

/**
 * @brief   打印词素 token
 *
 * 功能函数, 打印词法分析输出的词素
 *
 * @param   lineNo  行号
 * @return  void
 * */
void printToken(int lineNo) {
    // 判断词素类别, 并打印相关信息
    switch (token) {
        case Id:
            printf("\t%-3d: Id             --->   %s\n",lineNo, tokenString);
            break;
        case Char + Ptr:
            printf("\t%-3d: String         --->   %s\n",lineNo, tokenString);
            break;
        case Char:
            printf("\t%-3d: Char           --->   %c\n",lineNo, (char)tokenValue);
            break;
        case Num:
            printf("\t%-3d: Num            --->   %lld\n",lineNo, tokenValue);
            break;
        case CHAR:
        case INT:
        case IF:
        case ELSE:
        case RETURN:
        case WHILE:
        case FOR:
        case DO:
        case VOID:
            printf("\t%-3d: reserved word  --->   %s\n",lineNo, tokenString);
            break;
        case Assign:
            printf("\t%-3d: assign                =\n",lineNo);
            break;
        case Or:
            printf("\t%-3d: or                    |\n",lineNo);
            break;
        case Xor:
            printf("\t%-3d: xor                   ^\n",lineNo);
            break;
        case And:
            printf("\t%-3d: and                   &\n",lineNo);
            break;
        case Lt:
            printf("\t%-3d: less than             <\n",lineNo);
            break;
        case Gt:
            printf("\t%-3d: greater than          >\n",lineNo);
            break;
        case Add:
            printf("\t%-3d: add                   +\n",lineNo);
            break;
        case Sub:
            printf("\t%-3d: sub                   -\n",lineNo);
            break;
        case Mul:
            printf("\t%-3d: mul                   *\n",lineNo);
            break;
        case Div:
            printf("\t%-3d: div                   /\n",lineNo);
            break;
        case Mod:
            printf("\t%-3d: mod                   %%\n",lineNo);
            break;
        case Bracket:
            printf("\t%-3d:                      [\n",lineNo);
            break;
        case '!':
        case ']':
        case '(':
        case ')':
        case '{':
        case '}':
        case ',':
        case ';':
            printf("\t%-3d:                      %c\n",lineNo,(char)token);
            break;
        case Lor:
            printf("\t%-3d: lor                  ||\n",lineNo);
            break;
        case Land:
            printf("\t%-3d: land                 &&\n",lineNo);
            break;
        case Eq:
            printf("\t%-3d: eq                   ==\n",lineNo);
            break;
        case Ne:
            printf("\t%-3d: ne                   !=\n",lineNo);
            break;
        case Le:
            printf("\t%-3d: le                   <=\n",lineNo);
            break;
        case Ge:
            printf("\t%-3d: ge                   >=\n",lineNo);
            break;
        case Shl:
            printf("\t%-3d: shl                  <<\n",lineNo);
            break;
        case Shr:
            printf("\t%-3d: shr                  >>\n",lineNo);
            break;
        case Inc:
            printf("\t%-3d: inc                  ++\n",lineNo);
            break;
        case Dec:
            printf("\t%-3d: dec                  --\n",lineNo);
            break;
        default:
            break;
    }
}

/**
 * @brief 打印抽象语法树
 *
 * 递归调用自身, 遍历 AST 树, 打印节点内容
 *
 * @param   node  抽象语法树节点
 * @param   n     递归深度
 * @return  void
 * */
void printTree(sTreeNode* node, int n) {
    // 遍历非空节点
    while (node != NULL) {
        // 根据节点语句类型, 采取不同操作
        if (node->statementType == Function) {
            // 处理函数节点
            // 处理函数名与函数返回值类型
            printTab(n);
            printf("Function name: %s",node->name);
            printf("\ttype: ");
            printType(node->identifierType);
            printf("\n");
            // 处理参数列表
            printTab(n + 1);
            printf("Parameters:\n");
            printTree(node->children[0], n + 2);
            // 处理函数体
            printTab(n + 1);
            printf("Function Body:\n");
            printTree(node->children[1], n + 2);
        } else if (node->statementType == IfStatement) {
            // 处理 If 语句节点
            printTab(n);
            printf("IF statement:\n");
            // 处理条件表达式
            printTab(n + 1);
            printf("Conditions:\n");
            printTree(node->children[0],n + 2);
            // 处理成功分支语句
            printTab(n + 1);
            printf("Success condition:\n");
            printTree(node->children[1], n + 2);
            // 判断是否有 else 语句
            if (node->children[2] != NULL) {
                // 处理失败分支语句
                printTab(n + 1);
                printf("Failure condition:\n");
                printTree(node->children[2], n + 2);
            }
        } else if (node->statementType == WhileStatement) {
            // 处理 While 语句节点
            printTab(n);
            printf("While statement:\n");
            // 处理条件表达式
            printTab(n + 1);
            printf("Conditions:\n");
            printTree(node->children[0],n + 2);
            // 处理循环体
            printTab(n + 1);
            printf("While Body:\n");
            printTree(node->children[1], n + 2);
        } else if (node->statementType == ForStatement) {
            // 处理 For 语句节点
            printTab(n);
            printf("For statement:\n");
            // 处理初始化表达式
            printTab(n + 1);
            printf("Initiation:\n");
            printTree(node->children[0],n + 2);
            // 处理条件表达式
            printTab(n + 1);
            printf("Conditions:\n");
            printTree(node->children[1],n + 2);
            // 处理更新条件
            printTab(n + 1);
            printf("While Body:\n");
            printTree(node->children[2], n + 2);
            // 处理循环体
            printTab(n + 1);
            printf("While Body:\n");
            printTree(node->children[3], n + 2);
        } else if (node->statementType == DoWhileStatement) {
            // 处理 Do While 语句节点
            printTab(n);
            printf("Do While statement:\n");
            // 处理条件表达式
            printTab(n + 1);
            printf("Conditions:\n");
            printTree(node->children[0],n + 2);
            // 处理循环体
            printTab(n + 1);
            printf("Do While Body:\n");
            printTree(node->children[1], n + 2);
        } else if (node->statementType == ExpressStatement) {
            // 处理表达式语句节点
            printTab(n);
            if (node->expressionType == Operator) {
                // 打印运算符
                printOperator(node->operatorType);
                printf("\n");
            } else if (node->expressionType == Constant) {
                // 打印 Int, Char, String 常量
                if (node->identifierType == Char) {
                    printf("Char:   '%c'\n",(char)node->value);
                } else if (node->identifierType == Int) {
                    printf("Num:     %lld\n",node->value);
                } else {
                    printf("String:  %s",(char*)node->value);
                }
            } else if (node->expressionType == Variable) {
                // 打印变量
                printf("Id:%s\n",node->name);
            } else if (node->expressionType == Call) {
                // 打印函数调用
                printf("Function call:%s\n",node->name);
            }
            // 打印左侧子运算符
            printTree(node->children[0], n + 1);
            // 打印右侧子运算符
            printTree(node->children[1], n + 1);
        } else if (node->statementType == ReturnStatement) {
            // 处理返回语句节点
            printTab(n);
            printf("Return statement:\n");
            // 处理返回值
            if (node->children[0] != NULL) {
                printTab(n + 1);
                printf("Return value:\n");
                printTree(node->children[0], n + 2);
            }
        } else if (node->statementType == ParameterStatement) {
            // 处理参数节点
            // 根据节点标识符类型打印不同信息
            if (node->identifierType == Void) {
                // 处理空参数列表
                printTab(n);
                printType(node->identifierType);
                printf("\n");
            } else {
                // 处理非空参数列表
                printTab(n);
                printf("%s(",node->name);
                printType(node->identifierType);
                printf(")\n");
            }
        } else if (node->statementType == DeclareStatement) {
            // 处理声明语句节点
            printTab(n);
            printf("Declare statement:\n");
            // 打印声明变量的类型
            printTab(n + 1);
            printf("Type:");
            printType(node->identifierType);
            // 打印声明变量的名称
            if (node->identifierType == INT || node->identifierType == CHAR) {
                printf("\t\t\tId name: %s\n",node->name);
            } else {
                printf("\t\t\tId name: %s\t\t\tSize: %lld\n",node->name,node->size);
            }
        }
        // 处理兄弟结点
        node = node->sibling;
    }
}

/**
 * @brief 打印制表符
 *
 * 根据传入参数 n, 打印 n 个制表符与分割线
 *
 * @param n 制表符个数
 * @return void
 * */
static void printTab(int n) {
    for (int i = 0; i < n; i++) {
        printf("\t|");
    }
}

/**
 * @brief 打印类型
 *
 * 打印类型: Int, Char, Void, pointer
 *
 * @param   type    类型
 * @return  void
 * */
static void printType(int type) {
    // 判断类型, 打印相关信息
    if (type == Int) {
        printf("INT");
    } else if (type == Char) {
        printf("CHAR");
    } else if (type == Void) {
        printf("VOID");
    } else if (type > Ptr) {
        // 判断是 Int 指针 还是 Char 指针
        if (type % Ptr == 1) {
            printf("INT ");
        } else {
            printf("CHAR");
        }
        // 打印指针
        for (int i = type / Ptr; i > 0; i--) {
            printf(" PTR");
        }
    } else {
        // 打印错误信息
    }
}

/**
 * @brief   打印运算符
 *
 * 打印运算符类型
 *
 * @param   op  运算符
 * @return  void
 * */
static void printOperator(int op) {
    switch (op) {
        case Assign:
            printf("assign(=)");
            break;
        case Or:
            printf("or(|)");
            break;
        case Xor:
            printf("xor(^)");
            break;
        case And:
            printf("and(&)");
            break;
        case Lt:
            printf("less than(<)");
            break;
        case Gt:
            printf("greater than(>)");
            break;
        case Add:
            printf("add(+)");
            break;
        case Sub:
            printf("sub(-)");
            break;
        case Mul:
            printf("mul(*)");
            break;
        case Div:
            printf("div(/)");
            break;
        case Mod:
            printf("mod(%%)");
            break;
        case Bracket:
            printf("bracket([])");
            break;
        case '!':
            printf("Not(!)");
            break;
        case ']':
        case '(':
        case ')':
        case '{':
        case '}':
        case ',':
        case ';':
            printf("%c",op);
            break;
        case Lor:
            printf("lor(||)");
            break;
        case Land:
            printf("land(&&)");
            break;
        case Eq:
            printf("equal(==)");
            break;
        case Ne:
            printf("not equal(!=)");
            break;
        case Le:
            printf("le(<=)");
            break;
        case Ge:
            printf("ge(>=)");
            break;
        case Shl:
            printf("shl(<<)");
            break;
        case Shr:
            printf("shr(>>)");
            break;
        case Inc:
            printf("inc(++)");
            break;
        case Dec:
            printf("dec(--)");
            break;
        default:
            break;
    }
}

/**
 * @brief 打印汇编结果
 *
 * 打印汇编语言代码
 *
 * @param   void
 * @return  void
 * */
void printAssemble() {
    char* instructions [34] = {
        "IMM", "LEA", "JMP", "JZ", "JNZ", "CALL", "NVAR", "DARG",
        "RET", "LA", "LI", "LC", "SA", "SI", "SC", "PUSH", "OR", "XOR",
        "AND", "EQ", "NE", "LT", "GT", "LE", "GE", "SHL",
        "SHR", "ADD", "SUB", "MUL", "DIV", "MOD", "PRINTF", "EXIT"
    };                              // 指令集
    long long* dump = codeDump;     // 备份代码备份指针
    // 遍历代码段指针, 打印相关信息
    while (codeDump < code) {
        fprintf(stdout, "(%lld)  %.5s", (long long)codeDump, instructions[*codeDump]);
        if (*codeDump < RET) {
            ++codeDump;
            fprintf(stdout, "\t%lld\n", *codeDump);
        } else {
            fprintf(stdout, "\n");
        }
        codeDump++;
    }
    // 恢复代码段备份指针
    codeDump = dump;
}

/**
 * @brief 打印错误信息
 *
 * 打印错误信息到控制台
 *
 * @param   error   错误信息
 * @param   message 具体内容
 * @return  void
 * */
void printErrorInformation(char* error, const char* message) {
    if (message == NULL) {
        fprintf(stderr,"line %d: %s !\n",line, error);
    } else {
        fprintf(stderr,"line %d: %s: %s !\n",line, error, message);
    }
}