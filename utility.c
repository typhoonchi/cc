//
// Created by zhangyukun on 2022/4/11.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "globals.h"
#include "utility.h"

static void loadSrc(const char *fileName);
static int getBaseFileNameLen(const char *fileName);
static void initFileLoader(const char *fileName);
static void initScanner(const char *fileName, int baseLen);
static void initParser(const char *fileName, int baseLen);
static void initCodeGenerator(const char *fileName, int baseLen);
static void initCodeRunner(const char *fileName, int baseLen);
static void printType(int type);
static void printOperator(int op);
static void printTab(int n);
static void printNode(sTreeNode *node, int n);
static void destroyNode(sTreeNode *node);

/**
 * @brief 初始化编译器.
 *
 * 根据选择模式初始化输出流, 为编译器中用到的源代码存储区, 符号表, 节点栈, 代码段, 数据段, 堆栈段的分配存储区;
 * 并初始化存储区空间与相关指针.
 *
 * @param   void
 * @return  void
 * */
void init(const char *fileName) {
    int len = 0;        // 文件名消除扩展名后长度

    // 初始化错误信息输出流.
    errorOutputStream = stderr;
    // 加载源代码文件.
    initFileLoader(fileName);
    // 获取文件名长度
    len = getBaseFileNameLen(fileName);
    // 判断执行模式
    if ((phaseFlag >= 8) && (phaseFlag < 0xC)) {
        // 只进行词法分析初始化.
        initScanner(fileName, len);
    } else if (phaseFlag >= 0xC) {
        // 进行语法分析初始化.
        initScanner(fileName, len);
        initParser(fileName, len);
        if (phaseFlag >= 0xE) {
            // 进行代码生成初始化.
            initCodeGenerator(fileName, len);
            if (0xF == phaseFlag) {
                // 进行代码运行初始化.
                initCodeRunner(fileName, len);
            }
        }
    }
}

/**
 * @brief 回收动态分配空间.
 *
 * 关闭输出流, 回收编译器中用到的源代码存储区, 符号表, 节点栈, 代码段, 数据段, 堆栈段的空间;
 * 并将相关指针置空.
 *
 * @param   void
 * @return  void
 * */
void destroy() {
    if (scannerOutputStream != stdout) {
        fclose(scannerOutputStream);
    }
    if (parserOutputStream != stdout) {
        fclose(parserOutputStream);
    }
    if (codeGeneratorOutputStream != stdout) {
        fclose(codeGeneratorOutputStream);
    }
    if (codeRunnerOutputStream != stdout) {
        fclose(codeRunnerOutputStream);
    }
    // 释放源代码空间.
    free(source);
    source = sourcePtr = NULL;
    // 释放代码段空间.
    free(code);
    code = codePtr = NULL;
    // 释放数据段空间.
    free(data);
    data = NULL;
    // 释放堆栈段空间.
    free(stack);
    stack = NULL;
    // 释放 AST 空间.
    destroyNode(root);
    // 判断符号表是否为空.
    if (NULL != symbolTable) {
        // 释放非空符号表标识符指向空间.
        symbolPtr = symbolTable;
        // 遍历非空符号表, 释放标识符空间.
        while (symbolPtr->token != 0) {
            if (symbolPtr->name != NULL) {
                // 释放符号表中标识符名空间.
                free(symbolTable->name);
                symbolTable->name = NULL;
            }
            symbolPtr++;
        }
    }
    // 释放符号表空间.
    free(symbolTable);
    symbolTable = symbolPtr = NULL;
}

/**
 * @brief   打印词素 token.
 *
 * 功能函数, 打印词法分析输出的词素.
 *
 * @param   lineNo  行号.
 * @return  void
 * */
void printToken(int lineNo) {
    // 判断词素类别, 并打印相关信息
    switch (token) {
        case 0:
            break;
        case '!':
        case ']':
        case '(':
        case ')':
        case '{':
        case '}':
        case ',':
        case ';':
            fprintf(scannerOutputStream, "\t%-3d:                      %c\n", lineNo, (char)token);
            break;
        case Num:
            fprintf(scannerOutputStream, "\t%-3d: Num            --->   %lld\n", lineNo, tokenValue);
            break;
        case Character:
            fprintf(scannerOutputStream, "\t%-3d: Char           --->   %c\n", lineNo, (char)tokenValue);
            break;
        case String:
            fprintf(scannerOutputStream, "\t%-3d: String         --->   %s\n", lineNo, (char*)tokenValue);
            break;
        case Id:
            fprintf(scannerOutputStream, "\t%-3d: Id             --->   %s\n", lineNo, (char*)tokenValue);
            break;
        case INT:
        case CHAR:
        case VOID:
        case IF:
        case ELSE:
        case FOR:
        case WHILE:
        case DO:
        case RETURN:
            fprintf(scannerOutputStream, "\t%-3d: reserved word  --->   %s\n", lineNo, (char*)tokenValue);
            break;
        case Assign:
            fprintf(scannerOutputStream, "\t%-3d: assign                =\n", lineNo);
            break;
        case Lor:
            fprintf(scannerOutputStream, "\t%-3d: lor                  ||\n", lineNo);
            break;
        case Land:
            fprintf(scannerOutputStream, "\t%-3d: land                 &&\n", lineNo);
            break;
        case Or:
            fprintf(scannerOutputStream, "\t%-3d: or                    |\n", lineNo);
            break;
        case Xor:
            fprintf(scannerOutputStream, "\t%-3d: xor                   ^\n", lineNo);
            break;
        case And:
            fprintf(scannerOutputStream, "\t%-3d: and                   &\n", lineNo);
            break;
        case Eq:
            fprintf(scannerOutputStream, "\t%-3d: eq                   ==\n", lineNo);
            break;
        case Ne:
            fprintf(scannerOutputStream, "\t%-3d: ne                   !=\n", lineNo);
            break;
        case Lt:
            fprintf(scannerOutputStream, "\t%-3d: less than             <\n", lineNo);
            break;
        case Gt:
            fprintf(scannerOutputStream, "\t%-3d: greater than          >\n", lineNo);
            break;
        case Le:
            fprintf(scannerOutputStream, "\t%-3d: le                   <=\n", lineNo);
            break;
        case Ge:
            fprintf(scannerOutputStream, "\t%-3d: ge                   >=\n", lineNo);
            break;
        case Shl:
            fprintf(scannerOutputStream, "\t%-3d: shl                  <<\n", lineNo);
            break;
        case Shr:
            fprintf(scannerOutputStream, "\t%-3d: shr                  >>\n", lineNo);
            break;
        case Add:
            fprintf(scannerOutputStream, "\t%-3d: add                   +\n", lineNo);
            break;
        case Sub:
            fprintf(scannerOutputStream, "\t%-3d: sub                   -\n", lineNo);
            break;
        case Mul:
            fprintf(scannerOutputStream, "\t%-3d: mul                   *\n", lineNo);
            break;
        case Div:
            fprintf(scannerOutputStream, "\t%-3d: div                   /\n", lineNo);
            break;
        case Mod:
            fprintf(scannerOutputStream, "\t%-3d: mod                   %%\n", lineNo);
            break;
        case Inc:
            fprintf(scannerOutputStream, "\t%-3d: inc                  ++\n", lineNo);
            break;
        case Dec:
            fprintf(scannerOutputStream, "\t%-3d: dec                  --\n", lineNo);
            break;
        case Bracket:
            fprintf(scannerOutputStream, "\t%-3d:                      [\n", lineNo);
            break;
        default:
            handleErrorInformation(0, FUNCTION_ERROR, "utility.c/printToken()",
                                   "Get an Unknown Token", NULL);
    }
}

/**
 * @brief 打印抽象语法树节点.
 *
 * 打印 AST 节点内容.
 *
 * @param   void
 * @return  void
 * */
void printTree() {
    // 从根节点遍历 AST.
    printNode(root, 0);
}

/**
 * @brief 打印汇编结果.
 *
 * 打印汇编语言代码.
 *
 * @param   void
 * @return  void
 * */
void printAssemble() {
    char *instructions [34] = {
            "IMM", "LEA", "JMP", "JZ", "JNZ", "CALL", "NVAR", "DARG",
            "RET", "LA", "LI", "LC", "SA", "SI", "SC", "PUSH", "OR", "XOR",
            "AND", "EQ", "NE", "LT", "GT", "LE", "GE", "SHL",
            "SHR", "ADD", "SUB", "MUL", "DIV", "MOD", "PRINTF", "EXIT"
    };                                  // 指令集.
    long long *tempCode = code;         // 临时指针, 定位到代码段起始处.
    int lineNo = 1;                     // 代码行号.
    // 遍历代码段指针, 打印相关信息.
    while (tempCode < codePtr) {
        fprintf(codeGeneratorOutputStream, "%-4d (%lld)  %.5s",lineNo, (long long)tempCode, instructions[*tempCode - 1]);
        // 判断是否是带参数指令.
        if (*tempCode < RET) {
            // 打印参数.
            tempCode++;
            fprintf(codeGeneratorOutputStream, "\t%lld", *tempCode);
        }
        // 换行.
        fprintf(codeGeneratorOutputStream, "\n");
        // 行号自增.
        lineNo++;
        // 代码段指针后移.
        tempCode++;
    }
}

/**
 * @brief 打印错误信息.
 *
 * 打印错误信息到控制台.
 *
 * @param   lineNo      行号.
 * @param   errorCode   错误代码.
 * @param   location    错误位置.
 * @param   error       错误信息.
 * @param   message     具体内容.
 * @return  void
 * */
void handleErrorInformation(int lineNo, eErrorCode errorCode , const char *location, const char *error, const char *message) {
    if (NULL == message) {
        fprintf(errorOutputStream,"line %d: Get an Error in Function %s\n\tError Message: %s !\n",lineNo, location, error);
    } else {
        fprintf(errorOutputStream,"line %d: Get an Error in Function %s\n\tError Message: %s\n\tMore Information: %s !\n",lineNo,
                    location, error, message);
    }
    exit(errorCode);
}

/**
 * @brief   载入源代码.
 *
 * 通过文件指针打开指定源代码文件, 并将文件中内容读入到源代码缓冲区.
 *
 * @param   fileName    源代码文件名
 * @return  void
 * */
static void loadSrc(const char *fileName){
    FILE *filePtr = NULL;           // 文件指针.
    long size = 0;                 // 存储源代码长度.

    // 打开源代码文件.
    filePtr = fopen(fileName,"r");
    // 判断文件是否成功打开.
    if (NULL == filePtr) {
        // 打开文件失败, 打印错误信息.
        handleErrorInformation(0,FILE_ERROR,"utility.c/loadSrc()",
                               "Could Not Open Source Code: ", fileName);
    }
    // 移动指针到文件末尾 , 获取文件长度.
    fseek(filePtr,0,SEEK_END);
    size = ftell(filePtr);
    // 恢复指针到文件首.
    fseek(filePtr,0,SEEK_SET);
    // 读取源代码到文件缓冲区.
    size = (long)fread(source,sizeof(char),size,filePtr);
    // 判断读取源代码是否成功.
    if (0 == size) {
        // 读取源代码失败, 打印错误信息.
        handleErrorInformation(0, FILE_ERROR, "utility.c/loadSrc()",
                               "Could Not Read Source Code: ", fileName);
        exit(1);
    }
    // 关闭源代码文件.
    fclose(filePtr);
}

/**
 * @brief 获取不带扩展名的文件路径长度.
 *
 * 判断文件是否带有扩展名, 如果带有扩展名则返回消除扩展名后的文件路径长度, 否则打印错误信息.
 *
 * @param   fileName  文件路径.
 * @return  baseLen  不带扩展名的文件路径长度.
 * */
static int getBaseFileNameLen(const char *fileName) {
    int baseLen = 0;                             // 消除扩展名后的文件名长度.
    char *chPtr = strchr(fileName, '.');    // 定位扩展名前的 '.' 的位置.

    // 判断文件是否有扩展名.
    if (NULL == chPtr) {
        // 不带扩展名, 打印错误信息.
        handleErrorInformation(0, INIT_ERROR, "utility.c/getBaseFileNameLen()",
                               "Get an Invalid File Name", NULL);
    }
    // 带扩展名, 复制扩展名前的文件路径.
    baseLen = (int)(chPtr - fileName);
    // 返回不带扩展名的文件长度.
    return baseLen;
}

/**
 * @brief 初始化源代码加载器.
 *
 * 初始化源代码存储区, 读取源代码文件到存储区,并初始化源代码指针.
 *
 * @param   fileName    加载文件名
 * @return  void
 * */
static void initFileLoader(const char *fileName) {
    // 分配源代码存储区并初始化为 0.
    source = (char*)calloc(MAX_SIZE, sizeof(char));
    // 判断源代码存储区是否分配成功.
    if (NULL == source) {
        // 分配源代码区失败, 打印错误信息.
        handleErrorInformation(0, INIT_ERROR, "utility.c/initFileLoader()",
                               "Could Not Malloc Space for Source Code", NULL);
    }
    // 载入源代码.
    loadSrc(fileName);
    // 初始化源代码指针, 指向源代码缓冲区开始处.
    sourcePtr = source;
}

/**
 * @brief 初始化词法分析器.
 *
 * 初始化词法分析输出流, 符号表存储区, 数据段存储区, 并初始化符号表指针与数据段指针.
 *
 * @param   fileName     加载文件名.
 * @param   baseLen     不带扩展名的文件名长度.
 * @return  void
 * */
static void initScanner(const char *fileName, int baseLen) {
    char *outputFileName = NULL;        // 词法分析输出文件名.

    // 判断是否重定向到文件中.
    if (fileName != NULL) {
        // 为输出文件名分配空间.
        outputFileName = calloc((strlen(fileName) + 18), sizeof(char));
        // 判断输出文件名空间是否分配成功..
        if (NULL == outputFileName) {
            // 分配输出文件名空间失败, 打印错误信息.
            handleErrorInformation(0, INIT_ERROR, "utility.c/initScanner()",
                                   "Could Not Malloc Space for Output File Name", outputFileName);
        }
        // 获取输出文件名前半部分.
        strncpy(outputFileName, fileName, baseLen);
        // 补充输出文件名后半部分.
        strncat(outputFileName, "ScannerOutput.txt", 18);
        // 打开输出文件.
        scannerOutputStream = fopen(outputFileName, "w+");
        // 判断文件是否成功打开.
        if (NULL == scannerOutputStream) {
            // 打开文件失败, 打印错误信息.
            handleErrorInformation(0, INIT_ERROR, "utility.c/initScanner()",
                                   "Could Not Open Scanner Output File", outputFileName);
        }
        // 释放文件名空间.
        free(outputFileName);
    } else {
        // 不重定向到文件, 输出到标准输出.
        scannerOutputStream = stdout;
    }
    // 分配符号表存储区并初始化为0.
    symbolTable = (sSymbol*)calloc(MAX_SIZE, sizeof(sSymbol));
    // 判断符号表存储区是否分配成功.
    if (NULL == symbolTable) {
        // 分配符号表存储区失败, 打印错误信息.
        handleErrorInformation(0, INIT_ERROR, "utility.c/initScanner()",
                               "Could Not Malloc Space for Symbol Table", NULL);
    }
    // 分配数据段存储区并初始化为0.
    data = (long long*)calloc(MAX_SIZE, sizeof(long long));
    // 判断数据段存储区是否分配成功.
    if (NULL == data) {
        // 分配数据段存储区失败, 打印错误信息.
        handleErrorInformation(0, INIT_ERROR, "utility.c/initScanner()",
                               "Could Not Malloc Space for Data Segment", NULL);
    }
    // 初始化符号表指针, 指向符号表开始处.
    symbolPtr = symbolTable;
    // 初始化数据段指针, 指向数据段段开始处.
    dataPtr = data;
    // 初始化行号.
    line = 1;
}

/**
 * @brief 初始化语法分析器.
 *
 * 初始化语法分析输出流.
 *
 * @param   fileName     加载文件名.
 * @param   baseLen     不带扩展名的文件名长度.
 * @return  void
 * */
static void initParser(const char *fileName, int baseLen) {
    char *outputFileName = NULL;        // 语法分析输出文件名

    // 判断是否重定向到文件中.
    if (fileName != NULL) {
        // 为输出文件名分配空间并初始化为0.
        outputFileName = calloc((strlen(fileName) + 17), sizeof(char));
        // 判断输出文件名空间是否分配成功..
        if (NULL == outputFileName) {
            // 分配输出文件名空间失败, 打印错误信息..
            handleErrorInformation(0, INIT_ERROR, "utility.c/initParser()",
                                   "Could Not Malloc Space for Output File Name", outputFileName);
        }
        // 获取输出文件名前半部分.
        strncpy(outputFileName, fileName, baseLen);
        // 补充输出文件名后半部分.
        strncat(outputFileName, "ParserOutput.txt", 17);
        // 打开输出文件.
        parserOutputStream = fopen(outputFileName, "w+");
        // 判断文件是否成功打开.
        if (NULL == scannerOutputStream) {
            // 打开文件失败, 打印错误信息.
            handleErrorInformation(0, INIT_ERROR, "utility.c/initParser()",
                                   "Could Not Open Parser Output File", outputFileName);
        }
        // 释放文件名空间.
        free(outputFileName);
    } else {
        // 不重定向到文件, 输出到标准输出.
        parserOutputStream = stdout;
    }
}

/**
 * @brief 初始化代码生成器.
 *
 * 初始化代码生成输出流, 代码段存储区, 并初始化代码段指针.
 *
 * @param   fileName     加载文件名.
 * @param   baseLen     不带扩展名的文件名长度.
 * @return  void
 * */
static void initCodeGenerator(const char *fileName, int baseLen) {
    char *outputFileName = NULL;        // 代码生成输出文件名

    // 判断是否重定向到文件中.
    if (fileName != NULL) {
        // 为输出文件名分配空间并初始化为0.
        outputFileName = calloc((strlen(fileName) + 24), sizeof(char));
        // 判断输出文件名空间是否分配成功..
        if (NULL == outputFileName) {
            // 分配输出文件名空间失败, 打印错误信息..
            handleErrorInformation(0, INIT_ERROR, "utility.c/initCodeGenerator()",
                                   "Could Not Malloc Space for Output File Name", outputFileName);
        }
        // 获取输出文件名前半部分.
        strncpy(outputFileName, fileName, baseLen);
        // 补充输出文件名后半部分.
        strncat(outputFileName, "CodeGeneratorOutput.txt", 24);
        // 打开输出文件.
        codeGeneratorOutputStream = fopen(outputFileName, "w+");
        // 判断文件是否成功打开.
        if (NULL == scannerOutputStream) {
            // 打开文件失败, 打印错误信息.
            handleErrorInformation(0, INIT_ERROR, "utility.c/initCodeGenerator()",
                                   "Could Not Open Code Generator Output File", outputFileName);
        }
        // 释放文件名空间.
        free(outputFileName);
    } else {
        // 不重定向到文件, 输出到标准输出.
        codeGeneratorOutputStream = stdout;
    }
    // 分配代码段存储区并初始化为0.
    code = (long long*)calloc(MAX_SIZE, sizeof(long long));
    // 判断代码段码存储区是否分配成功.
    if (NULL == code) {
        // 分配代码段码存储区失败, 打印错误信息.
        handleErrorInformation(0, INIT_ERROR, "utility.c/init()",
                               "Could Not Malloc Space for Code Segment", NULL);
    }
    // 初始化代码段指针, 指向代码段开始处.
    codePtr = code;
}

/**
 * @brief 初始化代码运行器.
 *
 * 初始化代码运行输出流, 堆栈段存储区, 并初始化堆栈段指针.
 *
 * @param   fileName     加载文件名.
 * @param   baseLen     不带扩展名的文件名长度.
 * @return  void
 * */
static void initCodeRunner(const char *fileName, int baseLen) {
    char *outputFileName = NULL;        // 词法分析输出文件名

    // 判断是否重定向到文件中.
    if (fileName != NULL) {
        // 为输出文件名分配空间并初始化为0.
        outputFileName = calloc((strlen(fileName) + 21), sizeof(char));
        // 判断输出文件名空间是否分配成功..
        if (NULL == outputFileName) {
            // 分配输出文件名空间失败, 打印错误信息..
            handleErrorInformation(0, INIT_ERROR, "utility.c/initCodeRunner()",
                                   "Could Not Malloc Space for Output File Name", outputFileName);
        }
        // 获取输出文件名前半部分.
        strncpy(outputFileName, fileName, baseLen);
        // 补充输出文件名后半部分.
        strncat(outputFileName, "CodeRunnerOutput.txt", 21);
        // 打开输出文件.
        codeRunnerOutputStream = fopen(outputFileName, "w+");
        // 判断文件是否成功打开.
        if (NULL == scannerOutputStream) {
            // 打开文件失败, 打印错误信息.
            handleErrorInformation(0, INIT_ERROR, "utility.c/initCodeRunner()",
                                   "Could Not Open Code Runner Output File", outputFileName);
        }
        // 释放文件名空间.
        free(outputFileName);
    } else {
        // 不重定向到文件, 输出到标准输出.
        codeRunnerOutputStream = stdout;
    }
    // 分配堆栈段存储区并初始化为0.
    stack = (long long*)calloc(MAX_SIZE, sizeof(long long));
    // 判断堆栈段码存储区是否分配成功.
    if (NULL == stack) {
        // 分配堆栈段码存储区失败, 打印错误信息.
        handleErrorInformation(0, INIT_ERROR, "utility.c/init()",
                               "Could Not Malloc Space for Stack Segment", NULL);
    }
}
/**
 * @brief 打印类型.
 *
 * 打印类型: Int, Char, Void, pointer.
 *
 * @param   type    类型.
 * @return  void
 * */
static void printType(int type) {
    // 判断类型, 打印相关信息.
    if (Int == type) {
        fprintf(parserOutputStream, "INT");
    } else if (Char == type) {
        fprintf(parserOutputStream, "CHAR");
    } else if (Void == type) {
        fprintf(parserOutputStream, "VOID");
    } else if (type > Ptr) {
        // 判断是 Int 指针 还是 Char 指针.
        if (1 == type % Ptr) {
            // 打印 Int 指针.
            fprintf(parserOutputStream, "INT ");
        } else {
            // 打印 Char 指针.
            fprintf(parserOutputStream, "CHAR");
        }
        // 打印指针.
        for (int i = type / Ptr; i > 0; i--) {
            fprintf(parserOutputStream, " PTR");
        }
    } else {
        // 打印错误信息.
        handleErrorInformation(0, FUNCTION_ERROR, "utility.c/printType()",
                               "Get an Unknown Type", NULL);
    }
}

/**
 * @brief   打印运算符.
 *
 * 打印运算符类型.
 *
 * @param   op  运算符.
 * @return  void
 * */
static void printOperator(int op) {
    switch (op) {
        case Assign:
            fprintf(parserOutputStream, "assign(=)");
            break;
        case Or:
            fprintf(parserOutputStream, "or(|)");
            break;
        case Xor:
            fprintf(parserOutputStream, "xor(^)");
            break;
        case And:
            fprintf(parserOutputStream, "and(&)");
            break;
        case Lt:
            fprintf(parserOutputStream, "less than(<)");
            break;
        case Gt:
            fprintf(parserOutputStream, "greater than(>)");
            break;
        case Add:
            fprintf(parserOutputStream, "add(+)");
            break;
        case Sub:
            fprintf(parserOutputStream, "sub(-)");
            break;
        case Mul:
            fprintf(parserOutputStream, "mul(*)");
            break;
        case Div:
            fprintf(parserOutputStream, "div(/)");
            break;
        case Mod:
            fprintf(parserOutputStream, "mod(%%)");
            break;
        case Bracket:
            fprintf(parserOutputStream, "bracket([])");
            break;
        case '!':
            fprintf(parserOutputStream, "Not(!)");
            break;
        case ']':
        case '(':
        case ')':
        case '{':
        case '}':
        case ',':
        case ';':
            fprintf(parserOutputStream, "%c", op);
            break;
        case Lor:
            fprintf(parserOutputStream, "lor(||)");
            break;
        case Land:
            fprintf(parserOutputStream, "land(&&)");
            break;
        case Eq:
            fprintf(parserOutputStream, "equal(==)");
            break;
        case Ne:
            fprintf(parserOutputStream, "not equal(!=)");
            break;
        case Le:
            fprintf(parserOutputStream, "le(<=)");
            break;
        case Ge:
            fprintf(parserOutputStream, "ge(>=)");
            break;
        case Shl:
            fprintf(parserOutputStream, "shl(<<)");
            break;
        case Shr:
            fprintf(parserOutputStream, "shr(>>)");
            break;
        case Inc:
            fprintf(parserOutputStream, "inc(++)");
            break;
        case Dec:
            fprintf(parserOutputStream, "dec(--)");
            break;
        default:
            handleErrorInformation(0, FUNCTION_ERROR, "utility.c/printOperator()",
                                   "Get an Unknown Operator", NULL);
    }
}

/**
 * @brief 打印制表符.
 *
 * 根据传入参数 n, 打印 n 个制表符与分割线.
 *
 * @param n 制表符个数.
 * @return void
 * */
static void printTab(int n) {
    for (int i = 0; i < n; i++) {
        fprintf(parserOutputStream, "\t|");
    }
}

/**
 * @brief 打印抽象语法树节点.
 *
 * 打印 AST 节点内容.
 *
 * @param   node  抽象语法树节点.
 * @param   n     递归深度.
 * @return  void
 * */
static void printNode(sTreeNode *node, int n) {
    // 遍历非空节点
    while (node != NULL) {
        // 根据节点语句类型, 采取不同操作.
        if (Function == node->statementType) {
            // 处理函数节点.
            // 处理函数名与函数返回值类型.
            printTab(n);
            fprintf(parserOutputStream, "Function name: %s", node->name);
            fprintf(parserOutputStream, "\ttype: ");
            printType(node->identifierType);
            fprintf(parserOutputStream, "\n");
            // 处理参数列表.
            printTab(n + 1);
            fprintf(parserOutputStream, "Parameters:\n");
            printNode(node->children[0], n + 2);
            // 处理函数体.
            printTab(n + 1);
            fprintf(parserOutputStream, "Function Body:\n");
            printNode(node->children[1], n + 2);
        } else if (IfStatement == node->statementType) {
            // 处理 If 语句节点.
            printTab(n);
            fprintf(parserOutputStream, "IF statement:\n");
            // 处理条件表达式.
            printTab(n + 1);
            fprintf(parserOutputStream, "Conditions:\n");
            printNode(node->children[0],n + 2);
            // 处理成功分支语句.
            printTab(n + 1);
            fprintf(parserOutputStream, "Success condition:\n");
            printNode(node->children[1], n + 2);
            // 判断是否有 else 语句.
            if (node->children[2] != NULL) {
                // 处理失败分支语句.
                printTab(n + 1);
                fprintf(parserOutputStream, "Failure condition:\n");
                printNode(node->children[2], n + 2);
            }
        } else if (WhileStatement == node->statementType) {
            // 处理 While 语句节点.
            printTab(n);
            fprintf(parserOutputStream, "While statement:\n");
            // 处理条件表达式.
            printTab(n + 1);
            fprintf(parserOutputStream, "Conditions:\n");
            printNode(node->children[0],n + 2);
            // 处理循环体.
            printTab(n + 1);
            fprintf(parserOutputStream, "While Body:\n");
            printNode(node->children[1], n + 2);
        } else if (ForStatement == node->statementType) {
            // 处理 For 语句节点.
            printTab(n);
            fprintf(parserOutputStream, "For statement:\n");
            // 处理初始化表达式.
            printTab(n + 1);
            fprintf(parserOutputStream, "Initiation:\n");
            printNode(node->children[0],n + 2);
            // 处理条件表达式.
            printTab(n + 1);
            fprintf(parserOutputStream, "Conditions:\n");
            printNode(node->children[1],n + 2);
            // 处理更新条件.
            printTab(n + 1);
            fprintf(parserOutputStream, "Update :\n");
            printNode(node->children[2], n + 2);
            // 处理循环体.
            printTab(n + 1);
            fprintf(parserOutputStream, "While Body:\n");
            printNode(node->children[3], n + 2);
        } else if (DoWhileStatement == node->statementType) {
            // 处理 Do While 语句节点.
            printTab(n);
            fprintf(parserOutputStream, "Do While statement:\n");
            // 处理条件表达式.
            printTab(n + 1);
            fprintf(parserOutputStream, "Conditions:\n");
            printNode(node->children[1],n + 2);
            // 处理循环体.
            printTab(n + 1);
            fprintf(parserOutputStream, "Do While Body:\n");
            printNode(node->children[0], n + 2);
        } else if (ExpressStatement == node->statementType) {
            // 处理表达式语句节点.
            printTab(n);
            if (Operator == node->expressionType) {
                // 打印运算符.
                printOperator(node->operatorType);
                fprintf(parserOutputStream, "\n");
            } else if (Constant == node->expressionType) {
                // 打印 Int, Char, String 常量.
                if (Char == node->identifierType) {
                    fprintf(parserOutputStream, "Char:   '%c'\n", (char)node->value);
                } else if (Int == node->identifierType) {
                    fprintf(parserOutputStream, "Num:     %lld\n", node->value);
                } else {
                    fprintf(parserOutputStream, "String:  %s\n", (char*)node->value);
                }
            } else if (Variable == node->expressionType) {
                // 打印变量.
                fprintf(parserOutputStream, "Id:%s\n", node->name);
            } else if (Call == node->expressionType) {
                // 打印函数调用.
                fprintf(parserOutputStream, "Function call:%s\n", node->name);
            }
            // 打印左侧子运算符.
            printNode(node->children[0], n + 1);
            // 打印右侧子运算符.
            printNode(node->children[1], n + 1);
        } else if (ReturnStatement == node->statementType) {
            // 处理返回语句节点.
            printTab(n);
            fprintf(parserOutputStream, "Return statement:\n");
            // 处理返回值.
            if (node->children[0] != NULL) {
                printTab(n + 1);
                fprintf(parserOutputStream, "Return value:\n");
                printNode(node->children[0], n + 2);
            }
        } else if (ParameterStatement == node->statementType) {
            // 处理参数节点.
            // 根据节点标识符类型打印不同信息.
            if (Void == node->identifierType) {
                // 处理空参数列表.
                printTab(n);
                printType(node->identifierType);
                fprintf(parserOutputStream, "\n");
            } else {
                // 处理非空参数列表.
                printTab(n);
                fprintf(parserOutputStream, "%s(",node->name);
                printType(node->identifierType);
                fprintf(parserOutputStream, ")\n");
            }
        } else if (DeclareStatement == node->statementType) {
            // 处理声明语句节点.
            printTab(n);
            fprintf(parserOutputStream, "Declare statement:\n");
            // 打印声明变量的类型.
            printTab(n + 1);
            fprintf(parserOutputStream, "Type:");
            printType(node->identifierType);
            // 打印声明变量的名称.
            if ((INT == node->identifierType) || (CHAR == node->identifierType)) {
                fprintf(parserOutputStream, "\t\t\tId name: %s\n", node->name);
            } else {
                fprintf(parserOutputStream, "\t\t\tId name: %s\t\t\tSize: %lld\n", node->name, node->size);
            }
        }
        // 处理兄弟节点
        node = node->sibling;
    }
}

/**
 * @brief 回收 AST 空间.
 *
 * 回收所有 AST 节点的空间.
 *
 * @param   node    AST 节点, 依次释放该节点的孩子节点与兄弟结点的空间.
 * @return  void
 * */
static void destroyNode(sTreeNode *node) {
    // 判断节点是否为空.
    if (NULL == node) {
        // 空节点, 直接返回.
        return;
    }
    // 遍历孩子节点, 依次释放孩子节点空间.
    for (int i = 0; i < MAX_CHILDREN; i++) {
        if (node->children[i] != NULL) {
            destroyNode(node->children[i]);
            node->children[i] = NULL;
        }
    }
    // 释放兄弟节点空间.
    if (node->sibling != NULL) {
        destroyNode(node->sibling);
        node->sibling = NULL;
    }
    // 释放自身指向 AST 节点空间.
    free(node);
}