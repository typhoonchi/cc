//
// Created by zhangyukun on 2022/4/11.
//

#include "globals.h"
#include "utility.h"


/* 载入源代码
 *
 * @param fileName 源代码文件名
 * */
void loadSrc(const char* fileName){
    // 存储源代码长度
    long size = 0;
    // 打开源代码文件
    filePtr = fopen(fileName,"r");
    if (filePtr == NULL) {
        printErrorInformation("Could Not Open Source Code:", fileName);
        exit(1);
    }
    // 分配缓冲区读取源代码
    source = (char*)malloc(MAXSIZE);
    if (source == NULL) {
        printErrorInformation("Could Not Malloc for Source Code", NULL);
        exit(1);
    }
    // 移动指针到文件末尾 , 获取文件长度
    fseek(filePtr,0,SEEK_END);
    size = ftell(filePtr);
    // 恢复指针到文件首
    fseek(filePtr,0,SEEK_SET);
    // 读取源代码到文件缓冲区
    if (fread(source,sizeof(char),size,filePtr) == 0) {
        printErrorInformation("Could Not Read Source Code:", fileName);
        exit(1);
    }
    sourcePtr = sourceDump = source;
    // 关闭源代码文件
    fclose(filePtr);
}

/* 初始化编译器
 *
 * */
void init(void) {
    // 分配符号表存储区
    symbolTable = (struct symbol*)malloc(MAXSIZE * sizeof(struct symbol));
    // 分配表达式栈存储区
    nodeStack = (struct treeNode**) malloc(STACKSIZE * sizeof(struct treeNode*));
    // 初始化栈顶
    top = 0;
    if (symbolTable == NULL || nodeStack == NULL) {
        // 初始化失败
        printErrorInformation("Fail to Init", NULL);
        exit(1);
    }
    // 初始化符号表存储区
    memset(symbolTable, 0, MAXSIZE * sizeof(struct symbol));
    memset(nodeStack, 0, STACKSIZE * sizeof(struct treeNode*));
    // 初始化符号表指针
    symbolPtr = symbolTable;
}

/* 打印词素 token
 *
 * @param lineNo 行号
 * */
void printToken(int lineNo) {
    switch (token) {
        case Id:
            printf("\t%-3d: Id             --->   %s\n",lineNo, tokenString);
            break;
        case String:
            printf("\t%-3d: String         --->   %s\n",lineNo, tokenString);
            break;
        case Char:
            printf("\t%-3d: Char           --->   %c\n",lineNo, (char)tokenValue);
            break;
        case Num:
            printf("\t%-3d: Num            --->   %lld\n",lineNo, tokenValue);
            break;
        case CHAR:
        case INT:
        case IF:
        case ELSE:
        case RETURN:
        case WHILE:
        case VOID:
            printf("\t%-3d: reserved word  --->   %s\n",lineNo, tokenString);
            break;
        case Assign:
            printf("\t%-3d: assign                =\n",lineNo);
            break;
        case Or:
            printf("\t%-3d: or                    |\n",lineNo);
            break;
        case Xor:
            printf("\t%-3d: xor                   ^\n",lineNo);
            break;
        case And:
            printf("\t%-3d: and                   &\n",lineNo);
            break;
        case Lt:
            printf("\t%-3d: less than             <\n",lineNo);
            break;
        case Gt:
            printf("\t%-3d: greater than          >\n",lineNo);
            break;
        case Add:
            printf("\t%-3d: add                   +\n",lineNo);
            break;
        case Sub:
            printf("\t%-3d: sub                   -\n",lineNo);
            break;
        case Mul:
            printf("\t%-3d: mul                   *\n",lineNo);
            break;
        case Div:
            printf("\t%-3d: div                   /\n",lineNo);
            break;
        case Mod:
            printf("\t%-3d: mod                   %%\n",lineNo);
            break;
        case Bracket:
            printf("\t%-3d:                      [\n",lineNo);
            break;
        case '!':
        case ']':
        case '(':
        case ')':
        case '{':
        case '}':
        case ',':
        case ';':
            printf("\t%-3d:                      %c\n",lineNo,(char)token);
            break;
        case Lor:
            printf("\t%-3d: lor                  ||\n",lineNo);
            break;
        case Land:
            printf("\t%-3d: land                 &&\n",lineNo);
            break;
        case Eq:
            printf("\t%-3d: eq                   ==\n",lineNo);
            break;
        case Ne:
            printf("\t%-3d: ne                   !=\n",lineNo);
            break;
        case Le:
            printf("\t%-3d: le                   <=\n",lineNo);
            break;
        case Ge:
            printf("\t%-3d: ge                   >=\n",lineNo);
            break;
        case Shl:
            printf("\t%-3d: shl                  <<\n",lineNo);
            break;
        case Shr:
            printf("\t%-3d: shr                  >>\n",lineNo);
            break;
        case Inc:
            printf("\t%-3d: inc                  ++\n",lineNo);
            break;
        case Dec:
            printf("\t%-3d: dec                  --\n",lineNo);
            break;
        default:
            break;
    }
}

/* 打印源代码
 *
 * @param lineNo 行号
 * */
void printSource(int lineNo){
    printf("%d : ",lineNo);
    while ((*sourcePtr != '\n') && (*sourcePtr != '\0')){
        printf("%c",(char)*sourcePtr);
        sourcePtr++;
    }
    if (*sourcePtr != '\0') {
        printf("%c",(char)*sourcePtr);
        sourcePtr++;
    } else {
        printf("\n");
    }
}

/* 创建抽象语法树节点
 *
 * @param statementType  语句类型 If, While, Return,...
 * @param expressionType 表达式类型 Operator, Constant, Identifier, Call
 * @param operatorType   运算符类型 << >> ...
 * @return  抽象语法树节点指针, 指向语句节点
 * */
struct treeNode* createNode(int statementType, int expressionType, int operatorType){
    // 创建空节点
    struct treeNode* node = (struct treeNode*) malloc(sizeof(struct treeNode));
    if (node == NULL) {
        // 创建失败
        printErrorInformation("Fail to Create AST Node", NULL);
        exit(1);
    }
    // 初始化语句类型
    node->statementType = statementType;
    // 初始化孩子节点
    for (int i = 0; i < MAXCHILDREN; i++) {
        node->children[i] = NULL;
    }
    // 初始化兄弟结点
    node->sibling = NULL;
    //初始化数组相关内容
    node->size = 1;
    node->isArray = false;
    // 初始化表达式类型 , 仅当语句类型为表达式时有效
    node->expressionType = expressionType;
    // 初始化运算符类型 , 仅当语句类型为表达式时有效
    node->operatorType = operatorType;
    // 初始化常量值与变量名
    node->value = 0;
    node->name = NULL;
    return node;
}

/* 打印抽象语法树
 *
 * @param node 抽象语法树节点
 * @param n 递归深度
 * */
void printTree(struct treeNode* node, int n){
    while (node != NULL) {
        // 遍历非空节点
        if (node->statementType == Function) {
            // 处理函数节点

            // 处理函数名与函数返回值类型
            printTab(n);
            printf("Function name:%s\ttype:",node->name);
            printType(node->identifierType);
            printf("\n");

            // 处理参数列表
            printTab(n + 1);
            printf("Parameters:\n");
            printTree(node->children[0], n + 2);
            // 处理函数体
            printTab(n + 1);
            printf("Function Body:\n");
            printTree(node->children[1], n + 2);
        } else if (node->statementType == IfStatement) {
            // 处理 If 语句
            printTab(n);
            printf("IF statement:\n");
            // 处理条件表达式
            printTab(n + 1);
            printf("Conditions:\n");
            printTree(node->children[0],n + 2);
            // 处理成功分支语句
            printTab(n + 1);
            printf("Success condition:\n");
            printTree(node->children[1], n + 2);
            if (node->children[2] != NULL) {
                // 处理失败分支语句
                printTab(n + 1);
                printf("Failure condition:\n");
                printTree(node->children[2], n + 2);
            }
        } else if (node->statementType == WhileStatement) {
            // 处理 While 语句
            printTab(n);
            printf("While statement:\n");
            // 处理条件表达式
            printTab(n + 1);
            printf("Conditions:\n");
            printTree(node->children[0],n + 2);
            // 处理循环体
            printTab(n + 1);
            printf("While Body:\n");
            printTree(node->children[1], n + 2);
        } else if (node->statementType == ExpressStatement) {
            // 处理表达式语句
            printTab(n);
            if (node->expressionType == Operator) {
                // 打印运算符
                printOperator(node->operatorType);
                printf("\n");
            } else if (node->expressionType == Constant) {
                // 打印 Int 或 Char 常量
                if (node->identifierType == CHAR){
                    printf("Char:'%c'\n",(char)node->value);
                } else {
                    printf("Num:%lld\n",node->value);
                }
            } else if (node->expressionType == Identifier) {
                // 打印变量
                printf("Id:%s\n",node->name);
            } else if (node->expressionType == Call) {
                // 打印函数调用
                printf("Function call:%s\n",node->name);
            }
            // 打印左侧子运算符
            printTree(node->children[0], n + 1);
            // 打印右侧子运算符
            printTree(node->children[1], n + 1);
        } else if (node->statementType == ReturnStatement) {
            // 处理返回语句
            printTab(n);
            printf("Return statement:\n");
            // 处理返回值
            printTab(n + 1);
            printf("Return value:\n");
            printTree(node->children[0], n + 2);
        } else if (node->statementType == ParameterStatement) {
            // 处理参数列表
            if (node->identifierType == VOID) {
                // 处理空参数列表
                printTab(n);
                printType(node->identifierType);
                printf("\n");
            } else {
                // 处理非空参数列表
                printTab(n);
                printf("%s(",node->name);
                printType(node->identifierType);
                printf(")\n");
            }
        } else if (node->statementType == DeclareStatement) {
            // 处理声明语句
            printTab(n);
            printf("Declare statement:\n");
            // 处理声明类型
            printTab(n + 1);
            printf("Type:");
            printType(node->identifierType);
            // 处理声明变量
            if (node->identifierType == INT || node->identifierType == CHAR) {
                printf("\t\t\t\tid name:%s\n",node->name);
            } else {
                printf("\t\t\t\tid name:%s\t\t\tsize:%lld\n",node->name,node->size);
            }
        }
        // 遍历兄弟结点
        node = node->sibling;
    }
}

/* 打印制表符
 *
 * @param n 制表符个数
 * */
void printTab(int n){
    for (int i = 0; i < n; i++) {
        printf("\t|");
    }
}

/* 打印类型
 *
 * @param type 类型
 * */
void printType(int type){
    if (type == INT) {
        printf("INT");
    } else if (type == CHAR) {
        printf("CHAR");
    } else if (type == VOID) {
        printf("VOID");
    } else if (type == INTARRAY) {
        printf("INT  ARRAY");
    } else if (type == CHARARRAY) {
        printf("CHAR ARRAY");
    }

}

/* 打印运算符
 *
 * @param op 运算符
 * */
void printOperator(int op){
    switch (op) {
        case Assign:
            printf("assign(=)");
            break;
        case Or:
            printf("or(|)");
            break;
        case Xor:
            printf("xor(^)");
            break;
        case And:
            printf("and(&)");
            break;
        case Lt:
            printf("less than(<)");
            break;
        case Gt:
            printf("greater than(>)");
            break;
        case Add:
            printf("add(+)");
            break;
        case Sub:
            printf("sub(-)");
            break;
        case Mul:
            printf("mul(*)");
            break;
        case Div:
            printf("div(/)");
            break;
        case Mod:
            printf("mod(%%)");
            break;
        case Bracket:
            printf("bracket([])");
            break;
        case '!':
            printf("Not(!)");
            break;
        case ']':
        case '(':
        case ')':
        case '{':
        case '}':
        case ',':
        case ';':
            printf("%c",op);
            break;
        case Lor:
            printf("lor(||)");
            break;
        case Land:
            printf("land(&&)");
            break;
        case Eq:
            printf("equal(==)");
            break;
        case Ne:
            printf("not equal(!=)");
            break;
        case Le:
            printf("le(<=)");
            break;
        case Ge:
            printf("ge(>=)");
            break;
        case Shl:
            printf("shl(<<)");
            break;
        case Shr:
            printf("shr(>>)");
            break;
        case Inc:
            printf("inc(++)");
            break;
        case Dec:
            printf("dec(--)");
            break;
        default:
            break;
    }
}

/* 打印错误信息
 *
 * @param error 错误信息
 * @param message 具体内容
 * */
void printErrorInformation(char* error, const char* message){
    if (message == NULL){
        fprintf(stderr,"line %d: %s !\n",line, error);
    } else {
        fprintf(stderr,"line %d: %s: %s !\n",line, error, message);
    }
}