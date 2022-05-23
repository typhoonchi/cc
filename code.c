//
// Created by zhangyukun on 2022/5/15.
//

#include "code.h"

static long long argNum;
static long long localNum;
static int type;


static void generateFunctionCode(struct treeNode* node);
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
    long long* points[2];               // 地址指针数组, 用于分支和循环语句的地址回填
    struct treeNode* parameterNode;     // 参数节点

    // 遍历非空节点
    while (node != NULL) {
        // 生成代码
        if (node->statementType == IfStatement) {
            // 生成 If 语句代码
            // 生成条件判断表达式代码
            generateCode(node->children[0]);
            code++;
            *code = JZ;
            code++;
            // 记录条件判断不成立后跳转的地址
            points[0] = code;
            // 生成成功分支代码
            generateCode(node->children[1]);
            // 判断是否有 Else 语句
            if (node->children[2] != NULL) {
                code++;
                *code = JMP;
                // 回填条件判断不成立后的目标地址
                *points[0] = (long long)(code + 2);
                code++;
                // 记录条件判断成立后跳转的地址
                points[0] = code;
                // 生成失败分支代码
                generateCode(node->children[2]);
            }
            // 回填跳转目标地址
            *points[0] = (long long)(code + 1);
        } else if (node->statementType == WhileStatement) {
            // 生成 While 语句代码
            // 记录循环跳转地址
            points[0] = (code + 1);
            // 生成循环条件表达式代码
            generateCode(node->children[0]);
            code++;
            *code = JZ;
            code++;
            // 记录循环结束地址
            points[1] = code;
            // 生成循环体代码
            generateCode(node->children[1]);
            code++;
            *code = JMP;
            code++;
            // 回填循环跳转地址
            *code = (long long)points[0];
            // 回填循环结束地址
            *points[1] = (long long)(code + 1);
        } else if (node->statementType == ExpressStatement) {
            // 生成表达式代码
            generateExpressionStatementCode(node, argNum + 1);
        } else if (node->statementType == ReturnStatement) {
            // 生成 Return 语句代码
            // 生成返回值代码
            generateExpressionStatementCode(node->children[0], argNum + 1);
            code++;
            *code = RET;
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

    // 统计局部变量数量
    localNum = 0;
    // 回填函数在代码段中地址
    *(long long*)(node->value) = (long long)(code + 1);
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
//        localNum = 1;
        localNum++;
        childNode = tempNode->children[0];
        while (childNode != NULL) {
            localNum = localNum + size * childNode->value;
            size = size * childNode->value;
            childNode = childNode->children[0];
        }
        tempNode = tempNode->sibling;
    }
    // 为局部变量分配栈空间
    code++;
    *code = NVAR;
    code++;
    *code = localNum;
    // 生成函数体代码
    generateCode(tempNode);
    // 处理 void 类型函数定义, 补充 RET
    if (*code != RET) {
        code++;
        *code = RET;
    }
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
    int tempType;               //
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
            code++;
            *code = IMM;
            code++;
            *code = node->value;
            type = Int;
        } else if (node->identifierType == Char) {
            // 处理 Char 常量
            code++;
            *code = IMM;
            code++;
            *code = node->value | 0x7F7F7F7F7F7F7F00;
            type = Int;
        } else if (node->identifierType == Char + Ptr) {
            // 处理字符串常量
            code++;
            *code = IMM;
            code++;
            *code = node->value;
            type = Char + Ptr;
        }
    } else if (node->expressionType == Variable) {
        // 处理变量
        // 判断是全局变量还是局部变量
        if (node->classType == Glo) {
            // 处理全局变量
            code++;
            *code = IMM;
            code++;
            *code = node->value;
        } else {
            // 处理局部变量
            code++;
            *code = LEA;
            code++;
            *code = offset - node->value;
        }
        type = node->identifierType;
        // 载入变量
        code++;
        *code = LI;
    } else if (node->expressionType == Operator) {
        // 处理运算符
        if (node->operatorType == '!') {
            // 处理单目运算符 !
            code++;
            *code = PUSH;
            code++;
            *code = IMM;
            code++;
            *code = 0;
            code++;
            *code = EQ;
            type = Int;
        } else if (node->operatorType == Assign) {
            // 处理赋值运算符 =
            // 生成左侧表达式代码
            generateExpressionStatementCode(node->children[0], offset);
            // 把装入变压栈
            if (*code == LI) {
                *code = PUSH;
            }
            // 生成右侧表达式代码
            generateExpressionStatementCode(node->children[1], offset);
            code++;
            *code = SI;
        } else if (node->operatorType == Lor) {
            // 处理逻辑或 ||
            // 生成左侧表达式代码
            generateExpressionStatementCode(node->children[0], offset);
            // 短路处理, 左侧表达式为真, 直接跳转到后方语句
            code++;
            *code = JNZ;
            code++;
            point = code;
            // 生成右侧表达式代码
            generateExpressionStatementCode(node->children[1], offset);
            // 回填跳转地址
            *point = (long long)(code + 1);
            type = Int;
        } else if (node->operatorType == Land) {
            // 处理逻辑与 &&
            // 生成左侧表达式代码
            generateExpressionStatementCode(node->children[0], offset);
            // 短路处理, 左侧表达式为假, 直接跳转到后方语句
            code++;
            *code = JZ;
            code++;
            point = code;
            // 生成右侧表达式代码
            generateExpressionStatementCode(node->children[1], offset);
            // 回填跳转地址
            *point = (long long)(code + 1);
            type = Int;
        } else if (node->operatorType == Or) {
            // 处理按位或 |
            // 生成左侧表达式代码
            generateExpressionStatementCode(node->children[0], offset);
            code++;
            *code = PUSH;
            // 生成右侧表达式代码
            generateExpressionStatementCode(node->children[1], offset);
            code++;
            *code = OR;
            type = Int;
        } else if (node->operatorType == Xor) {
            // 处理异或 ^
            // 生成左侧表达式代码
            generateExpressionStatementCode(node->children[0], offset);
            code++;
            *code = PUSH;
            // 生成右侧表达式代码
            generateExpressionStatementCode(node->children[1], offset);
            code++;
            *code = XOR;
            type = Int;
        } else if (node->operatorType == And) {
            // 处理按位与 &
            // 生成左侧表达式代码
            generateExpressionStatementCode(node->children[0], offset);
            code++;
            *code = PUSH;
            // 生成右侧表达式代码
            generateExpressionStatementCode(node->children[1], offset);
            code++;
            *code = AND;
            type = Int;
        } else if (node->operatorType == Eq) {
            // 处理等于 ==
            // 生成左侧表达式代码
            generateExpressionStatementCode(node->children[0], offset);
            code++;
            *code = PUSH;
            // 生成右侧表达式代码
            generateExpressionStatementCode(node->children[1], offset);
            code++;
            *code = EQ;
            type = Int;
        } else if (node->operatorType == Ne) {
            // 处理不等于 !+
            // 生成左侧表达式代码
            generateExpressionStatementCode(node->children[0], offset);
            code++;
            *code = PUSH;
            // 生成右侧表达式代码
            generateExpressionStatementCode(node->children[1], offset);
            code++;
            *code = NE;
            type = Int;
        } else if (node->operatorType == Lt) {
            // 处理小于 <
            // 生成左侧表达式代码
            generateExpressionStatementCode(node->children[0], offset);
            code++;
            *code = PUSH;
            // 生成右侧表达式代码
            generateExpressionStatementCode(node->children[1], offset);
            code++;
            *code = LT;
            type = Int;
        } else if (node->operatorType == Gt) {
            // 处理大于 >
            // 生成左侧表达式代码
            generateExpressionStatementCode(node->children[0], offset);
            code++;
            *code = PUSH;
            // 生成右侧表达式代码
            generateExpressionStatementCode(node->children[1], offset);
            code++;
            *code = GT;
            type = Int;
        } else if (node->operatorType == Le) {
            // 处理小于等于 <=
            // 生成左侧表达式代码
            generateExpressionStatementCode(node->children[0], offset);
            code++;
            *code = PUSH;
            // 生成右侧表达式代码
            generateExpressionStatementCode(node->children[1], offset);
            code++;
            *code = LE;
            type = Int;
        } else if (node->operatorType == Ge) {
            // 处理大于等于 >=
            // 生成左侧表达式代码
            generateExpressionStatementCode(node->children[0], offset);
            code++;
            *code = PUSH;
            // 生成右侧表达式代码
            generateExpressionStatementCode(node->children[1], offset);
            code++;
            *code = GE;
            type = Int;
        } else if (node->operatorType == Shl) {
            // 处理左移 <<
            // 生成左侧表达式代码
            generateExpressionStatementCode(node->children[0], offset);
            code++;
            *code = PUSH;
            // 生成右侧表达式代码
            generateExpressionStatementCode(node->children[1], offset);
            code++;
            *code = SHL;
            type = Int;
        } else if (node->operatorType == Shr) {
            // 处理右移 >>
            // 生成左侧表达式代码
            generateExpressionStatementCode(node->children[0], offset);
            code++;
            *code = PUSH;
            // 生成右侧表达式代码
            generateExpressionStatementCode(node->children[1], offset);
            code++;
            *code = SHR;
            type = Int;
        } else if (node->operatorType == Add) {
            // 处理加 +
            // 生成左侧表达式代码
            generateExpressionStatementCode(node->children[0], offset);
            code++;
            *code = PUSH;
            // 生成右侧表达式代码
            generateExpressionStatementCode(node->children[1], offset);
            code++;
            *code = ADD;
            type = Int;
        } else if (node->operatorType == Sub) {
            // 处理逻减 -
            // 生成左侧表达式代码
            generateExpressionStatementCode(node->children[0], offset);
            code++;
            *code = PUSH;
            // 生成右侧表达式代码
            generateExpressionStatementCode(node->children[1], offset);
            code++;
            *code = SUB;
            type = Int;
        } else if (node->operatorType == Mul) {
            // 处理乘 *
            // 生成左侧表达式代码
            generateExpressionStatementCode(node->children[0], offset);
            code++;
            *code = PUSH;
            // 生成右侧表达式代码
            generateExpressionStatementCode(node->children[1], offset);
            code++;
            *code = MUL;
            type = Int;
        } else if (node->operatorType == Div) {
            // 处理除 /
            // 生成左侧表达式代码
            generateExpressionStatementCode(node->children[0], offset);
            code++;
            *code = PUSH;
            // 生成右侧表达式代码
            generateExpressionStatementCode(node->children[1], offset);
            code++;
            *code = DIV;
            type = Int;
        } else if (node->operatorType == Mod) {
            // 处理取模 %
            // 生成左侧表达式代码
            generateExpressionStatementCode(node->children[0], offset);
            code++;
            *code = PUSH;
            // 生成右侧表达式代码
            generateExpressionStatementCode(node->children[1], offset);
            code++;
            *code = MOD;
            type = Int;
        } else if (node->operatorType == Bracket) {
            // 处理下标 []
            // 生成左侧表达式代码
            generateExpressionStatementCode(node->children[0], offset);
            code++;
            *code = PUSH;
            tempType = type;
            // 生成右侧表达式代码
            generateExpressionStatementCode(node->children[1], offset);
            type = tempType;
            if (type > Ptr) {
                code++;
                *code = PUSH;
                code++;
                *code = IMM;
                code++;
                *code = 8;
                code++;
                *code = MUL;
            }
            code++;
            *code = ADD;
            code++;
            *code = LI;
            type = type - Ptr;
        }
    } else if (node->expressionType == Call) {
        // 处理函数调用
        tempNode = node->children[0];
        num = 0;
        // 处理参数列表
        while (tempNode != NULL) {
            // 处理参数表达式
            generateExpressionStatementCode(tempNode, offset);
            code++;
            *code = PUSH;
            num++;
            tempNode = tempNode->sibling;
        }
        if (node->classType == Sys) {
            code++;
            *code = *((long long*)(node->value));
        } else {
            code++;
            *code = CALL;
            code++;
            *code = *((long long*)(node->value));
        }
        if (num > 0) {
            code++;
            *code = DARG;
            code++;
            *code = num;
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
            ax = *(long long*)ax;
        } else if (op == LI) {
            ax = *(long long*)ax;
        } else if (op == SC) {
            *(long long*)*sp = ax;
            sp++;
        } else if (op == SI) {
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