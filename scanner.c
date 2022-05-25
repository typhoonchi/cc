//
// Created by zhangyukun on 2022/4/11.
//

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "globals.h"
#include "scanner.h"
#include "utility.h"

static void createTokenString();
static void scan();

/**
 * @brief 词法分析, 并打印词素
 *
 * 扫描源代码缓冲区, 输出词素, 并打印词素
 *
 * @param void
 * @return void
 * */
void getToken(void) {
    // 词法分析
    scan();
    // 打印词素信息
    if (scanTrace) {
        printToken(line);
    }
}

/**
 * @brief 初始化关键字信息
 *
 * 初始化关键字与系统调用函数 printf
 *
 * @param void
 * @return void
 * */
void initKeywords(void) {
    int record;         // 记录标志位信息
    char* keywords[10] = {
            "int", "char", "if", "else", "return", "while", "for", "do", "void", "printf"
    };                  // 关键字信息

    // 记录打印词素标志位信息
    record = scanTrace;
    // 打印词素标志位置零
    scanTrace = 0;
    // 扫描关键字
    for (int i = 0; i < 9; i++) {
        source = keywords[i];
        getToken();
        symbolPtr->token = INT + i;
    }
    // 处理系统调用函数
    source = keywords[9];
    getToken();
    symbolPtr->class = Sys;
    symbolPtr->type = Int;
    symbolPtr->address = PRINTF;
    // 恢复标志位与相关信息
    source = sourceDump;
    scanTrace = record;
}

/**
 * @brief 分配词素名称存储区
 *
 * 为 tokenString 分配内存空间, 并初始化
 *
 * @param   void
 * @return  void
 * */
static void createTokenString() {
    // 分配词素名称存储区
    tokenString = malloc(BUFFER_SIZE * sizeof(char));
    // 判断存储区是否创建成功
    if (tokenString == NULL) {
        // 创建失败, 打印错误信息
        printErrorInformation("Fail to create tokenString", NULL);
        exit(1);
    }
    // 初始化存储区
    memset(tokenString, 0, BUFFER_SIZE * sizeof(char));
}

/**
 * @brief 词法分析
 *
 * 扫描源代码缓冲区, 输出词素
 *
 * @param void
 * @return void
 * */
static void scan() {
    int index = 0;                  // 词素名称索引
    char* chPtr = NULL;             // 字符指针
    long long* base = NULL;         // 数据段指针, 存储字符串常量时用于定位数据段初始位置

    // 从源代码缓冲区中取字符
    while ((token = (unsigned char)*source) != '\0') {
        // 源代码缓冲区指针移动, 向前看一个
        source++;
        // 根据 DFA 处理词素
        if (token == '\n') {
            // 跳过换行符
            line++;
        } else if (token == '#') {
            // 跳过预处理和宏定义等操作
            while ((*source != '\0') && (*source != '\n')) {
                source++;
            }
        } else if (token == ' ' || token == '\t') {
            // 跳过
        } else if (isalpha((int)token) || token == '_') {
            // 处理标识符 ID
            // 分配词素名称存储区
            createTokenString();
            // 定位到标识符开始位置
            chPtr = source - 1;
            // 记录到 tokenString 中, 供后续使用
            tokenString[index++] = *chPtr;
            while (isalpha(*source) || isnumber(*source) || (*source == '_')) {
                tokenString[index] = *source;
                index++;
                // 计算 hash 值, 加速符号表查找
                token = token * 147 + (*source);
                // 源代码指针后移
                source++;
            }
            // 计算 hash 值
            token = (token << 6) + (source - chPtr);
            // 查找符号表, 确认该标识符是否已经存在
            symbolPtr = symbolTable;
            while (symbolPtr->token != 0) {
                if ((symbolPtr->hash == token) && !memcmp((char*)symbolPtr->name, chPtr, source - chPtr)) {
                    // 该标识符已存在, 直接返回其 token 类型
                    token = symbolPtr->token;
                    return;
                }
                symbolPtr++;
            }
            // 该标识符不存在, 向符号表中插入该标识符的名称, hash 值, token 类型
            symbolPtr->name = tokenString;
            symbolPtr->hash = token;
            token = symbolPtr->token = Id;
            return;
        } else if (isnumber((int)token)) {
            // 处理数字
            tokenValue = token - '0';
            // 判断数字是 Dec, Oct 还是 Hex
            if (token != '0') {
                // 处理十进制非零整数
                while (isnumber(*source)) {
                    tokenValue = tokenValue * 10 + (*source - '0');
                    source++;
                }
            } else {
                if ((*source == 'x') || (*source == 'X')) {
                    // 处理十六进制数字
                    source++;
                    while (isnumber(*source) || (*source >= 'a' && *source <= 'f') || (*source >= 'A' && *source <= 'F')) {
                        token = (unsigned char)*source;
                        source++;
                        /*很神奇, 利用了 ASCII 码,
                         * 0 - 9 的 ASCII 码是 0x30 - 0x39
                         * A - F 的 ASCII 码是 0x41 - 0x46
                         * a - f 的 ASCII 码是 0x61 - 0x66*/
                        tokenValue = tokenValue * 16 + (token & 0xF) + (token >= 'A' ? 9 : 0);
                    }
                } else if ((*source >= '0') && (*source <= '7')) {
                    // 处理八进制数字
                    while ((*source >= '0') && (*source <= '7')) {
                        tokenValue = tokenValue * 8 + (*source - '0');
                        source++;
                    }
                } else {
                    // 单独处理 0
                    tokenValue = token - '0';
                }
            }
            // 记录词素信息
            token = Num;
            return;
        } else if (token == '\'') {
            // 处理字符
            tokenValue = (long long)*source;
            source++;
            // 判断是否为转义字符
            if (tokenValue == '\\') {
                // 处理转义字符
                tokenValue = (long long)*source;
                source++;
                if (tokenValue == 'n') {
                    tokenValue = '\n';
                } else if (tokenValue == 't') {
                    tokenValue = '\t';
                } else if (tokenValue == '0') {
                    tokenValue = '\0';
                }
            }
            // 判断是否是正常的字符
            if (*source != '\'') {
                // 异常字符, 打印错误信息
                exit(1);
            }
            source++;
            // 记录词素信息
            token = Char;
            return;
        } else if (token == '"') {
            // 处理字符串
            // 分配词素名称存储区
            createTokenString();
            // 定位数据段初始位置
            base = data;
            // 处理字符串
            while ((*source != 0) && (*source != '"')) {
                tokenValue = (long long)*source;
                source++;
                // 判断是否为转义字符
                if (tokenValue == '\\') {
                    // 处理转义字符
                    tokenValue = (long long)*source;
                    source++;
                    if (tokenValue == 'n') {
                        tokenValue = '\n';
                    } else if (tokenValue == 't') {
                        tokenValue = '\t';
                    } else if (tokenValue == '0') {
                        tokenValue = '\0';
                    }
                }
                // 记录字符串
                tokenString[index] = (char)tokenValue;
                *((unsigned char*)(data) + index) = (unsigned char)tokenValue;
                index++;
            }
            // 存储 '\0'
            *((unsigned char*)(data) + index) = '\0';
            data += ((index % 8) ? (index / 8 + 1) : (index / 8));
            source++;
            // 记录到符号表
            token = Char + Ptr;
            // 记录字符串存储首地址信息
            tokenValue = (long long)base;
            return;
        } else if (token == '/') {
            // 处理除法, 单行注释和多行注释
            if (*source == '/') {
                // 处理单行注释
                while ((*source != '\0') && (*source != '\n')) {
                    source++;
                }
            } else if (*source == '*') {
                // 处理多行注释
                source++;
                // 向前看两个字符, 用于判断 "*/"
                while (!((*source == '*') && (*(source + 1) == '/'))) {
                    if (*source == '\n') {
                        line++;
                    }
                    source++;
                }
                // 始终向前看一个字符
                source += 2;
            } else {
                // 处理除法并记录
                token = Div;
                return;
            }
        } else if (token == '=') {
            // 处理赋值与等于
            if (*source == '=') {
                // 处理等于
                source++;
                token = Eq;
            } else {
                // 处理赋值
                token = Assign;
            }
            return;
        } else if (token == '+') {
            // 处理加法与自增
            if (*source == '+') {
                // 处理自增
                source++;
                token = Inc;
            } else {
                // 处理加法
                token = Add;
            }
            return;
        } else if (token == '-') {
            // 处理减法与自减
            if (*source == '-') {
                // 处理自减
                source++;
                token = Dec;
            } else {
                // 处理减法
                token = Sub;
            }
            return;
        } else if (token == '*') {
            // 处理乘法
            token = Mul;
            return;
        } else if (token == '%') {
            // 处理取模
            token = Mod;
            return;
        } else if (token == '!') {
            // 处理非运算与不等于
            if (*source == '=') {
                // 处理不等于
                source++;
                token = Ne;
            } else {
                // 处理非运算
                return;
            }
            return;
        } else if (token == '&') {
            // 处理按位与与逻辑与
            if (*source == '&') {
                // 处理逻辑与
                source++;
                token = Land;
            } else {
                // 处理按位与
                token = And;
            }
            return;
        } else if (token == '|') {
            // 处理按位或与逻辑或
            if (*source == '|') {
                // 处理逻辑或
                source++;
                token = Lor;
            } else {
                // 处理按位或
                token = Or;
            }
            return;
        } else if (token == '^') {
            // 处理异或
            token = Xor;
            return;
        } else if (token == '<') {
            // 处理小于等于, 小于, 左移
            if (*source == '=') {
                // 处理小于等于
                source++;
                token = Le;
            } else if (*source == '<') {
                // 处理左移
                source++;
                token = Shl;
            } else {
                // 处理小于
                token = Lt;
            }
            return;
        } else if (token == '>') {
            // 处理大于等于, 大于, 右移
            if (*source == '=') {
                // 处理大于等于
                source++;
                token = Ge;
            } else if (*source == '>') {
                // 处理右移
                source++;
                token = Shr;
            } else {
                // 处理大于
                token = Gt;
            }
            return;
        } else if (token == '[') {
            // 处理下标
            token = Bracket;
            return;
        } else if (token == ']' || token == '(' || token == ')' || token == '{' || token == '}' || token == ';' || token == ',') {
            // 处理其余符号
            return;
        } else {
            printErrorInformation("Get Unknown token",NULL);
            exit(1);
        }
    }
}