//
// Created by zhangyukun on 2022/4/11.
//

#ifndef GLOBALS_H
#define GLOBALS_H

#include <stdio.h>

#define MAX_SIZE 1024 * 1024
#define STACK_SIZE 128
#define BUFFER_SIZE 256
#define MAX_CHILDREN 4

typedef enum eErrorCode{
    FILE_ERROR = 1, INIT_ERROR, SCAN_ERROR, PARSE_ERROR, CODE_ERROR, RUNNING_ERROR, FUNCTION_ERROR
} eErrorCode;       // 错误代码: 文件错误, 初始化错误, 词法分析错误, 语法分析错误, 代码生成错误, 运行错误, 功能错误

typedef enum eInstructionSet{
    IMM = 1, LEA, JMP, JZ, JNZ, CALL, NVAR, DARG, RET, LA, LI, LC, SA, SI, SC, PUSH,
    OR, XOR, AND, EQ, NE, LT, GT, LE, GE, SHL, SHR, ADD, SUB, MUL, DIV, MOD,
    PRINTF, EXIT
} eInstructionSet;  // 虚拟机指令集

typedef enum eToken{
    Num = 128, Character, String, Id,
    INT, CHAR, VOID, IF, ELSE, FOR, WHILE, DO, RETURN,
    Assign, Lor, Land, Or, Xor, And, Eq, Ne,
    Lt, Gt, Le, Ge, Shl, Shr,
    Add, Sub, Mul, Div, Mod, Inc, Dec, Bracket
} eToken;           //关键字与运算符

typedef enum eClass{
    Fun = 1, Sys, Glo, Loc
} eClass;           // 标识符类别: 函数, 系统调用, 全局变量, 局部变量

typedef enum eType{
    Int = 1, Char, Void, Ptr = 5
} eType;            // 变量或函数返回值类型: Int, Char, Void, Ptr

typedef enum eExpressionType{
    Constant = 1, Variable, Call, Operator
} eExpressionType;  // 表达式中词素的类型: 运算符 , 常量 , 标识符 , 函数调用

typedef enum eStatementType{
    DeclareStatement = 1, Function, ParameterStatement, IfStatement, WhileStatement,
    ForStatement, DoWhileStatement, ExpressStatement, ReturnStatement,
} eStatementType;   // 节点类型: 变量声明语句, 函数定义语句, 参数语句, If 语句, While 语句, For 语句, Do While 语句, 表达式语句, 返回语句

typedef struct symbol{
    eToken token;               // 词素类型
    long long hash;             // 标识符 hash 值
    char* name;                 // 标识符名称
    eClass class;               // 标识符类别 Fun, Sys, Glo, Loc
    eType type;                 // 标识符类型 Int, Char, Ptr
    long long address;          // 标识符地址 变量地址在数据段, 函数地址在代码段
    eClass gloClass;            // 局部变量遮罩全局变量时备份标识符类别
    eType gloType;              // 局部变量遮罩全局变量时备份标识符类型
    long long gloAddress;       // 局部变量遮罩全局变量时备份标识符地址
} sSymbol;      // 符号表结构体

typedef struct treeNode{
    struct treeNode *children[MAX_CHILDREN];    // AST 孩子节点指针
    struct treeNode *sibling;                   // AST 兄弟节点指针
    eStatementType statementType;               // 节点类型
    eType identifierType;                        // 标识符类型
    char* name;                                 // 标识符名
    long long size;                             // 数组大小
    eExpressionType expressionType;             // 表达式词素类型
    eClass classType;                           // 标识符类别
    eToken operatorType;                        // 运算符类型
    long long value;                            // 常量值
} sTreeNode;        // 抽象语法树 AST 结构体

extern FILE *scannerOutputStream, *parserOutputStream, *codeGeneratorOutputStream,
                *codeRunnerOutputStream, *errorOutputStream;
extern char *source, *sourcePtr;
extern long long token, tokenValue;
extern int line;
extern sSymbol *symbolTable, *symbolPtr;
extern int phaseFlag;
extern int scanTrace, parseTrace, generateTrace;
extern sTreeNode *root;
extern long long *code, *codePtr, *mainPtr,
                    *stack, *data, *dataPtr;

#endif // GLOBALS_H
