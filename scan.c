//
// Created by zhangyukun on 2022/4/11.
//
#include "globals.h"
#include "scan.h"
#include "utility.h"

/* 词法分析 */
void getToken(void){
    int index = 0;
    tokenString = malloc(BUFFERSIZE * sizeof(char));
    memset(tokenString, 0, BUFFERSIZE * sizeof(char));
    char* chPtr = NULL;

    // 定位到空符号表
/*    while (symbolPtr->token != 0) {
        symbolPtr++;
    }*/
    // 取字符
    while ((token = (unsigned char)*source) != '\0') {
        // 取字符
//        token = (unsigned char)*source;
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
//            token = symbolPtr->token = Num;
//            symbolPtr->value = tokenValue;
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
//                token = symbolPtr->token = String;
//                symbolPtr->name = tokenString;
            } else {
                // 记录到符号表
                token = Char;
//                token = symbolPtr->token = Char;
//                symbolPtr->value = tokenValue;
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
//                tokenValue = symbolPtr->value = token;
//                token = symbolPtr->token = Div;
                token = Div;
                break;
            }
        } else if (token == '=') {
            // 处理赋值与等于
            if (*source == '=') {
                // 处理等于
                source++;
//                token = symbolPtr->token = Eq;
//                symbolPtr->name = "==";
                token = Eq;
            } else {
                // 处理赋值
//                tokenValue = symbolPtr->value = token;
//                token = symbolPtr->token = Assign;
                token = Assign;
            }
            break;
        } else if (token == '+') {
            // 处理加法与自增
            if (*source == '+') {
                // 处理自增
                source++;
//                token = symbolPtr->token = Inc;
//                symbolPtr->name = "++";
                token = Inc;
            } else {
                // 处理加法
//                tokenValue = symbolPtr->value = token;
//                token = symbolPtr->token = Add;
                token = Add;
            }
            break;
        } else if (token == '-') {
            // 处理减法与自减
            if (*source == '-') {
                // 处理自减
                source++;
//                symbolPtr->token = Dec;
//                symbolPtr->name = "--";
                token = Dec;
            } else {
                // 处理减法
//                symbolPtr->token = Sub;
//                symbolPtr->value = token;
                token = Sub;
            }
            break;
        } else if (token == '*') {
            // 处理乘法
//            symbolPtr->token = Mul;
//            symbolPtr->value = token;
            token = Mul;
            break;
        } else if (token == '%') {
            // 处理取模
//            symbolPtr->token = Mod;
//            symbolPtr->value = token;
            token = Mod;
            break;
        } else if (token == '!') {
            // 处理非运算与不等于
            if (*source == '=') {
                // 处理不等于
                source++;
//                symbolPtr->token = Ne;
//                symbolPtr->name = "!=";
                token = Ne;
            } else {
                // 处理非运算
//                symbolPtr->token = Not;
//                symbolPtr->value = token;
                break;
            }
            break;
        } else if (token == '&') {
            // 处理按位与与逻辑与
            if (*source == '&') {
                // 处理逻辑与
                source++;
//                symbolPtr->token = Land;
//                symbolPtr->name = "&&";
                token = Land;
            } else {
                // 处理按位与
//                symbolPtr->token = And;
//                symbolPtr->value = token;
                token = And;
            }
            break;
        } else if (token == '|') {
            // 处理按位或与逻辑或
            if (*source == '|') {
                // 处理逻辑或
                source++;
//                symbolPtr->token = Lor;
//                symbolPtr->name = "||";
                token = Lor;
            } else {
                // 处理按位或
//                symbolPtr->token = Or;
//                symbolPtr->value = token;
                token = Or;
            }
            break;
        } else if (token == '^') {
            // 处理异或
//            symbolPtr->token = Xor;
//            symbolPtr->value = token;
            token = Xor;
            break;
        } else if (token == '<') {
            // 处理小于等于, 小于, 左移
            if (*source == '=') {
                // 处理小于等于
                source++;
//                symbolPtr->token = Le;
//                symbolPtr->name = "<=";
                token = Le;
            } else if (*source == '<'){
                // 处理左移
                source++;
//                symbolPtr->token = Shl;
//                symbolPtr->name = "<<";
                token = Shl;
            } else {
                // 处理小于
//                symbolPtr->token = Lt;
//                symbolPtr->value = token;
                token = Lt;
            }
            break;
        } else if (token == '>') {
            // 处理大于等于, 大于, 右移
            if (*source == '=') {
                // 处理大于等于
                source++;
//                symbolPtr->token = Ge;
//                symbolPtr->name = ">=";
                token = Ge;
            } else if (*source == '>'){
                // 处理右移
                source++;
//                symbolPtr->token = Shr;
//                symbolPtr->name = ">>";
                token = Shr;
            } else {
                // 处理大于
//                symbolPtr->token = Gt;
//                symbolPtr->value = token;
                token = Gt;
            }
            break;
        } else if (token == '[') {
            token = Bracket;
            break;
        } else if (token == ']' || token == '(' || token == ')' || token == '{' || token == '}' || token == ';' || token == ',') {
//            symbolPtr->token = Lbracket;
//            symbolPtr->value = token;
//            token = Lbracket;
            break;
        } else {
            exit(1);
        }
    }
    if (scanTrace) {
        printToken(line);
    }
}

void initKeywords(void){
    int record;
    char* keywords[7] = {
            "char", "int", "if", "else", "return", "while", "void"
    };
    record = scanTrace;
    scanTrace = 0;
    for (int i = 0; i < 7; i++) {
        source = keywords[i];
        getToken();
        symbolPtr->token = CHAR + i;
    }
    source = sourceDump;
    scanTrace = record;
}