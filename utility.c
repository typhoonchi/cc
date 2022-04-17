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
    if (symbolTable == NULL) {
        return false;
    }
    memset(symbolTable, 0, MAXSIZE * sizeof(struct symbol));
    symbolPtr = symbolTable;
    return true;
}

void printToken(int lineNo, const struct symbol* ptr) {
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
        case Not:
            printf("\t%-3d: not                   !\n",lineNo);
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
        case Lbracket:
            printf("\t%-3d:                      [\n",lineNo);
            break;
        case Rbracket:
            printf("\t%-3d:                      ]\n",lineNo);
            break;
        case Lbrace:
            printf("\t%-3d:                      {\n",lineNo);
            break;
        case Rbrace:
            printf("\t%-3d:                      }\n",lineNo);
            break;
        case Lparenthesis:
            printf("\t%-3d:                      (\n",lineNo);
            break;
        case Rparenthesis:
            printf("\t%-3d:                      )\n",lineNo);
            break;
        case Comma:
            printf("\t%-3d:                      ,\n",lineNo);
            break;
        case Semicolon:
            printf("\t%-3d:                      ;\n",lineNo);
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
//    if (ptr->token == Id) {
//        printf("\t%-3d: Id             --->   %s\n",lineNo, ptr->name);
//    } else if (ptr->token == String) {
//        printf("\t%-3d: String         --->   %s\n",lineNo, ptr->name);
//    } else if (ptr->token == Char) {
//        if (ptr->value == '\n') {
//            printf("\t%-3d: Char           --->   \\n\n",lineNo);
//        } else {
//            printf("\t%-3d: Char           --->   %c\n",lineNo, (char)ptr->value);
//        }
//    } else if (ptr->token == Num) {
//        printf("\t%-3d: Num            --->   %lld\n",lineNo, ptr->value);
//    } else if (ptr->token >= CHAR && ptr->token <= VOID) {
//        printf("\t%-3d: reserved word  --->   %s\n",lineNo, ptr->name);
//    } else if (ptr->token >= Assign && ptr->token <= Semicolon) {
//        printf("\t%-3d:                       %c\n",lineNo, (char)ptr->value);
//    } else if (ptr->token >= Lor && ptr->token <= Dec) {
//        printf("\t%-3d:                       %s\n",lineNo,ptr->name);
//    }
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
