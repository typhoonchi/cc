//
// Created by zhangyukun on 2022/4/11.
//

#ifndef _GLOBALS_H
#define _GLOBALS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define MAXSIZE 1024 * 1024
#define STACKSIZE 128
#define BUFFERSIZE 256
#define MAXCHILDREN 3

//指令集枚举
// instruction set: copy from c4, change JSR/ENT/ADJ/LEV/BZ/BNZ to CALL/NVAR/DARG/RET/JZ/JNZ.
enum {IMM, LEA, JMP, JZ, JNZ, CALL, NVAR, DARG, RET, LI, LC, SI, SC, PUSH,
    OR, XOR, AND, EQ, NE, LT, GT, LE, GE, SHL, SHR, ADD, SUB, MUL, DIV, MOD,
    OPEN, READ, CLOS, PRTF, MALC, FREE, MSET, MCMP, EXIT};

//关键字与运算符
enum {
    Num = 128, Fun, Sys, Glo, Loc, Id, String, Char, INTARRAY, CHARARRAY, Error,
    CHAR, INT, IF, ELSE, RETURN, WHILE, VOID,
    Assign, Lor, Land, Or, Xor, And, Eq, Ne,
    Lt, Gt, Le, Ge, Shl, Shr,
    Add, Sub, Mul, Div, Mod, Not, Inc, Dec, Bracket
};

enum {
    Operator = 1, Constant, Identifier, Call
};

// 节点类型
enum {
    IfStatement = 1, WhileStatement, ExpressStatement, ReturnStatement, DeclareStatement, Function, ParameterStatement
};

struct symbol{
    int token;
    long long hash;
    char* name;
    int class;
    int type;
    long long value;
    int gClass;
    int gType;
    long long gValue;

};

struct treeNode{
    struct treeNode* children[MAXCHILDREN];
    struct treeNode* sibling;
    int lineNo;     // 行号
    int nodeType;   // 节点类型
    int type;       // 函数返回值 或 变量类型
    long long size; // 数组大小
    bool isArray;   // 是否是数组
    int valType;
    int opType;     // 运算符类型
    long long value;// 常量值
    char* name;     // 变量名
};

extern char* source;
extern char* sourceDump;
extern char* tokenString;
extern FILE* filePtr;
extern long long token, tokenValue;
extern int line;
// symbol table & pointer
extern struct symbol * symbolTable, * symbolPtr;
extern int scanTrace;
extern int sourceTrace;
extern char* sourcePtr;
extern struct treeNode* root;
extern struct treeNode** symbolStack;
extern int top;
#endif //_GLOBALS_H
