
#include <stdio.h>

#include "globals.h"
#include "utility.h"
#include "scanner.h"
#include "parser.h"
#include "code.h"

FILE *scannerOutputStream;          // 词法分析输出流.
FILE *parserOutputStream;           // 语法分析输出流.
FILE *codeGeneratorOutputStream;    // 代码生成输出流.
FILE *codeRunnerOutputStream;       // 代码运行输出流.
FILE *errorOutputStream;            // 错误输出流.
char *source;                       // 源代码缓冲区.
char*sourcePtr;                     // 源代码指针.
sSymbol *symbolTable;               // 符号表.
sSymbol *symbolPtr;                 // 符号表指针.
long long *data;                    // 数据段.
long long *dataPtr;                 // 数据段指针.
long long token;                    // token类型
long long tokenValue;               // 常量值, 字符值, 字符串地址或变量名地址.
int line;                           // 行号.
sTreeNode *root;                    // AST 根节点.
long long *mainPtr;                 // main函数指针.
long long *code;                    // 代码段.
long long *codePtr;                 // 代码段指针.
long long *stack;                   // 堆栈段.
int phaseFlag;                      /*阶段标志位:
 * [0x0, 0x8) 不进行任何分析;
 * [0x8, 0xC) 进行到词法分析;
 * [0xC,0xE) 进行到语法分析;
 * [0xE, 0xF) 进行到代码生成;
 * [0xF, 0xF) 进行到代码运行.
 * */
int scanTrace;                      // 打印标志位: 是否打印词法分析结果.
int parseTrace;                     // 打印标志位: 是否打印语法分析结果.
int generateTrace;                  // 打印标志位: 是否打印代码生成结果.

int main(int argc, char** argv) {
    // 初始化标志位.
    phaseFlag = 0xF;
    scanTrace = 1;
    parseTrace = 1;
    generateTrace = 1;
    // 根据传入的参数个数, 处理输入所有源代码文件
    for (int i = 1; i < argc; i++) {
        // 初始化语法分析器
        init(*(argv + i));
        // 打印源代码
        printf("%s\n",source);
        // 初始化关键字
        initKeywords();
        if ((phaseFlag >= 8) && (phaseFlag < 0xC)) {
            // 只进行词法分析
            getToken();
            while (token > 0) {
                getToken();
            }
        } else if (phaseFlag >= 0xC){
            // 进行语法分析
            parse();
            if (parseTrace) {
                // 打印 AST
                printTree();
            }
            if (phaseFlag >= 0xE){
                // 进行代码生成
                generateCode(root);
                if (generateTrace) {
                    // 打印代码生成结果
                    printAssemble();
                }
                if (0xF == phaseFlag) {
                    // 运行代码
                    runCode();
                }
            }
        }
        // 回收编译器空间, 关闭相关输出流.
        destroy();
    }
    // 打印结束信息
    if (argc < 3) {
        printf("\nSuccessfully Compile %d File!\n", argc - 1);
    } else {
        printf("\nSuccessfully Compile %d Files!\n", argc - 1);
    }
    return 0;
}
