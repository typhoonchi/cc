
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "globals.h"
#include "utility.h"
#include "scanner.h"
#include "parser.h"
#include "code.h"

char* source = NULL;                // 源代码缓冲区指针
char* sourceDump = NULL;            // 源代码备份指针
char* tokenString = NULL;           // token 名称缓冲区
long long token, tokenValue;        // token类型与值
int line = 1;                       // 行号
int scanTrace = 1;                  // 是否打印词法分析结果
sTreeNode* root = NULL;             // AST 根节点
sSymbol* symbolTable, * symbolPtr;  // 符号表与符号表指针
long long* code, *codeDump, *mainPtr, * stack, * data;  // 代码段指针, 代码段备份指针, main函数指针, 堆栈段指针, 数据段指针

static int scanOnly = 0;                    // 是否只进行词法分析
static int parseOnly = 0;                   // 是否只进行词法分析和语法分析
static int parseTrace = 1;                  // 是否打印 AST
static int assemblyTrace = 1;               // 是否打印 AST

int main(int argc, char** argv) {
    // 载入源代码
    loadSrc(*(argv + 1));
    // 初始化语法分析器
    init();
    // 打印源代码
    printf("%s\n",source);
    // 初始化关键字
    initKeywords();
    if (scanOnly) {
        // 词法分析
        getToken();
        while (token > 0) {
            getToken();
        }
    } else {
        // 语法分析
        parse();
        if (parseTrace) {
            // 打印 AST
            printTree(root, 0);
        }
        if (!parseOnly) {
            generateCode(root);
            if (assemblyTrace) {
                printAssemble();
            }
            runCode();
        }
    }

    return 0;
}
