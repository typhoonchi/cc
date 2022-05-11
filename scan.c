//
// Created by zhangyukun on 2022/4/11.
//
#include "globals.h"
#include "scan.h"
#include "utility.h"

/* 词法分析
 *
 * */
void getToken(void){
    int index = 0;
    char* chPtr = NULL;
    tokenString = malloc(BUFFERSIZE * sizeof(char));
    if (tokenString == NULL) {
        printErrorInformation("Fail to create tokenString", NULL);
        exit(1);
    }
    memset(tokenString, 0, BUFFERSIZE * sizeof(char));

    // 取字符
    while ((token = (unsigned char)*source) != '\0') {
        // 取字符
        // 缓冲区指针移动, 向前看一个
        source++;
        // DFA
        if (token == '\n') {
            // 跳过换行符
            line++;
            if (scanTrace && sourceTrace) {
                printSource(line);
            }
        } else if (token == '#') {
            // 跳过预处理和宏定义等操作
            while ((*source != '\0') && (*source != '\n')) {
                source++;
            }
        } else if (token == ' ' || token == '\t') {
            // do nothing
        } else if (isalpha((int)token) || token == '_') {
            // 处理标识符 ID
            // 定位到标识符开始位置
            chPtr = source - 1;
            // 记录到 tokenString 中, 供后续使用
            tokenString[index++] = *chPtr;
            while (isalpha(*source) || isnumber(*source) || (*source == '_')) {
                tokenString[index++] = *source;
                // 计算 hash 值, 加速符号表查找
                token = token * 147 + (*source);
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
                    break;
                }
                symbolPtr++;
            }
            if (symbolPtr->token == 0) {
                // 该标识符不存在, 向符号表中插入该标识符的名称, hash 值, token 类型
                symbolPtr->name = tokenString;
                symbolPtr->hash = token;
                token = symbolPtr->token = Id;
            }
            break;
        } else if (isnumber((int)token)) {
            // 处理数字
            tokenValue = token - '0';
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
                } else if (*source >= '0' && *source <= '7') {
                    // 处理八进制数字
                    while (*source >= '0' && *source <= '7') {
                        tokenValue = tokenValue * 8 + (*source - '0');
                        source++;
                    }
                } else {
                    // 单独处理 0
                    tokenValue = token - '0';
                }
            }
            // 插入符号表
            token = Num;
            break;
        } else if (token == '"' || token == '\'') {
            // 处理字符串和字符
            while ((*source != 0) && (*source != token)) {
                tokenValue = (long long)*source;
                source++;
                if (tokenValue == '\\') {
                    // 处理换行符
                    tokenValue = (long long)*source;
                    source++;
                    if (tokenValue == 'n') {
                        tokenValue = '\n';
                    }
                }
                // 处理字符串
                if (token == '"') {
                    tokenString[index++] = (char)tokenValue;
                }
            }
            source++;
            if (token == '"') {
                // 记录到符号表
                token = String;
            } else {
                // 记录到符号表
                token = Char;
            }
            break;
        } else if (token == '/') {
            // 处理除法, 单行注释和多行注释
            if (*source == '/') {
                // 处理单行注释
                source++;
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
                        if (scanTrace && sourceTrace) {
                            printSource(line);
                        }
                    }
                    source++;
                }
                // 始终向前看一个字符
                source += 2;
            } else {
                // 处理除法并记录
                token = Div;
                break;
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
            break;
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
            break;
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
            break;
        } else if (token == '*') {
            // 处理乘法
            token = Mul;
            break;
        } else if (token == '%') {
            // 处理取模
            token = Mod;
            break;
        } else if (token == '!') {
            // 处理非运算与不等于
            if (*source == '=') {
                // 处理不等于
                source++;
                token = Ne;
            } else {
                // 处理非运算
                break;
            }
            break;
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
            break;
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
            break;
        } else if (token == '^') {
            // 处理异或
            token = Xor;
            break;
        } else if (token == '<') {
            // 处理小于等于, 小于, 左移
            if (*source == '=') {
                // 处理小于等于
                source++;
                token = Le;
            } else if (*source == '<'){
                // 处理左移
                source++;
                token = Shl;
            } else {
                // 处理小于
                token = Lt;
            }
            break;
        } else if (token == '>') {
            // 处理大于等于, 大于, 右移
            if (*source == '=') {
                // 处理大于等于
                source++;
                token = Ge;
            } else if (*source == '>'){
                // 处理右移
                source++;
                token = Shr;
            } else {
                // 处理大于
                token = Gt;
            }
            break;
        } else if (token == '[') {
            // 处理下标
            token = Bracket;
            break;
        } else if (token == ']' || token == '(' || token == ')' || token == '{' || token == '}' || token == ';' || token == ',') {
            // 处理其余符号
            break;
        } else {
            printErrorInformation("Get Unknown token",NULL);
            exit(1);
        }
    }
    if (scanTrace) {
        printToken(line);
    }
}

/* 初始化关键字信息
 *
 * */
void initKeywords(void){
    // 记录标志位信息
    int record;
    // 关键字信息
    char* keywords[7] = {
            "char", "int", "if", "else", "return", "while", "void"
    };
    record = scanTrace;
    scanTrace = 0;
    // 扫描关键字
    for (int i = 0; i < 7; i++) {
        source = keywords[i];
        getToken();
        symbolPtr->token = CHAR + i;
    }
    // 恢复标志位与相关信息
    source = sourceDump;
    scanTrace = record;
}