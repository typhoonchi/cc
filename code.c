//
// Created by zhangyukun on 2022/5/15.
//

#include "code.h"
/*
 *
 * */
static int argNum;
static int type;

void generateCode(struct treeNode* node){
    long long* points[2];
    struct treeNode* parameterNode;
    int localNum;
    localNum = 0;
    while (node != NULL) {
        if (node->statementType == IfStatement) {
            generateCode(node->children[0]);
            *++code = JZ;
            points[0] = ++code;
            generateCode(node->children[1]);
            if (node->children[2] != NULL) {
                *points[0] = (long long)(code + 3);
                *++code = JMP;
                points[0] = ++code;
                generateCode(node->children[2]);
            }
            *points[0] = (long long)(code + 1);
        } else if (node->statementType == WhileStatement) {
            points[0] = (code + 1);
            generateCode(node->children[0]);
            *++code = JZ;
            points[1] = ++code;
            generateCode(node->children[1]);
            *++code = JMP;
            *++code = (long long)points[0];
            *points[1] = (long long)(code + 1);
        } else if (node->statementType == ExpressStatement) {
            generateExpressionStatementCode(node, argNum + 1);
        } else if (node->statementType == ReturnStatement) {
            generateExpressionStatementCode(node->children[0], argNum + 1);
            *++code = RET;
        } else if (node->statementType == Function) {
            generateFunctionCode(node);
        } else if (node->statementType == ParameterStatement) {
            // skip
        } else {
            // skip
        }
        node = node->sibling;
    }
}


void generateFunctionCode(struct treeNode* node) {
    int localNum = 0;
    struct treeNode* tempNode;
    *(long long*)(node->value) = (long long)(code + 1);
    tempNode = node->children[0];
    if (tempNode->identifierType != VOID) {
        argNum = 0;
        while (tempNode != NULL) {
            argNum++;
            tempNode = tempNode->sibling;
        }
    }
    tempNode = node->children[1];
    while (tempNode->statementType == DeclareStatement) {
        localNum++;
        tempNode = tempNode->sibling;
    }
    *++code = NVAR;
    *++code = localNum;
    generateCode(tempNode);
    if (*code != RET) {
        *++code = RET;
    }
}
/*
 *
 * */
void generateExpressionStatementCode(struct treeNode* node, int offset) {
    long long* point, num;
    int tempType;
    struct treeNode* tempNode;
    if (node == NULL) {
        return;
    }
    if (node->expressionType == Constant) {
        if (node->identifierType == Int) {
            *++code = IMM;
            *++code = node->value;
            type = Int;
        } else if (node->identifierType == Char) {
            *++code = IMM;
            *++code = node->value | 0x7F7F7F7F7F7F7F00;
            type = Int;
        } else if (node->identifierType == Char + Ptr) {
            *++code = IMM;
            *++code = node->value;
            type = Char + Ptr;
        }
    } else if (node->expressionType == Identifier) {
        if (node->classType == Glo) {
            *++code = IMM;
            *++code = node->value;
        } else {
            *++code = LEA;
            *++code = offset - node->value;
        }
        type = node->identifierType;
        *++code = LI;
    } else if (node->expressionType == Operator) {
        if (node->operatorType == '!') {
            *++code = PUSH;
            *++code = IMM;
            *++code = 0;
            *++code = EQ;
            type = Int;
        } else if (node->operatorType == Assign) {
            generateExpressionStatementCode(node->children[0], offset);
            if (*code == LI) {
                *code = PUSH;
            }
            generateExpressionStatementCode(node->children[1], offset);
            *++code = SI;
        } else if (node->operatorType == Lor) {
            generateExpressionStatementCode(node->children[0], offset);
            *++code = JNZ;
            point = ++code;
            generateExpressionStatementCode(node->children[1], offset);
            *point = (long long)(code + 1);
            type = Int;
        } else if (node->operatorType == Land) {
            generateExpressionStatementCode(node->children[0], offset);
            *++code = JZ;
            point = ++code;
            generateExpressionStatementCode(node->children[1], offset);
            *point = (long long)(code + 1);
            type = Int;
        } else if (node->operatorType == Or) {
            generateExpressionStatementCode(node->children[0], offset);
            *++code = PUSH;
            generateExpressionStatementCode(node->children[1], offset);
            *++code = OR;
            type = Int;
        } else if (node->operatorType == Xor) {
            generateExpressionStatementCode(node->children[0], offset);
            *++code = PUSH;
            generateExpressionStatementCode(node->children[1], offset);
            *++code = XOR;
            type = Int;
        } else if (node->operatorType == And) {
            generateExpressionStatementCode(node->children[0], offset);
            *++code = PUSH;
            generateExpressionStatementCode(node->children[1], offset);
            *++code = AND;
            type = Int;
        } else if (node->operatorType == Eq) {
            generateExpressionStatementCode(node->children[0], offset);
            *++code = PUSH;
            generateExpressionStatementCode(node->children[1], offset);
            *++code = EQ;
            type = Int;
        } else if (node->operatorType == Ne) {
            generateExpressionStatementCode(node->children[0], offset);
            *++code = PUSH;
            generateExpressionStatementCode(node->children[1], offset);
            *++code = NE;
            type = Int;
        } else if (node->operatorType == Lt) {
            generateExpressionStatementCode(node->children[0], offset);
            *++code = PUSH;
            generateExpressionStatementCode(node->children[1], offset);
            *++code = LT;
            type = Int;
        } else if (node->operatorType == Gt) {
            generateExpressionStatementCode(node->children[0], offset);
            *++code = PUSH;
            generateExpressionStatementCode(node->children[1], offset);
            *++code = GT;
            type = Int;
        } else if (node->operatorType == Le) {
            generateExpressionStatementCode(node->children[0], offset);
            *++code = PUSH;
            generateExpressionStatementCode(node->children[1], offset);
            *++code = LE;
            type = Int;
        } else if (node->operatorType == Ge) {
            generateExpressionStatementCode(node->children[0], offset);
            *++code = PUSH;
            generateExpressionStatementCode(node->children[1], offset);
            *++code = GE;
            type = Int;
        } else if (node->operatorType == Shl) {
            generateExpressionStatementCode(node->children[0], offset);
            *++code = PUSH;
            generateExpressionStatementCode(node->children[1], offset);
            *++code = SHL;
            type = Int;
        } else if (node->operatorType == Shr) {
            generateExpressionStatementCode(node->children[0], offset);
            *++code = PUSH;
            generateExpressionStatementCode(node->children[1], offset);
            *++code = SHR;
            type = Int;
        } else if (node->operatorType == Add) {
            generateExpressionStatementCode(node->children[0], offset);
            *++code = PUSH;
            generateExpressionStatementCode(node->children[1], offset);
            *++code = ADD;
            type = Int;
        } else if (node->operatorType == Sub) {
            generateExpressionStatementCode(node->children[0], offset);
            *++code = PUSH;
            generateExpressionStatementCode(node->children[1], offset);
            *++code = SUB;
            type = Int;
        } else if (node->operatorType == Mul) {
            generateExpressionStatementCode(node->children[0], offset);
            *++code = PUSH;
            generateExpressionStatementCode(node->children[1], offset);
            *++code = MUL;
            type = Int;
        } else if (node->operatorType == Div) {
            generateExpressionStatementCode(node->children[0], offset);
            *++code = PUSH;
            generateExpressionStatementCode(node->children[1], offset);
            *++code = DIV;
            type = Int;
        } else if (node->operatorType == Mod) {
            generateExpressionStatementCode(node->children[0], offset);
            *++code = PUSH;
            generateExpressionStatementCode(node->children[1], offset);
            *++code = MOD;
            type = Int;
        } else if (node->operatorType == Bracket) {
            generateExpressionStatementCode(node->children[0], offset);
            *++code = PUSH;
            tempType = type;
            generateExpressionStatementCode(node->children[1], offset);
            type = tempType;
            if (type > Ptr) {
                *++code = PUSH;
                *++code = IMM;
                *++code = 8;
                *++code = MUL;
            }
            *++code = ADD;
            *++code = LI;
            type = type - Ptr;
        }
    } else if (node->expressionType == Call) {
        tempNode = node->children[0];
        num = 0;
        while (tempNode != NULL) {
            generateExpressionStatementCode(tempNode, offset);
            *++code = PUSH;
            num++;
            tempNode = tempNode->sibling;
        }
        if (node->classType == Sys) {
            *++code = *((long long*)(node->value));
        } else {
            *++code = CALL;
            *++code = *((long long*)(node->value));
        }
        if (num > 0) {
            *++code = DARG;
            *++code = num;
        }
        type = node->identifierType;
    } else {
        return;
    }
}

void runCode(int argc, char** argv){
    long long op;
    long long* temp;
    bp = sp = (long long*)((long long)stack + MAXSIZE);
    *--sp = EXIT;
    *--sp = PUSH;
    temp = sp;
    *--sp = argc;
    *--sp = (long long)argv;
    *--sp = (long long)temp;
    pc = (long long*)(*mainPtr);
    if (pc == NULL) {
        printErrorInformation("Fail to Find main Function", NULL);
        exit(1);
    }
    cycle = 0;
    while (true) {
        cycle++;
        op = *pc++;
        if (op == IMM) {
            ax = *pc++;
        } else if (op == LEA) {
            ax = (long long)(bp + *pc++);
        } else if (op == LC) {
            ax = *(long long*)ax;
        } else if (op == LI) {
            ax = *(long long*)ax;
        } else if (op == SC) {
            *(long long*)*sp++ = ax;
        } else if (op == SI) {
            *(long long*)*sp++ = ax;
        } else if (op == PUSH) {
            *--sp = ax;
        } else if (op == JMP) {
            pc = (long long*)*pc;
        } else if (op == JZ) {
            if (ax == 0) {
                pc = (long long*)*pc;
            } else {
                pc = pc + 1;
            }
        } else if (op == JNZ) {
            if (ax != 0) {
                pc = (long long*)*pc;
            } else {
                pc = pc + 1;
            }
        } else if (op == OR) {
            ax = *sp++ | ax;
        } else if (op == XOR) {
            ax = *sp++ ^ ax;
        } else if (op == AND) {
            ax = *sp++ & ax;
        } else if (op == EQ) {
            ax = *sp++ == ax;
        } else if (op == NE) {
            ax = *sp++ != ax;
        } else if (op == LT) {
            ax = *sp++ < ax;
        } else if (op == LE) {
            ax = *sp++ <= ax;
        } else if (op == GT) {
            ax = *sp++ > ax;
        } else if (op == GE) {
            ax = *sp++ >= ax;
        } else if (op == SHL) {
            ax = *sp++ << ax;
        } else if (op == SHR) {
            ax = *sp++ >> ax;
        } else if (op == ADD) {
            ax = *sp++ + ax;
        } else if (op == SUB) {
            ax = *sp++ - ax;
        } else if (op == MUL) {
            ax = *sp++ * ax;
        } else if (op == DIV) {
            ax = *sp++ / ax;
        } else if (op == MOD) {
            ax = *sp++ % ax;
        } else if (op == CALL) {
            *--sp = (long long)(pc + 1);
            pc = (long long*)*pc;
        } else if (op == NVAR) {
            *--sp = (long long)bp;
            bp = sp;
            sp = sp - *pc++;
        } else if (op == DARG) {
            sp = sp + *pc++;
        } else if (op == RET) {
            sp = bp;
            bp = (long long*)*sp++;
            pc = (long long*)*sp++;
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