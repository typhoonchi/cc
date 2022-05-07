//
// Created by zhangyukun on 2022/4/11.
//

#include "globals.h"
#include "utility.h"


bool loadSrc(const char* fileName){
    // 存储源代码长度
    long size = 0;
    // 打开源代码文件
    filePtr = fopen(fileName,"r");
    if (filePtr == NULL) {
        fprintf(stderr,"Could not open source code: \"%s\"\n",fileName);
        return false;
    }
    // 分配缓冲区读取源代码
    source = (char*)malloc(MAXSIZE);
    if (source == NULL) {
        fprintf(stderr,"Could not malloc (%d) for source code",MAXSIZE);
        return false;
    }
    // 移动指针到文件末尾 , 获取文件长度
    fseek(filePtr,0,SEEK_END);
    size = ftell(filePtr);
    // 恢复指针到文件首
    fseek(filePtr,0,SEEK_SET);
    // 读取源代码到文件缓冲区
    if (fread(source,sizeof(char),size,filePtr) == 0) {
        fprintf(stderr,"Could not read source code: \"%s\"\n",fileName);
        return false;
    }
    sourcePtr = sourceDump = source;
    // 关闭源代码文件
    fclose(filePtr);
    return true;
}

bool init(void) {
    symbolTable = (struct symbol*)malloc(MAXSIZE * sizeof(struct symbol));
    symbolStack = (struct treeNode**) malloc(STACKSIZE * sizeof(struct treeNode*));
    top = 0;
    if (symbolTable == NULL || symbolStack == NULL) {
        return false;
    }
    memset(symbolTable, 0, MAXSIZE * sizeof(struct symbol));
    symbolPtr = symbolTable;

    return true;
}

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

struct treeNode* createNode(int statementType, int expressionType, int operatorType){
    struct treeNode* node = (struct treeNode*) malloc(sizeof(struct treeNode));
    if (node == NULL) {
        exit(1);
    }
    node->statementType = statementType;
    for (int i = 0; i < MAXCHILDREN; i++) {
        node->children[i] = NULL;
    }
    node->sibling = NULL;
    node->size = 1;
    node->isArray = false;
    node->expressionType = expressionType;
    node->operatorType = operatorType;
    return node;
}

void printTree(struct treeNode* node, int n){
    struct treeNode* temp;
    while (node != NULL) {
        if (node->statementType == Function) {
            printTab(n);
            printf("Function name:%s\ttype:",node->name);
            printType(node->identifierType);
            printf("\n");

            printTab(n + 1);
            printf("Parameters:\n");
            printTree(node->children[0], n + 2);
            printTab(n + 1);
            printf("Function Body:\n");
            printTree(node->children[1], n + 2);
        } else if (node->statementType == IfStatement) {
            printTab(n);
            printf("IF statement:\n");
            printTab(n);
            printf("Conditions:\n");
            printTree(node->children[0],n + 1);
            printTab(n);
            printf("Success condition:\n");
            printTree(node->children[1], n + 1);
            if (node->children[2] != NULL) {
                printTab(n);
                printf("Failure condition:\n");
                printTree(node->children[2], n + 1);
            }
        } else if (node->statementType == WhileStatement) {
            printTab(n);
            printf("While statement:\n");
            printTab(n + 1);
            printf("Conditions:\n");
            printTree(node->children[0],n + 2);
            printTab(n + 1);
            printf("While Body:\n");
            printTree(node->children[1], n + 2);
        } else if (node->statementType == ExpressStatement) {
            printTab(n);
            if (node->expressionType == Operator) {
                printOperator(node->operatorType);
                printf("\n");
            } else if (node->expressionType == Constant) {
                if (node->identifierType == CHAR){
                    printf("Char:'%c'\n",(char)node->value);
                } else {
                    printf("Num:%lld\n",node->value);
                }
            } else if (node->expressionType == Identifier) {
                printf("Id:%s\n",node->name);
            } else if (node->expressionType == Call) {
                printf("Function call:%s\n",node->name);
            }
            printTree(node->children[0], n + 1);
            printTree(node->children[1], n + 1);
        } else if (node->statementType == ReturnStatement) {
            printTab(n);
            printf("Return statement:\n");
            printTab(n + 1);
            printf("Return value:\n");
            printTree(node->children[0], n + 2);
        } else if (node->statementType == ParameterStatement) {
            if (node->identifierType == VOID) {
                printTab(n);
                printType(node->identifierType);
                printf("\n");
            } else {
                printTab(n);
                printf("%s(",node->name);
                printType(node->identifierType);
                printf(")\n");
            }
        } else if (node->statementType == DeclareStatement) {
            printTab(n);
            printf("Declare statement:\n");
            printTab(n + 1);
            printf("Type:");
            printType(node->identifierType);
            if (node->identifierType == INT || node->identifierType == CHAR) {
                printf("\tid name:%s\n",node->name);
            } else {
                printf("\tid name:%s\tsize:%lld\n",node->name,node->size);
            }
        }
        node = node->sibling;
    }
}


void printTab(int n){
    for (int i = 0; i < n; i++) {
        printf("\t|");
    }
}

void printType(int type){
    if (type == INT) {
        printf("INT       ");
    } else if (type == CHAR) {
        printf("CHAR      ");
    } else if (type == VOID) {
        printf("VOID      ");
    } else if (type == INTARRAY) {
        printf("INT ARRAY ");
    } else if (type == CHARARRAY) {
        printf("CHAR ARRAY");
    }

}
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