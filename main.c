
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "globals.h"
#include "utility.h"
#include "scanner.h"
#include "parser.h"
#include "code.h"

FILE *scannerOutputStream, *parserOutputStream, *codeGeneratorOutputStream,
        *codeRunnerOutputStream, *errorOutputStream;                     //
char *source, *sourcePtr;                // 源代码缓冲区指针源代码备份指针
char *tokenString;           // token 名称缓冲区
long long token, tokenValue;        // token类型与值
int line = 1;                       // 行号
int phaseFlag = 0xF;                  //阶段标志位 [0x0, 0x8) 不进行任何分析; [0x8, 0xC) 进行到词法分析; [0xC,0xE) 进行到语法分析;
                                        // [0xE, 0xF) 进行到代码生成; [0xF, 0xF) 进行到代码运行
int scanTrace = 1;                  // 是否打印词法分析结果
int parseTrace = 1;                  // 是否打印 AST
int generateTrace = 1;               // 是否打印代码生成结果
sTreeNode* root = NULL;             // AST 根节点
sSymbol* symbolTable, * symbolPtr;  // 符号表与符号表指针
long long *code, *codePtr, *mainPtr,
            *stack, *data, *dataPtr;  // 代码段指针, 代码段备份指针, main函数指针, 堆栈段指针, 数据段指针


int main(int argc, char** argv) {
    // 初始化语法分析器
    init(*(argv + 1));
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
    destroy();
    printf("\nSuccessfully Compile !\n");
    return 0;
}
