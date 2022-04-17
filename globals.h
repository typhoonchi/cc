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
#define BUFFERSIZE 256

//指令集枚举
// instruction set: copy from c4, change JSR/ENT/ADJ/LEV/BZ/BNZ to CALL/NVAR/DARG/RET/JZ/JNZ.
enum {IMM, LEA, JMP, JZ, JNZ, CALL, NVAR, DARG, RET, LI, LC, SI, SC, PUSH,
    OR, XOR, AND, EQ, NE, LT, GT, LE, GE, SHL, SHR, ADD, SUB, MUL, DIV, MOD,
    OPEN, READ, CLOS, PRTF, MALC, FREE, MSET, MCMP, EXIT};

//关键字与运算符
enum {Num = 128, Fun, Sys, Glo, Loc, Id, String, Char, Error,
    CHAR, INT, IF, ELSE, RETURN, WHILE, VOID,
    // operators in precedence order.
    Assign, Or, Xor, And, Not, Lt, Gt,
    Add, Sub, Mul, Div, Mod, Lbracket, Rbracket,
    Lbrace, Rbrace, Lparenthesis, Rparenthesis, Comma, Semicolon,
    Lor, Land, Eq, Ne, Le, Ge, Shl, Shr, Inc, Dec
};

//符号表
// fields of symbol_table: copy from c4, rename HXX to GXX
enum {Token, Hash, Name, Class, Type, Value, GlobalClass, GlobalType, GlobalValue, SymbolTableSize};

struct symbol{
    int token;
    long long hash;
    char* name;
    long long value;
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

#endif //_GLOBALS_H
