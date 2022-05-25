//
// Created by zhangyukun on 2022/5/15.
//

#include "code.h"

static long long argNum;
static long long localNum;
static int type;


static void generateFunctionCode(struct treeNode* node);
static void generateIfStatementCode(struct treeNode* node);
static void generateWhileStatementCode(struct treeNode* node);
static void generateForStatementCode(struct treeNode* node);
static void generateDoWhileStatementCode(struct treeNode* node);
static void generateExpressionStatementCode(struct treeNode* node, long long offset);


/**
 * @brief 代码生成
 *
 * 根据 AST, 生成虚拟机代码
 *
 * @param   node    AST 节点
 * @return  void
 * */
void generateCode(struct treeNode* node){

    // 遍历非空节点
    while (node != NULL) {
        // 生成代码
        if (node->statementType == IfStatement) {
            // 生成 If 语句代码
            generateIfStatementCode(node);
        } else if (node->statementType == WhileStatement) {
            // 生成 While 语句代码
            generateWhileStatementCode(node);
        } else if (node->statementType == ForStatement)  {
            //
            generateForStatementCode(node);
        } else if (node->statementType == DoWhileStatement) {
            //
            generateDoWhileStatementCode(node);
        } else if (node->statementType == ExpressStatement) {
            // 生成表达式代码
            generateExpressionStatementCode(node, argNum + 1);
        } else if (node->statementType == ReturnStatement) {
            // 生成 Return 语句代码
            // 生成返回值代码
            generateExpressionStatementCode(node->children[0], argNum + 1);
            *code = RET;
            code++;
        } else if (node->statementType == Function) {
            // 生成函数定义代码
            generateFunctionCode(node);
        } else if (node->statementType == ParameterStatement) {
            // skip
        } else {
            // skip
        }
        node = node->sibling;
    }
}

/**
 * @brief 生成函数定义代码
 *
 * 生成函数定义代码
 *
 * @param   node    AST 节点
 * @return  void
 * */
void generateFunctionCode(struct treeNode* node) {
    struct treeNode* tempNode, *childNode;      // 临时节点
    long long size = 1;
    long long scale = 1;
    int currentType = 1;
    long long base;
    long long currentAddress;

    // 统计局部变量数量
    localNum = 0;
    // 回填函数在代码段中地址
    *(long long*)(node->value) = (long long)code;
    // 统计参数个数
    argNum = 0;
    tempNode = node->children[0];
    if (tempNode->identifierType != Void) {
        while (tempNode != NULL) {
            argNum++;
            tempNode = tempNode->sibling;
        }
    }
    // 统计局部变量个数
    tempNode = node->children[1];
    while (tempNode->statementType == DeclareStatement) {
        localNum++;
        size = 1;
        scale = 1;
        currentType = tempNode->identifierType;
        childNode = tempNode->children[0];
        while (childNode != NULL) {
            currentType -= Ptr;
            if (childNode->sibling == NULL) {
                if (currentType == Int) {
                    scale = 2;
                } else if (currentType == Char) {
                    scale = 8;
                }
            }
            localNum = localNum + size * ((childNode->value % scale) ? (childNode->value / scale + 1) : (childNode->value / scale));
            size = size * childNode->value;
            childNode = childNode->sibling;
        }
        tempNode = tempNode->sibling;
    }
    // 为局部变量分配栈空间
    *code = NVAR;
    code++;
    *code = localNum;
    code++;
    // s
    tempNode = node->children[1];
    while (tempNode->statementType == DeclareStatement) {
        if (tempNode->identifierType > Ptr) {
            size = 1;
            scale = 1;
            currentType = tempNode->identifierType;
            currentAddress = argNum + 1 - tempNode->value + size;
            childNode = tempNode->children[0];
            while (childNode != NULL) {
                base = currentAddress;
                currentType -= Ptr;
                if (childNode->sibling == NULL) {
                    if (currentType == Int) {
                        scale = 2;
                    } else if (currentType == Char) {
                        scale = 8;
                    }
                }
                for (int i = 0; i < size; i++) {
                    *code = LEA;
                    code++;
                    *code = base - size + i;
                    code++;
                    *code = PUSH;
                    code++;
                    *code = LEA;
                    code++;
                    *code = currentAddress;
                    code++;
                    *code = SA;
                    code++;
                    currentAddress += ((childNode->value % scale) ? (childNode->value / scale + 1) : (childNode->value / scale));
                }
                size = size * childNode->value;
                childNode = childNode->sibling;
            }
        }
        tempNode = tempNode->sibling;
    }
    // 生成函数体代码
    generateCode(tempNode);
    // 处理 void 类型函数定义, 补充 RET
    if (*(code - 1) != RET) {
        *code = RET;
        code++;
    }
}

static void generateIfStatementCode(struct treeNode* node) {
    long long* falseLabel;

    // 生成条件判断表达式代码
    generateCode(node->children[0]);
    *code = JZ;
    code++;
    // 记录条件判断不成立后跳转的地址
    falseLabel = code;
    code++;
    // 生成成功分支代码
    generateCode(node->children[1]);
    // 判断是否有 Else 语句
    if (node->children[2] != NULL) {
        *code = JMP;
        code++;
        // 回填条件判断不成立后的目标地址
        *falseLabel = (long long)(code + 1);
        // 记录条件判断成立后跳转的地址
        falseLabel = code;
        code++;
        // 生成失败分支代码
        generateCode(node->children[2]);
    }
    // 回填跳转目标地址
    *falseLabel = (long long)code;
}

static void generateWhileStatementCode(struct treeNode* node) {
    long long* conditionLabel, *endLabel;

    // 记录循环跳转地址
    conditionLabel = code;
    // 生成循环条件表达式代码
    generateCode(node->children[0]);
    *code = JZ;
    code++;
    // 记录循环结束地址
    endLabel = code;
    code++;
    // 生成循环体代码
    generateCode(node->children[1]);
    *code = JMP;
    code++;
    // 回填循环跳转地址
    *code = (long long)conditionLabel;
    code++;
    // 回填循环结束地址
    *endLabel = (long long)code;
}

static void generateForStatementCode(struct treeNode* node) {
    long long* conditionLabel, *endLabel;

    generateCode(node->children[0]);
    conditionLabel = code;
    generateCode(node->children[1]);
    *code = JZ;
    code++;
    endLabel = code;
    code++;
    generateCode(node->children[3]);
    generateCode(node->children[2]);
    *code = JMP;
    code++;
    *code = (long long)conditionLabel;
    code++;
    *endLabel = (long long)code;
}

static void generateDoWhileStatementCode(struct treeNode* node) {
    long long* conditionLabel;

    conditionLabel = code;
    generateCode(node->children[0]);
    generateCode(node->children[1]);
    *code = JNZ;
    code++;
    *code = (long long)conditionLabel;
    code++;
}

/**
 * @brief 生成表达式代码
 *
 * 生成表达式代码
 *
 * @param   node    AST 节点
 * @param   offset
 * @return  void
 * */
void generateExpressionStatementCode(struct treeNode* node, long long offset) {
    long long* point, num;      //
    int tempType, record;               //
    struct treeNode* tempNode;  // 临时节点

    // 判断节点是否为空
    if (node == NULL) {
        // 空节点, 直接返回
        return;
    }
    // 处理表达式不同部分
    if (node->expressionType == Constant) {
        // 处理常量
        if (node->identifierType == Int) {
            // 处理 Int 常量
            *code = IMM;
            code++;
            *code = node->value;
            code++;
            type = Int;
        } else if (node->identifierType == Char) {
            // 处理 Char 常量
            *code = IMM;
            code++;
            *code = node->value;
            code++;
            type = Char;
        } else if (node->identifierType == Char + Ptr) {
            // 处理字符串常量
            *code = IMM;
            code++;
            *code = node->value;
            code++;
            type = Char + Ptr;
        }
    } else if (node->expressionType == Variable) {
        // 处理变量
        // 判断是全局变量还是局部变量
        if (node->classType == Glo) {
            // 处理全局变量
            *code = IMM;
            code++;
            *code = node->value;
            code++;
        } else {
            // 处理局部变量
            *code = LEA;
            code++;
            *code = offset - node->value;
            code++;
        }
        type = node->identifierType;
        // 载入变量
        if (node->identifierType == Int) {
            *code = LI;
        } else if (node->identifierType == Char) {
            *code = LC;
        } else {
            *code = LA;
        }
        code++;
    } else if (node->expressionType == Operator) {
        tempType = type;
        // 处理运算符
        if (node->operatorType == '!') {
            // 处理单目运算符 !
            *code = PUSH;
            code++;
            *code = IMM;
            code++;
            *code = 0;
            code++;
            *code = EQ;
            code++;
            type = Int;
        } else if (node->operatorType == Assign) {
            // 处理赋值运算符 =
            // 生成左侧表达式代码
            generateExpressionStatementCode(node->children[0], offset);
            tempType = type;
            // 把装入变压栈
            if ((*(code - 1) == LI) || (*(code - 1) == LC) || (*(code - 1) == LA)) {
                record = (int)(*(code - 1) + SI - LI);
                *(code - 1) = PUSH;
            } else {
                exit(1);
            }
            // 生成右侧表达式代码
            generateExpressionStatementCode(node->children[1], offset);
            type = tempType;
            *code = record;
            code++;
        } else if (node->operatorType == Lor) {
            // 处理逻辑或 ||
            // 生成左侧表达式代码
            generateExpressionStatementCode(node->children[0], offset);
            // 短路处理, 左侧表达式为真, 直接跳转到后方语句
            *code = JNZ;
            code++;
            point = code;
            code++;
            // 生成右侧表达式代码
            generateExpressionStatementCode(node->children[1], offset);
            type = Int;
            // 回填跳转地址
            *point = (long long)code;
        } else if (node->operatorType == Land) {
            // 处理逻辑与 &&
            // 生成左侧表达式代码
            generateExpressionStatementCode(node->children[0], offset);
            // 短路处理, 左侧表达式为假, 直接跳转到后方语句
            *code = JZ;
            code++;
            point = code;
            code++;
            // 生成右侧表达式代码
            generateExpressionStatementCode(node->children[1], offset);
            type = Int;
            // 回填跳转地址
            *point = (long long)code;
        } else if (node->operatorType == Or) {
            // 处理按位或 |
            // 生成左侧表达式代码
            generateExpressionStatementCode(node->children[0], offset);
            tempType = type;
            *code = PUSH;
            code++;
            // 生成右侧表达式代码
            generateExpressionStatementCode(node->children[1], offset);
            type = tempType;
            *code = OR;
            code++;
        } else if (node->operatorType == Xor) {
            // 处理异或 ^
            // 生成左侧表达式代码
            generateExpressionStatementCode(node->children[0], offset);
            tempType = type;
            *code = PUSH;
            code++;
            // 生成右侧表达式代码
            generateExpressionStatementCode(node->children[1], offset);
            type = tempType;
            *code = XOR;
            code++;
        } else if (node->operatorType == And) {
            // 处理按位与 &
            // 生成左侧表达式代码
            generateExpressionStatementCode(node->children[0], offset);
            tempType = type;
            *code = PUSH;
            code++;
            // 生成右侧表达式代码
            generateExpressionStatementCode(node->children[1], offset);
            type = tempType;
            *code = AND;
            code++;
        } else if (node->operatorType == Eq) {
            // 处理等于 ==
            // 生成左侧表达式代码
            generateExpressionStatementCode(node->children[0], offset);
            *code = PUSH;
            code++;
            // 生成右侧表达式代码
            generateExpressionStatementCode(node->children[1], offset);
            type = Int;
            *code = EQ;
            code++;
        } else if (node->operatorType == Ne) {
            // 处理不等于 !+
            // 生成左侧表达式代码
            generateExpressionStatementCode(node->children[0], offset);
            *code = PUSH;
            code++;
            // 生成右侧表达式代码
            generateExpressionStatementCode(node->children[1], offset);
            type = Int;
            *code = NE;
            code++;
        } else if (node->operatorType == Lt) {
            // 处理小于 <
            // 生成左侧表达式代码
            generateExpressionStatementCode(node->children[0], offset);
            *code = PUSH;
            code++;
            // 生成右侧表达式代码
            generateExpressionStatementCode(node->children[1], offset);
            type = Int;
            *code = LT;
            code++;
        } else if (node->operatorType == Gt) {
            // 处理大于 >
            // 生成左侧表达式代码
            generateExpressionStatementCode(node->children[0], offset);
            *code = PUSH;
            code++;
            // 生成右侧表达式代码
            generateExpressionStatementCode(node->children[1], offset);
            type = Int;
            *code = GT;
            code++;
        } else if (node->operatorType == Le) {
            // 处理小于等于 <=
            // 生成左侧表达式代码
            generateExpressionStatementCode(node->children[0], offset);
            *code = PUSH;
            code++;
            // 生成右侧表达式代码
            generateExpressionStatementCode(node->children[1], offset);
            type = Int;
            *code = LE;
            code++;
        } else if (node->operatorType == Ge) {
            // 处理大于等于 >=
            // 生成左侧表达式代码
            generateExpressionStatementCode(node->children[0], offset);
            *code = PUSH;
            code++;
            // 生成右侧表达式代码
            generateExpressionStatementCode(node->children[1], offset);
            type = Int;
            *code = GE;
            code++;
        } else if (node->operatorType == Shl) {
            // 处理左移 <<
            // 生成左侧表达式代码
            generateExpressionStatementCode(node->children[0], offset);
            tempType = type;
            *code = PUSH;
            code++;
            // 生成右侧表达式代码
            generateExpressionStatementCode(node->children[1], offset);
            type = tempType;
            *code = SHL;
            code++;
        } else if (node->operatorType == Shr) {
            // 处理右移 >>
            // 生成左侧表达式代码
            generateExpressionStatementCode(node->children[0], offset);
            tempType = type;
            *code = PUSH;
            code++;
            // 生成右侧表达式代码
            generateExpressionStatementCode(node->children[1], offset);
            type = tempType;
            *code = SHR;
            code++;
        } else if (node->operatorType == Add) {
            // 处理加 +
            // 生成左侧表达式代码
            generateExpressionStatementCode(node->children[0], offset);
            tempType = type;
            *code = PUSH;
            code++;
            // 生成右侧表达式代码
            generateExpressionStatementCode(node->children[1], offset);
            type = tempType;
            if (type > Ptr) {
                if (type == Int + Ptr) {
                    *code = PUSH;
                    code++;
                    *code = IMM;
                    code++;
                    *code = 4;
                    code++;
                    *code = MUL;
                    code++;
                } else if (type == Char + Ptr) {
                    *code = PUSH;
                    code++;
                    *code = IMM;
                    code++;
                    *code = 1;
                    code++;
                    *code = MUL;
                    code++;
                } else {
                    *code = PUSH;
                    code++;
                    *code = IMM;
                    code++;
                    *code = 8;
                    code++;
                    *code = MUL;
                    code++;
                }
            }
            *code = ADD;
            code++;
        } else if (node->operatorType == Sub) {
            // 处理逻减 -
            // 生成左侧表达式代码
            generateExpressionStatementCode(node->children[0], offset);
            tempType = type;
            *code = PUSH;
            code++;
            // 生成右侧表达式代码
            generateExpressionStatementCode(node->children[1], offset);
            type = tempType;
            *code = SUB;
            code++;
        } else if (node->operatorType == Mul) {
            // 处理乘 *
            // 生成左侧表达式代码
            generateExpressionStatementCode(node->children[0], offset);
            tempType = type;
            *code = PUSH;
            code++;
            // 生成右侧表达式代码
            generateExpressionStatementCode(node->children[1], offset);
            type = tempType;
            *code = MUL;
            code++;
        } else if (node->operatorType == Div) {
            // 处理除 /
            // 生成左侧表达式代码
            generateExpressionStatementCode(node->children[0], offset);
            tempType = type;
            *code = PUSH;
            code++;
            // 生成右侧表达式代码
            generateExpressionStatementCode(node->children[1], offset);
            type = tempType;
            *code = DIV;
            code++;
        } else if (node->operatorType == Mod) {
            // 处理取模 %
            // 生成左侧表达式代码
            generateExpressionStatementCode(node->children[0], offset);
            tempType = type;
            *code = PUSH;
            code++;
            // 生成右侧表达式代码
            generateExpressionStatementCode(node->children[1], offset);
            type = tempType;
            *code = MOD;
            code++;
        } else if (node->operatorType == Bracket) {
            // 处理下标 []
            // 生成左侧表达式代码
            generateExpressionStatementCode(node->children[0], offset);
            *code = PUSH;
            code++;
            tempType = type;
            // 生成右侧表达式代码
            generateExpressionStatementCode(node->children[1], offset);
            type = tempType;
            type = type - Ptr;
            if (type > Ptr) {
                *code = PUSH;
                code++;
                *code = IMM;
                code++;
                *code = 8;
                code++;
                *code = MUL;
                code++;
                *code = ADD;
                code++;
                *code = LA;
                code++;
            } else {
                if (type == Int) {
                    *code = PUSH;
                    code++;
                    *code = IMM;
                    code++;
                    *code = 4;
                    code++;
                    *code = MUL;
                    code++;
                    *code = ADD;
                    code++;
                    *code = LI;
                    code++;
                } else if (type == Char) {
                    *code = PUSH;
                    code++;
                    *code = IMM;
                    code++;
                    *code = 1;
                    code++;
                    *code = MUL;
                    code++;
                    *code = ADD;
                    code++;
                    *code = LC;
                    code++;
                }
            }
        }
    } else if (node->expressionType == Call) {
        // 处理函数调用
        tempNode = node->children[0];
        num = 0;
        // 处理参数列表
        while (tempNode != NULL) {
            // 处理参数表达式
            generateExpressionStatementCode(tempNode, offset);
            *code = PUSH;
            code++;
            num++;
            tempNode = tempNode->sibling;
        }
        if (node->classType == Sys) {
            *code = *((long long*)(node->value));
            code++;
        } else {
            *code = CALL;
            code++;
            *code = *((long long*)(node->value));
            code++;
        }
        if (num > 0) {
            *code = DARG;
            code++;
            *code = num;
            code++;
        }
        type = node->identifierType;
    } else {
        return;
    }
}

/**
 * @brief 运行虚拟机代码
 *
 * 运行虚拟机代码
 *
 * @param   argc
 * @param   argv
 * @return  void
 * */
void runCode(int argc, char** argv){
    long long op;
    long long* temp;
    bp = sp = (long long*)((long long)stack + MAX_SIZE);
    sp--;
    *sp = EXIT;
    sp--;
    *sp = PUSH;
    temp = sp;
    sp--;
    *sp = argc;
    sp--;
    *sp = (long long)argv;
    sp--;
    *sp = (long long)temp;
    pc = (long long*)(*mainPtr);
    if (pc == NULL) {
        printErrorInformation("Fail to Find main Function", NULL);
        exit(1);
    }
    cycle = 0;
    while (true) {
        cycle++;
        op = *pc;
        pc++;
        if (op == IMM) {
            ax = *pc;
            pc++;
        } else if (op == LEA) {
            ax = (long long)(bp + *pc);
            pc++;
        } else if (op == LC) {
            ax = *(unsigned char*)ax;
        } else if (op == LI) {
            ax = *(int*)ax;
        } else if (op == LA) {
            ax = *(long long*)ax;
        } else if (op == SC) {
            *(unsigned char*)*sp = (unsigned char)ax;
            sp++;
        } else if (op == SI) {
            *(int*)*sp = (int)ax;
            sp++;
        } else if (op == SA) {
            *(long long*)*sp = ax;
            sp++;
        } else if (op == PUSH) {
            sp--;
            *sp = ax;
        } else if (op == JMP) {
            pc = (long long*)*pc;
        } else if (op == JZ) {
            if (ax == 0) {
                pc = (long long*)*pc;
            } else {
                pc++;
            }
        } else if (op == JNZ) {
            if (ax != 0) {
                pc = (long long*)*pc;
            } else {
                pc++;
            }
        } else if (op == OR) {
            ax = *sp | ax;
            sp++;
        } else if (op == XOR) {
            ax = *sp ^ ax;
            sp++;
        } else if (op == AND) {
            ax = *sp & ax;
            sp++;
        } else if (op == EQ) {
            ax = *sp == ax;
            sp++;
        } else if (op == NE) {
            ax = *sp != ax;
            sp++;
        } else if (op == LT) {
            ax = *sp < ax;
            sp++;
        } else if (op == LE) {
            ax = *sp <= ax;
            sp++;
        } else if (op == GT) {
            ax = *sp > ax;
            sp++;
        } else if (op == GE) {
            ax = *sp >= ax;
            sp++;
        } else if (op == SHL) {
            ax = *sp << ax;
            sp++;
        } else if (op == SHR) {
            ax = *sp >> ax;
            sp++;
        } else if (op == ADD) {
            ax = *sp + ax;
            sp++;
        } else if (op == SUB) {
            ax = *sp - ax;
            sp++;
        } else if (op == MUL) {
            ax = *sp * ax;
            sp++;
        } else if (op == DIV) {
            ax = *sp / ax;
            sp++;
        } else if (op == MOD) {
            ax = *sp % ax;
            sp++;
        } else if (op == CALL) {
            sp--;
            *sp = (long long)(pc + 1);
            pc = (long long*)*pc;
        } else if (op == NVAR) {
            sp--;
            *sp = (long long)bp;
            bp = sp;
            sp = sp - *pc;
            pc++;
        } else if (op == DARG) {
            sp = sp + *pc;
            pc++;
        } else if (op == RET) {
            sp = bp;
            bp = (long long*)*sp;
            sp++;
            pc = (long long*)*sp;
            sp++;
        } else if (op == EXIT) {
            printf("exit(%lld)\n",*sp);
            return;
        } else if (op == PRINTF) {
            temp = sp + pc[1] - 1;
            ax = printf((char*)temp[0], temp[-1], temp[-2], temp[-3], temp[-4], temp[-5]);
        }else {
            printErrorInformation("Get Unknown Instruction", NULL);
            exit(1);
        }
    }
}