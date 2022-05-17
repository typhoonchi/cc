#include "globals.h"
#include "utility.h"
#include "scanner.h"
#include "parser.h"
#include "code.h"

char* source = NULL;                // 源代码缓冲区指针
char* sourceDump = NULL;            // 源代码备份指针
char* sourcePtr = NULL;             // 源代码打印指针
char* tokenString = NULL;           // token 名称缓冲区
FILE* filePtr = NULL;                // 文件指针
long long token, tokenValue;        // token类型与值
int line = 1;                       // 行号
int scanTrace = 1;                  // 是否打印词法分析结果
int sourceTrace = 1;                // 是否打印源代码
int parseTrace = 1;                 // 是否打印 AST
struct treeNode* root = NULL;       // AST 根节点
struct treeNode** nodeStack;        // 表达式栈
int top;                            // 栈顶
struct symbol* symbolTable, * symbolPtr; // 符号表与符号表指针
long long* code, *codeDump, *mainPtr, * stack, * data;
long long* pc, * sp, * bp, ibp;
long long ax, cycle;

int main(int argc, char** argv) {
    // 载入源代码
    loadSrc(*(argv + 1));
    // 初始化语法分析器
    init();
    // 打印源代码
    printf("%s\n",source);
    // 初始化关键字
    initKeywords();
    // 打印源代码
    if (scanTrace && sourceTrace) {
        printSource(line);
    }
    // 语法分析
    parse();
    if (parseTrace) {
        // 打印 AST
        printTree(root, 0);
    }
    printAssemble();
    runCode(--argc,++argv);
    return 0;
}
