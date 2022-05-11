#include "globals.h"
#include "utility.h"
#include "scan.h"
#include "parse.h"

char* source = NULL;
char* sourceDump = NULL;
char* sourcePtr = NULL;
char* tokenString = NULL;
FILE* filePtr = NULL;
long long token, tokenValue;
int line = 1;
int scanTrace = 1;
int sourceTrace = 0;
int parseTrace = 1;
struct treeNode* root = NULL;
struct treeNode** symbolStack;
int* operatorStack;
int top;
// symbol table & pointer
struct symbol * symbolTable,
            * symbolPtr;

int main(int argc, char** argv) {
    // 载入源代码
    loadSrc(*(argv + 1));
    // 初始化语法分析器
    init();
    // 打印源代码
    printf("%s\n",source);
    // 初始化关键字
    initKeywords();
    if (scanTrace && sourceTrace) {
        printSource(line);
    }
    parse();
    if (parseTrace) {
        printTree(root, 0);
    }
//    if (loadSrc(*(argv + 1))) {
//        if (!init()) {
//            printErrorInformation("Fail to Init", false);
//            exit(1);
//        }
//        printf("%s\n",source);
//        initKeywords();
//        if (scanTrace && sourceTrace) {
//            printSource(line);
//        }
////        while ((*source) != '\0') {
////            getToken();
////        }
//        parse();
//        printTree(root, 0);
//    } else {
//        printErrorInformation("Fail to Load Source File", false);
//        exit(1);
//    }
    return 0;
}
