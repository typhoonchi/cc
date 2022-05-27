//
// Created by zhangyukun on 2022/4/11.
//

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "globals.h"
#include "scanner.h"
#include "utility.h"

static char *tokenString;                  // token 名称缓冲区

static void createTokenString();
static void scan();

/**
 * @brief 词法分析, 并打印词素.
 *
 * 扫描源代码缓冲区, 输出词素, 并打印词素.
 *
 * @param   void
 * @return  void
 * */
void getToken(void) {
    // 词法分析.
    scan();
    // 打印词素信息.
    if (scanTrace) {
        printToken(line);
    }
}

/**
 * @brief 初始化关键字信息.
 *
 * 初始化关键字与系统调用函数 printf.
 *
 * @param   void
 * @return  void
 * */
void initKeywords(void) {
    int record;         // 记录标志位信息.
    char* keywords[10] = {
            "int", "char", "void", "if", "else",
            "for", "while", "do", "return", "printf"
    };                  // 关键字信息.

    // 记录打印词素标志位信息.
    record = scanTrace;
    // 打印词素标志位置零.
    scanTrace = 0;
    // 扫描关键字.
    for (int i = 0; i < 9; i++) {
        sourcePtr = keywords[i];
        getToken();
        symbolPtr->token = INT + i;
    }
    // 处理系统调用函数.
    sourcePtr = keywords[9];
    getToken();
    symbolPtr->class = Sys;
    symbolPtr->type = Int;
    symbolPtr->address = PRINTF;
    // 恢复标志位与相关信息.
    sourcePtr = source;
    scanTrace = record;
}

/**
 * @brief 分配词素名称存储区.
 *
 * 为 tokenString 分配内存空间, 并初始化.
 *
 * @param   void
 * @return  void
 * */
static void createTokenString() {
    // 分配词素名称存储区.
    tokenString = malloc(BUFFER_SIZE * sizeof(char));
    // 判断存储区是否创建成功.
    if (NULL == tokenString) {
        // 创建失败, 打印错误信息.
        handleErrorInformation(line, SCAN_ERROR, "scanner.c/createTokenString()",
                               "Could Not Malloc Space for tokenString", NULL);
    }
    // 初始化存储区.
    memset(tokenString, 0, BUFFER_SIZE * sizeof(char));
}

/**
 * @brief 词法分析.
 *
 * 扫描源代码缓冲区, 输出词素.
 *
 * @param   void
 * @return  void
 * */
static void scan() {
    int index = 0;                  // 词素名称索引.
    char* chPtr = NULL;             // 字符指针.
    long long* base = NULL;         // 数据段指针, 存储字符串常量时用于定位数据段初始位置.

    // 从源代码缓冲区中取字符.
    while ((token = (unsigned char)*sourcePtr) != '\0') {
        // 源代码缓冲区指针移动, 向前看一个.
        sourcePtr++;
        // 根据 DFA 处理词素.
        if ('\n' == token) {
            // 跳过换行符.
            line++;
        } else if ('#' == token) {
            // 跳过预处理和宏定义等操作.
            while ((*sourcePtr != '\0') && (*sourcePtr != '\n')) {
                sourcePtr++;
            }
        } else if ((' ' == token) || ('\t' == token)) {
            // 跳过.
        } else if (isalpha((int)token) || ('_' == token)) {
            // 处理标识符 ID.
            // 分配词素名称存储区.
            createTokenString();
            // 定位到标识符开始位置.
            chPtr = sourcePtr - 1;
            // 记录到 tokenString 中, 供后续使用.
            tokenString[index++] = *chPtr;
            while (isalpha(*sourcePtr) || isnumber(*sourcePtr) || ('_' == *sourcePtr)) {
                tokenString[index] = *sourcePtr;
                index++;
                // 计算 hash 值, 加速符号表查找.
                token = token * 147 + (*sourcePtr);
                // 源代码指针后移.
                sourcePtr++;
            }
            // 计算 hash 值.
            token = (token << 6) + (sourcePtr - chPtr);
            // 查找符号表, 确认该标识符是否已经存在.
            symbolPtr = symbolTable;
            while (symbolPtr->token != 0) {
                if ((symbolPtr->hash == token) && !memcmp((char*)symbolPtr->name, chPtr, sourcePtr - chPtr)) {
                    // 该标识符已存在, 直接返回其 token 类型.
                    token = symbolPtr->token;
                    tokenValue = (long long)symbolPtr->name;
                    free(tokenString);
                    return;
                }
                symbolPtr++;
            }
            // 该标识符不存在, 向符号表中插入该标识符的名称, hash 值, token 类型.
            symbolPtr->name = tokenString;
            symbolPtr->hash = token;
            token = symbolPtr->token = Id;
            tokenValue = (long long)symbolPtr->name;
            return;
        } else if (isnumber((int)token)) {
            // 处理数字.
            tokenValue = token - '0';
            // 判断数字是 Dec, Oct 还是 Hex.
            if (token != '0') {
                // 处理十进制非零整数.
                while (isnumber(*sourcePtr)) {
                    tokenValue = tokenValue * 10 + (*sourcePtr - '0');
                    sourcePtr++;
                }
            } else {
                if (('x' == *sourcePtr) || ('X' == *sourcePtr)) {
                    // 处理十六进制数字.
                    sourcePtr++;
                    while (isnumber(*sourcePtr) || (*sourcePtr >= 'a' && *sourcePtr <= 'f') || (*sourcePtr >= 'A' && *sourcePtr <= 'F')) {
                        token = (unsigned char)*sourcePtr;
                        sourcePtr++;
                        /*很神奇, 利用了 ASCII 码,
                         * 0 - 9 的 ASCII 码是 0x30 - 0x39
                         * A - F 的 ASCII 码是 0x41 - 0x46
                         * a - f 的 ASCII 码是 0x61 - 0x66*/
                        tokenValue = tokenValue * 16 + (token & 0xF) + (token >= 'A' ? 9 : 0);
                    }
                } else if ((*sourcePtr >= '0') && (*sourcePtr <= '7')) {
                    // 处理八进制数字.
                    while ((*sourcePtr >= '0') && (*sourcePtr <= '7')) {
                        tokenValue = tokenValue * 8 + (*sourcePtr - '0');
                        sourcePtr++;
                    }
                } else {
                    // 单独处理 0.
                    tokenValue = token - '0';
                }
            }
            // 记录词素信息.
            token = Num;
            return;
        } else if ('\'' == token) {
            // 处理字符.
            tokenValue = (long long)*sourcePtr;
            sourcePtr++;
            // 判断是否为转义字符.
            if ('\\' == tokenValue) {
                // 处理转义字符.
                tokenValue = (long long)*sourcePtr;
                sourcePtr++;
                if ('n' == tokenValue) {
                    tokenValue = '\n';
                } else if ('t' == tokenValue) {
                    tokenValue = '\t';
                } else if ('0' == tokenValue) {
                    tokenValue = '\0';
                }
            }
            // 判断是否是正常的字符.
            if (*sourcePtr != '\'') {
                // 异常字符, 打印错误信息.
                handleErrorInformation(line, SCAN_ERROR, "scanner.c/scan()",
                                       "Get an Incorrect Character", NULL);
            }
            sourcePtr++;
            // 记录词素信息.
            token = Character;
            return;
        } else if ('"' == token) {
            // 处理字符串.
            // 定位数据段初始位置.
            base = dataPtr;
            // 处理字符串.
            while ((*sourcePtr != 0) && (*sourcePtr != '"')) {
                tokenValue = (long long)*sourcePtr;
                sourcePtr++;
                // 判断是否为转义字符.
                if ('\\' == tokenValue) {
                    // 处理转义字符.
                    tokenValue = (long long)*sourcePtr;
                    sourcePtr++;
                    if ('n' == tokenValue) {
                        tokenValue = '\n';
                    } else if ('t' == tokenValue) {
                        tokenValue = '\t';
                    } else if ('0' == tokenValue) {
                        tokenValue = '\0';
                    }
                }
                // 记录字符串.
                *((unsigned char*)(dataPtr) + index) = (unsigned char)tokenValue;
                index++;
            }
            // 存储 '\0'.
            *((unsigned char*)(dataPtr) + index) = '\0';
            dataPtr += ((index % 8) ? (index / 8 + 1) : (index / 8));
            sourcePtr++;
            // 记录到符号表.
            token = String;
            // 记录字符串存储首地址信息.
            tokenValue = (long long)base;
            return;
        } else if ('/' == token) {
            // 处理除法, 单行注释和多行注释.
            if ('/' == *sourcePtr) {
                // 处理单行注释.
                while ((*sourcePtr != '\0') && (*sourcePtr != '\n')) {
                    sourcePtr++;
                }
            } else if ('*' == *sourcePtr) {
                // 处理多行注释.
                sourcePtr++;
                // 向前看两个字符, 用于判断 "*/".
                while (!(('*' == *sourcePtr) && ('/' == *(sourcePtr + 1)))) {
                    if ('\n' == *sourcePtr) {
                        line++;
                    }
                    sourcePtr++;
                }
                // 始终向前看一个字符.
                sourcePtr += 2;
            } else {
                // 处理除法并记录.
                token = Div;
                return;
            }
        } else if ('=' == token) {
            // 处理赋值与等于.
            if ('=' == *sourcePtr) {
                // 处理等于.
                sourcePtr++;
                token = Eq;
            } else {
                // 处理赋值.
                token = Assign;
            }
            return;
        } else if ('+' == token) {
            // 处理加法与自增.
            if ('+' == *sourcePtr) {
                // 处理自增.
                sourcePtr++;
                token = Inc;
            } else {
                // 处理加法.
                token = Add;
            }
            return;
        } else if ('-' == token) {
            // 处理减法与自减.
            if ('-' == *sourcePtr) {
                // 处理自减.
                sourcePtr++;
                token = Dec;
            } else {
                // 处理减法.
                token = Sub;
            }
            return;
        } else if ('*' == token) {
            // 处理乘法.
            token = Mul;
            return;
        } else if ('%' == token) {
            // 处理取模.
            token = Mod;
            return;
        } else if ('!' == token) {
            // 处理非运算与不等于.
            if ('=' == *sourcePtr) {
                // 处理不等于.
                sourcePtr++;
                token = Ne;
            } else {
                // 处理非运算.
                return;
            }
            return;
        } else if ('&' == token) {
            // 处理按位与与逻辑与.
            if ('&' == *sourcePtr) {
                // 处理逻辑与.
                sourcePtr++;
                token = Land;
            } else {
                // 处理按位与.
                token = And;
            }
            return;
        } else if ('|' == token) {
            // 处理按位或与逻辑或.
            if ('|' == *sourcePtr) {
                // 处理逻辑或.
                sourcePtr++;
                token = Lor;
            } else {
                // 处理按位或.
                token = Or;
            }
            return;
        } else if ('^' == token) {
            // 处理异或.
            token = Xor;
            return;
        } else if ('<' == token) {
            // 处理小于等于, 小于, 左移.
            if ('=' == *sourcePtr) {
                // 处理小于等于.
                sourcePtr++;
                token = Le;
            } else if ('<' == *sourcePtr) {
                // 处理左移.
                sourcePtr++;
                token = Shl;
            } else {
                // 处理小于.
                token = Lt;
            }
            return;
        } else if ('>' == token) {
            // 处理大于等于, 大于, 右移.
            if ('=' == *sourcePtr) {
                // 处理大于等于.
                sourcePtr++;
                token = Ge;
            } else if ('>' == *sourcePtr) {
                // 处理右移.
                sourcePtr++;
                token = Shr;
            } else {
                // 处理大于.
                token = Gt;
            }
            return;
        } else if ('[' == token) {
            // 处理下标.
            token = Bracket;
            return;
        } else if ((']' == token) || ('(' == token) || (')' == token) || ('{' == token) ||
                    ('}' == token) || (';' == token) || (',' == token)) {
            // 处理其余符号.
            return;
        } else {
            // 异常 token, 打印错误信息.
            handleErrorInformation(line, SCAN_ERROR, "scanner.c/scan()",
                                   "Get an Unknown token",NULL);
        }
    }
}