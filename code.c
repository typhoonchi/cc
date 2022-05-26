//
// Created by zhangyukun on 2022/5/15.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#include "code.h"

static long long offsetBase;                // 存放局部变量偏移基准
static long long localSize;                 // 存放局部变量占用空间大小
static int type;                            // 存放表达式当前结果的类型

static void generateFunctionCode(struct treeNode *node);
static void generateIfStatementCode(struct treeNode *node);
static void generateWhileStatementCode(struct treeNode *node);
static void generateForStatementCode(struct treeNode *node);
static void generateDoWhileStatementCode(struct treeNode *node);
static void generateExpressionStatementCode(struct treeNode *node);

/**
 * @brief 代码生成.
 *
 * 根据 AST, 生成虚拟机代码.
 *
 * @param   node    AST 节点.
 * @return  void
 * */
void generateCode(struct treeNode *node){

    // 遍历非空节点.
    while (node != NULL) {
        // 生成代码.
        if (node->statementType == DeclareStatement) {
            // 不处理变量声明语句.
        } else if (node->statementType == Function) {
            // 生成函数定义代码.
            generateFunctionCode(node);
        } else if (node->statementType == ParameterStatement) {
            // 不处理参数列表.
        } else if (node->statementType == IfStatement) {
            // 生成 If 语句代码.
            generateIfStatementCode(node);
        } else if (node->statementType == WhileStatement) {
            // 生成 While 语句代码.
            generateWhileStatementCode(node);
        } else if (node->statementType == ForStatement)  {
            // 生成 For 语句代码.
            generateForStatementCode(node);
        } else if (node->statementType == DoWhileStatement) {
            // 生成 Do While 语句代码.
            generateDoWhileStatementCode(node);
        } else if (node->statementType == ExpressStatement) {
            // 生成表达式代码.
            generateExpressionStatementCode(node);
        } else if (node->statementType == ReturnStatement) {
            // 生成 Return 语句代码.
            // 生成返回值代码.
            generateExpressionStatementCode(node->children[0]);
            *codePtr = RET;
            codePtr++;
        } else {
            handleErrorInformation(0, CODE_ERROR, "code.c/generateCode()",
                                   "Get an Unknown AST node", NULL);
        }
        node = node->sibling;
    }
}

/**
 * @brief 生成函数定义代码.
 *
 * 生成函数定义代码.
 *
 * @param   node    AST 节点.
 * @return  void
 * */
void generateFunctionCode(struct treeNode *node) {
    struct treeNode *tempNode = NULL, *childNode = NULL;    // 临时节点.
    long long size = 1, scale = 1;                          // 局部变量数组大小, int_64 与数组元素大小的比值.
    int currentType = 1;                                    // 解析时变量当前的类型.
    long long base = 1, currentAddress = 1;                 // 要填写的栈基地址, 当前栈偏移地址.

    // 初始化局部变量空间大小.
    localSize = 0;
    // 回填函数在代码段中地址.
    *(long long*)(node->value) = (long long)codePtr;
    // 计算偏移基准.
    offsetBase = 1;
    tempNode = node->children[0];
    if (tempNode->identifierType != Void) {
        while (tempNode != NULL) {
            offsetBase++;
            tempNode = tempNode->sibling;
        }
    }
    // 计算局部变量占用空间大小.
    tempNode = node->children[1];
    while (tempNode->statementType == DeclareStatement) {
        // 局部变量空间大小增加 1.
        localSize++;
        // 初始化数组大小与比例值.
        size = 1;
        scale = 1;
        // 保存当前变量类型.
        currentType = tempNode->identifierType;
        // 定位到变量的孩子节点, 解析数组.
        childNode = tempNode->children[0];
        // 解析数组.
        while (childNode != NULL) {
            // 解析一层 [], 当前类型减小一个 Ptr.
            currentType -= Ptr;
            // 用于最后一层空间分配.
            if (childNode->sibling == NULL) {
                if (currentType == Int) {
                    scale = 2;
                } else if (currentType == Char) {
                    scale = 8;
                }
            }
            // 更新局部变量空间大小.
            localSize = localSize + size * ((childNode->value % scale) ? (childNode->value / scale + 1) : (childNode->value / scale));
            // 更新数组大小.
            size = size * childNode->value;
            // 继续解析数组.
            childNode = childNode->sibling;
        }
        tempNode = tempNode->sibling;
    }
    // 为局部变量分配栈空间.
    *codePtr = NVAR;
    codePtr++;
    *codePtr = localSize;
    codePtr++;
    // 初始化局部变量数组的指针关系.
    tempNode = node->children[1];
    while (tempNode->statementType == DeclareStatement) {
        // 判断是否可能是数组.
        if (tempNode->identifierType > Ptr) {
            // 初始化数组大小与比例.
            size = 1;
            scale = 1;
            // 记录数组类型.
            currentType = tempNode->identifierType;
            // 计算数组基地址指向栈偏移地址.
            currentAddress = offsetBase - tempNode->value + size;
            // 定位到孩子节点, 解析数组.
            childNode = tempNode->children[0];
            // 解析数组.
            while (childNode != NULL) {
                // 记录要填写栈基准位置.
                base = currentAddress;
                // 解析一层 [], 当前类型值减 Ptr.
                currentType -= Ptr;
                // 用于最后一层空间分配.
                if (childNode->sibling == NULL) {
                    if (currentType == Int) {
                        scale = 2;
                    } else if (currentType == Char) {
                        scale = 8;
                    }
                }
                // 填写指针地址.
                for (int i = 0; i < size; i++) {
                    *codePtr = LEA;
                    codePtr++;
                    *codePtr = base - size + i;
                    codePtr++;
                    *codePtr = PUSH;
                    codePtr++;
                    *codePtr = LEA;
                    codePtr++;
                    *codePtr = currentAddress;
                    codePtr++;
                    *codePtr = SA;
                    codePtr++;
                    // 更新偏移地址.
                    currentAddress += ((childNode->value % scale) ? (childNode->value / scale + 1) : (childNode->value / scale));
                }
                // 计算数组大小.
                size = size * childNode->value;
                // 继续解析数组.
                childNode = childNode->sibling;
            }
        }
        tempNode = tempNode->sibling;
    }
    // 生成函数体代码.
    generateCode(tempNode);
    // 处理 void 类型函数定义, 补充 RET.
    if (*(codePtr - 1) != RET) {
        *codePtr = RET;
        codePtr++;
    }
}

/**
 * @brief 生成 If 语句代码.
 *
 * 生成 If 语句代码.
 *
 * @param   node    AST 节点.
 * @return  void
 * */
static void generateIfStatementCode(struct treeNode *node) {
    long long* falseLabel = NULL;      // 记录分支失败跳转目标地址.

    // 生成条件判断表达式代码.
    generateCode(node->children[0]);
    *codePtr = JZ;
    codePtr++;
    // 记录条件判断不成立后跳转的地址.
    falseLabel = codePtr;
    codePtr++;
    // 生成成功分支代码.
    generateCode(node->children[1]);
    // 判断是否有 Else 语句.
    if (node->children[2] != NULL) {
        // 生成 Else 语句代码.
        *codePtr = JMP;
        codePtr++;
        // 回填条件判断不成立后的目标地址.
        *falseLabel = (long long)(codePtr + 1);
        // 记录条件判断成立后跳转的地址.
        falseLabel = codePtr;
        codePtr++;
        // 生成失败分支代码.
        generateCode(node->children[2]);
    }
    // 回填跳转目标地址.
    *falseLabel = (long long)codePtr;
}

/**
 * @brief 生成 While 语句代码.
 *
 * 生成 While 语句代码.
 *
 * @param   node    AST 节点.
 * @return  void
 * */
static void generateWhileStatementCode(struct treeNode *node) {
    long long* conditionLabel = NULL, *endLabel = NULL;   // 记录循环跳转地址与结束地址.

    // 记录循环跳转地址.
    conditionLabel = codePtr;
    // 生成循环条件表达式代码.
    generateCode(node->children[0]);
    *codePtr = JZ;
    codePtr++;
    // 记录循环结束地址.
    endLabel = codePtr;
    codePtr++;
    // 生成循环体代码.
    generateCode(node->children[1]);
    *codePtr = JMP;
    codePtr++;
    // 回填循环跳转地址.
    *codePtr = (long long)conditionLabel;
    codePtr++;
    // 回填循环结束地址.
    *endLabel = (long long)codePtr;
}

/**
 * @brief 生成 For 语句代码.
 *
 * 生成 For 语句代码.
 *
 * @param   node    AST 节点.
 * @return  void
 * */
static void generateForStatementCode(struct treeNode *node) {
    long long* conditionLabel = NULL, *endLabel = NULL;   // 记录循环跳转地址与结束地址.

    // 生成初始化部分代码.
    generateCode(node->children[0]);
    // 记录循环跳转地址.
    conditionLabel = codePtr;
    // 生成条件判断表达式代码.
    generateCode(node->children[1]);
    *codePtr = JZ;
    codePtr++;
    // 记录循环结束地址.
    endLabel = codePtr;
    codePtr++;
    // 生成循环体代码.
    generateCode(node->children[3]);
    // 生成更新循环条件代码.
    generateCode(node->children[2]);
    *codePtr = JMP;
    codePtr++;
    // 回填循环跳转地址.
    *codePtr = (long long)conditionLabel;
    codePtr++;
    // 回填循环结束地址.
    *endLabel = (long long)codePtr;
}

/**
 * @brief 生成 Do While 语句代码.
 *
 * 生成 Do While 语句代码.
 *
 * @param   node    AST 节点.
 * @return  void
 * */
static void generateDoWhileStatementCode(struct treeNode *node) {
    long long* conditionLabel = NULL;      // 记录循环跳转地址.

    // 记录循环跳转地址.
    conditionLabel = codePtr;
    // 生成循环体代码.
    generateCode(node->children[0]);
    // 生成条件判断表达式代码.
    generateCode(node->children[1]);
    *codePtr = JNZ;
    codePtr++;
    // 回填循环跳转地址.
    *codePtr = (long long)conditionLabel;
    codePtr++;
}

/**
 * @brief 生成表达式代码.
 *
 * 生成表达式代码.
 *
 * @param   node    AST 节点.
 * @return  void
 * */
void generateExpressionStatementCode(struct treeNode *node) {
    long long* point = NULL;                    // 记录逻辑与, 逻辑或的短路跳转地址.
    long long num = 0;                          // 记录函数调用的参数.
    int tempType = 0, record = 0;               // 记录数据类型与指令.
    struct treeNode* tempNode = NULL;           // 临时节点.

    // 判断节点是否为空.
    if (node == NULL) {
        // 空节点, 直接返回.
        return;
    }
    // 处理表达式不同部分.
    if (node->expressionType == Constant) {
        // 处理常量.
        if (node->identifierType == Int) {
            // 处理 Int 常量.
            *codePtr = IMM;
            codePtr++;
            *codePtr = node->value;
            codePtr++;
            type = Int;
        } else if (node->identifierType == Char) {
            // 处理 Char 常量.
            *codePtr = IMM;
            codePtr++;
            *codePtr = node->value;
            codePtr++;
            type = Char;
        } else if (node->identifierType == Char + Ptr) {
            // 处理字符串常量.
            *codePtr = IMM;
            codePtr++;
            *codePtr = node->value;
            codePtr++;
            type = Char + Ptr;
        }
    } else if (node->expressionType == Variable) {
        // 处理变量.
        // 判断是全局变量还是局部变量.
        if (node->classType == Glo) {
            // 处理全局变量.
            *codePtr = IMM;
            codePtr++;
            *codePtr = node->value;
            codePtr++;
        } else {
            // 处理局部变量.
            *codePtr = LEA;
            codePtr++;
            *codePtr = offsetBase - node->value;
            codePtr++;
        }
        type = node->identifierType;
        // 载入变量.
        if (node->identifierType == Int) {
            *codePtr = LI;
        } else if (node->identifierType == Char) {
            *codePtr = LC;
        } else {
            *codePtr = LA;
        }
        codePtr++;
    } else if (node->expressionType == Call) {
        // 处理函数调用.
        tempNode = node->children[0];
        num = 0;
        // 处理参数列表.
        while (tempNode != NULL) {
            // 处理参数表达式.
            generateExpressionStatementCode(tempNode);
            *codePtr = PUSH;
            codePtr++;
            num++;
            tempNode = tempNode->sibling;
        }
        // 判断是系统调用还是自定义函数调用.
        if (node->classType == Sys) {
            // 处理系统调用
            *codePtr = *((long long*)(node->value));
            codePtr++;
        } else {
            // 处理自定义函数调用.
            *codePtr = CALL;
            codePtr++;
            *codePtr = *((long long*)(node->value));
            codePtr++;
        }
        // 回收参数空间.
        if (num > 0) {
            *codePtr = DARG;
            codePtr++;
            *codePtr = num;
            codePtr++;
        }
        type = node->identifierType;
    } else if (node->expressionType == Operator) {
        // 处理运算符.
        if (node->operatorType == '!') {
            // 处理单目运算符 !.
            *codePtr = PUSH;
            codePtr++;
            *codePtr = IMM;
            codePtr++;
            *codePtr = 0;
            codePtr++;
            *codePtr = EQ;
            codePtr++;
            type = Int;
        } else if (node->operatorType == Assign) {
            // 处理赋值运算符 =.
            // 生成左侧表达式代码.
            generateExpressionStatementCode(node->children[0]);
            tempType = type;
            // 把装入变压栈.
            if ((*(codePtr - 1) == LI) || (*(codePtr - 1) == LC) || (*(codePtr - 1) == LA)) {
                record = (int)(*(codePtr - 1) + SI - LI);
                *(codePtr - 1) = PUSH;
            } else {
                exit(1);
            }
            // 生成右侧表达式代码.
            generateExpressionStatementCode(node->children[1]);
            type = tempType;
            // 写回数据.
            *codePtr = record;
            codePtr++;
        } else if (node->operatorType == Lor) {
            // 处理逻辑或 ||.
            // 生成左侧表达式代码.
            generateExpressionStatementCode(node->children[0]);
            // 短路处理, 左侧表达式为真, 直接跳转到后方语句.
            *codePtr = JNZ;
            codePtr++;
            point = codePtr;
            codePtr++;
            // 生成右侧表达式代码.
            generateExpressionStatementCode(node->children[1]);
            type = Int;
            // 回填跳转地址.
            *point = (long long)codePtr;
        } else if (node->operatorType == Land) {
            // 处理逻辑与 &&.
            // 生成左侧表达式代码.
            generateExpressionStatementCode(node->children[0]);
            // 短路处理, 左侧表达式为假, 直接跳转到后方语句.
            *codePtr = JZ;
            codePtr++;
            point = codePtr;
            codePtr++;
            // 生成右侧表达式代码.
            generateExpressionStatementCode(node->children[1]);
            type = Int;
            // 回填跳转地址.
            *point = (long long)codePtr;
        } else if (node->operatorType == Or) {
            // 处理按位或 |.
            // 生成左侧表达式代码.
            generateExpressionStatementCode(node->children[0]);
            tempType = type;
            *codePtr = PUSH;
            codePtr++;
            // 生成右侧表达式代码.
            generateExpressionStatementCode(node->children[1]);
            type = tempType;
            *codePtr = OR;
            codePtr++;
        } else if (node->operatorType == Xor) {
            // 处理异或 ^.
            // 生成左侧表达式代码.
            generateExpressionStatementCode(node->children[0]);
            tempType = type;
            *codePtr = PUSH;
            codePtr++;
            // 生成右侧表达式代码.
            generateExpressionStatementCode(node->children[1]);
            type = tempType;
            *codePtr = XOR;
            codePtr++;
        } else if (node->operatorType == And) {
            // 处理按位与 &.
            // 生成左侧表达式代码.
            generateExpressionStatementCode(node->children[0]);
            tempType = type;
            *codePtr = PUSH;
            codePtr++;
            // 生成右侧表达式代码.
            generateExpressionStatementCode(node->children[1]);
            type = tempType;
            *codePtr = AND;
            codePtr++;
        } else if (node->operatorType == Eq) {
            // 处理等于 ==.
            // 生成左侧表达式代码.
            generateExpressionStatementCode(node->children[0]);
            *codePtr = PUSH;
            codePtr++;
            // 生成右侧表达式代码.
            generateExpressionStatementCode(node->children[1]);
            type = Int;
            *codePtr = EQ;
            codePtr++;
        } else if (node->operatorType == Ne) {
            // 处理不等于 !+.
            // 生成左侧表达式代码.
            generateExpressionStatementCode(node->children[0]);
            *codePtr = PUSH;
            codePtr++;
            // 生成右侧表达式代码.
            generateExpressionStatementCode(node->children[1]);
            type = Int;
            *codePtr = NE;
            codePtr++;
        } else if (node->operatorType == Lt) {
            // 处理小于 <.
            // 生成左侧表达式代码.
            generateExpressionStatementCode(node->children[0]);
            *codePtr = PUSH;
            codePtr++;
            // 生成右侧表达式代码.
            generateExpressionStatementCode(node->children[1]);
            type = Int;
            *codePtr = LT;
            codePtr++;
        } else if (node->operatorType == Gt) {
            // 处理大于 >.
            // 生成左侧表达式代码.
            generateExpressionStatementCode(node->children[0]);
            *codePtr = PUSH;
            codePtr++;
            // 生成右侧表达式代码.
            generateExpressionStatementCode(node->children[1]);
            type = Int;
            *codePtr = GT;
            codePtr++;
        } else if (node->operatorType == Le) {
            // 处理小于等于 <=.
            // 生成左侧表达式代码.
            generateExpressionStatementCode(node->children[0]);
            *codePtr = PUSH;
            codePtr++;
            // 生成右侧表达式代码.
            generateExpressionStatementCode(node->children[1]);
            type = Int;
            *codePtr = LE;
            codePtr++;
        } else if (node->operatorType == Ge) {
            // 处理大于等于 >=.
            // 生成左侧表达式代码.
            generateExpressionStatementCode(node->children[0]);
            *codePtr = PUSH;
            codePtr++;
            // 生成右侧表达式代码.
            generateExpressionStatementCode(node->children[1]);
            type = Int;
            *codePtr = GE;
            codePtr++;
        } else if (node->operatorType == Shl) {
            // 处理左移 <<.
            // 生成左侧表达式代码.
            generateExpressionStatementCode(node->children[0]);
            tempType = type;
            *codePtr = PUSH;
            codePtr++;
            // 生成右侧表达式代码.
            generateExpressionStatementCode(node->children[1]);
            type = tempType;
            *codePtr = SHL;
            codePtr++;
        } else if (node->operatorType == Shr) {
            // 处理右移 >>.
            // 生成左侧表达式代码.
            generateExpressionStatementCode(node->children[0]);
            tempType = type;
            *codePtr = PUSH;
            codePtr++;
            // 生成右侧表达式代码.
            generateExpressionStatementCode(node->children[1]);
            type = tempType;
            *codePtr = SHR;
            codePtr++;
        } else if (node->operatorType == Add) {
            // 处理加 +.
            // 生成左侧表达式代码.
            generateExpressionStatementCode(node->children[0]);
            tempType = type;
            *codePtr = PUSH;
            codePtr++;
            // 生成右侧表达式代码.
            generateExpressionStatementCode(node->children[1]);
            type = tempType;
            // 判断是否是指针加.
            if (type > Ptr) {
                // 判断是指针指针加还是 Int 指针加还是 Char 指针加.
                if (type == Int + Ptr) {
                    // Int 指针加.
                    *codePtr = PUSH;
                    codePtr++;
                    *codePtr = IMM;
                    codePtr++;
                    *codePtr = 4;
                    codePtr++;
                    *codePtr = MUL;
                    codePtr++;
                } else if (type == Char + Ptr) {
                    // Char 指针加.
                    *codePtr = PUSH;
                    codePtr++;
                    *codePtr = IMM;
                    codePtr++;
                    *codePtr = 1;
                    codePtr++;
                    *codePtr = MUL;
                    codePtr++;
                } else {
                    // Ptr 指针加.
                    *codePtr = PUSH;
                    codePtr++;
                    *codePtr = IMM;
                    codePtr++;
                    *codePtr = 8;
                    codePtr++;
                    *codePtr = MUL;
                    codePtr++;
                }
            }
            *codePtr = ADD;
            codePtr++;
        } else if (node->operatorType == Sub) {
            // 处理逻减 -.
            // 生成左侧表达式代码.
            generateExpressionStatementCode(node->children[0]);
            tempType = type;
            *codePtr = PUSH;
            codePtr++;
            // 生成右侧表达式代码.
            generateExpressionStatementCode(node->children[1]);
            type = tempType;
            // 判断是否是指针减.
            if (type > Ptr) {
                // 判断是指针指针减还是 Int 指针减还是 Char 指针减.
                if (type == Int + Ptr) {
                    // Int 指针减.
                    *codePtr = PUSH;
                    codePtr++;
                    *codePtr = IMM;
                    codePtr++;
                    *codePtr = 4;
                    codePtr++;
                    *codePtr = MUL;
                    codePtr++;
                } else if (type == Char + Ptr) {
                    // Char 指针减.
                    *codePtr = PUSH;
                    codePtr++;
                    *codePtr = IMM;
                    codePtr++;
                    *codePtr = 1;
                    codePtr++;
                    *codePtr = MUL;
                    codePtr++;
                } else {
                    // Ptr 指针减.
                    *codePtr = PUSH;
                    codePtr++;
                    *codePtr = IMM;
                    codePtr++;
                    *codePtr = 8;
                    codePtr++;
                    *codePtr = MUL;
                    codePtr++;
                }
            }
            *codePtr = SUB;
            codePtr++;
        } else if (node->operatorType == Mul) {
            // 处理乘 *.
            // 生成左侧表达式代码.
            generateExpressionStatementCode(node->children[0]);
            tempType = type;
            *codePtr = PUSH;
            codePtr++;
            // 生成右侧表达式代码.
            generateExpressionStatementCode(node->children[1]);
            type = tempType;
            *codePtr = MUL;
            codePtr++;
        } else if (node->operatorType == Div) {
            // 处理除 /.
            // 生成左侧表达式代码.
            generateExpressionStatementCode(node->children[0]);
            tempType = type;
            *codePtr = PUSH;
            codePtr++;
            // 生成右侧表达式代码.
            generateExpressionStatementCode(node->children[1]);
            type = tempType;
            *codePtr = DIV;
            codePtr++;
        } else if (node->operatorType == Mod) {
            // 处理取模 %.
            // 生成左侧表达式代码.
            generateExpressionStatementCode(node->children[0]);
            tempType = type;
            *codePtr = PUSH;
            codePtr++;
            // 生成右侧表达式代码.
            generateExpressionStatementCode(node->children[1]);
            type = tempType;
            *codePtr = MOD;
            codePtr++;
        } else if (node->operatorType == Bracket) {
            // 处理下标 [].
            // 生成左侧表达式代码.
            generateExpressionStatementCode(node->children[0]);
            *codePtr = PUSH;
            codePtr++;
            tempType = type;
            // 生成右侧表达式代码.
            generateExpressionStatementCode(node->children[1]);
            type = tempType;
            type = type - Ptr;
            // 判断取下标后是什么类型.
            if (type > Ptr) {
                // 指针类型.
                *codePtr = PUSH;
                codePtr++;
                *codePtr = IMM;
                codePtr++;
                *codePtr = 8;
                codePtr++;
                *codePtr = MUL;
                codePtr++;
                *codePtr = ADD;
                codePtr++;
                *codePtr = LA;
                codePtr++;
            } else {
                if (type == Int) {
                    // Int 类型.
                    *codePtr = PUSH;
                    codePtr++;
                    *codePtr = IMM;
                    codePtr++;
                    *codePtr = 4;
                    codePtr++;
                    *codePtr = MUL;
                    codePtr++;
                    *codePtr = ADD;
                    codePtr++;
                    *codePtr = LI;
                    codePtr++;
                } else if (type == Char) {
                    // Char 类型.
                    *codePtr = PUSH;
                    codePtr++;
                    *codePtr = IMM;
                    codePtr++;
                    *codePtr = 1;
                    codePtr++;
                    *codePtr = MUL;
                    codePtr++;
                    *codePtr = ADD;
                    codePtr++;
                    *codePtr = LC;
                    codePtr++;
                }
            }
        }
    } else {
        // 遇到位置表达式节点, 打印错误信息.
        handleErrorInformation(0, CODE_ERROR, "code,c/generateExpressionStatementCode()",
                               "Get an Unknown Expression AST Node", NULL);
    }
}

/**
 * @brief 运行虚拟机代码.
 *
 * 运行虚拟机代码.
 *
 * @param   void
 * @return  void
 * */
void runCode(){
    long long op = 0;               // 记录指令.
    long long* temp = NULL;         // 辅助指针.
    long long *pc, *sp, *bp;        // pc 指针, sp 指针, bp 指针.
    long long ax;                   // ax寄存器.

    // 初始化 sp 指针与 bp指针.
    bp = sp = (long long*)((long long)stack + MAX_SIZE);
    // 初始化 ax.
    ax = 0;
    // 获取 main 函数入口指令.
    pc = (long long*)(*mainPtr);
    // 初始化栈空间内容.
    *sp = EXIT;
    sp--;
    *sp = PUSH;
    temp = sp;
    sp--;
    *sp = (long long)temp;
    // 判断 main 函数入口是否存在.
    if (pc == NULL) {
        // 未找到 main 函数入口, 打印错误信息.
        handleErrorInformation(0, RUNNING_ERROR, "code.c/runCode()",
                               "Could not Find main Function", NULL);
    }
    // 执行指令.
    while (true) {
        // 取指令
        op = *pc;
        // pc 自增
        pc++;
        // 判断指令类型并执行相应操作
        if (op == IMM) {
            // 装入立即数.
            // 装入立即数到 ax.
            ax = *pc;
            pc++;
        } else if (op == LEA) {
            // 装入有效地址
            // 装入局部变量或参数的偏移地址.
            ax = (long long)(bp + *pc);
            pc++;
        } else if (op == JMP) {
            // 直接跳转.
            // pc装入跳转地址
            pc = (long long*)*pc;
        } else if (op == JZ) {
            // 条件不成立时跳转.
            // 判断 ax 中值是否为0.
            if (ax == 0) {
                // ax 为 0, pc 装入跳转地址.
                pc = (long long*)*pc;
            } else {
                // ax 不为 0, PC自增, 不跳转.
                pc++;
            }
        } else if (op == JNZ) {
            // 条件成立时跳转.
            // 判断 ax 中值是否为 0.
            if (ax != 0) {
                // ax 不为 0, pc 装入跳转地址.
                pc = (long long*)*pc;
            } else {
                // ax 为0, pc 自增, 不跳转.
                pc++;
            }
        } else if (op == CALL) {
            // 函数调用.
            // 栈顶下移.
            sp--;
            // 保存函数调用后下一条指令地址.
            *sp = (long long)(pc + 1);
            // pc 装入调用函数入口地址.
            pc = (long long*)*pc;
        } else if (op == NVAR) {
            // 为函数调用分配局部变量空间.
            // 栈顶下移.
            sp--;
            // 存放调用者的栈基地址.
            *sp = (long long)bp;
            // 更新函数调用的栈基地址为当前地址.
            bp = sp;
            // sp 下移, 作为局部变量的空间.
            sp = sp - *pc;
            pc++;
        } else if (op == DARG) {
            // 回收参数的空间.
            // sp 上移, 回收参数空间.
            sp = sp + *pc;
            pc++;
        } else if (op == RET) {
            // 函数调用调用返回.
            // 栈顶回到栈基处.
            sp = bp;
            // 恢复调用者的栈基.
            bp = (long long*)*sp;
            sp++;
            // 装入函数调用后执行的指令地址.
            pc = (long long*)*sp;
            sp++;
        } else if (op == LA) {
            // 装入地址.
            ax = *(long long*)ax;
        } else if (op == LI) {
            // 装入 Int 整数.
            ax = *(int*)ax;
        }  else if (op == LC) {
            // 装入字符.
            ax = *(unsigned char*)ax;
        } else if (op == SA) {
            // 存储地址.
            *(long long*)*sp = ax;
            sp++;
        } else if (op == SI) {
            // 存储 Int 整数.
            *(int*)*sp = (int)ax;
            sp++;
        } else if (op == SC) {
            // 存储字符.
            *(unsigned char*)*sp = (unsigned char)ax;
            sp++;
        } else if (op == PUSH) {
            // 压栈.
            sp--;
            *sp = ax;
        } else if (op == OR) {
            // 处理按位或 |.
            ax = *sp | ax;
            sp++;
        } else if (op == XOR) {
            // 处理异或 ^.
            ax = *sp ^ ax;
            sp++;
        } else if (op == AND) {
            // 处理按位与 &.
            ax = *sp & ax;
            sp++;
        } else if (op == EQ) {
            // 处理等于 ==.
            ax = *sp == ax;
            sp++;
        } else if (op == NE) {
            // 处理不等于 !=.
            ax = *sp != ax;
            sp++;
        } else if (op == LT) {
            // 处理小于 <.
            ax = *sp < ax;
            sp++;
        } else if (op == LE) {
            // 处理小于等于 <=.
            ax = *sp <= ax;
            sp++;
        } else if (op == GT) {
            // 处理大于 >.
            ax = *sp > ax;
            sp++;
        } else if (op == GE) {
            // 处理大于等于 >=.
            ax = *sp >= ax;
            sp++;
        } else if (op == SHL) {
            // 处理左移 <<.
            ax = *sp << ax;
            sp++;
        } else if (op == SHR) {
            // 处理右移 >>.
            ax = *sp >> ax;
            sp++;
        } else if (op == ADD) {
            // 处理加 +.
            ax = *sp + ax;
            sp++;
        } else if (op == SUB) {
            // 处理减 -.
            ax = *sp - ax;
            sp++;
        } else if (op == MUL) {
            // 处理乘 *.
            ax = *sp * ax;
            sp++;
        } else if (op == DIV) {
            // 处理除 /.
            ax = *sp / ax;
            sp++;
        } else if (op == MOD) {
            // 处理取模 %.
            ax = *sp % ax;
            sp++;
        } else if (op == PRINTF) {
            // 处理打印操作.
            //
            temp = sp + pc[1] - 1;
            ax = fprintf(codeRunnerOutputStream,(char*)temp[0], temp[-1], temp[-2], temp[-3], temp[-4], temp[-5]);
        } else if (op == EXIT) {
            // 程序结束, 退出.
            return;
        } else {
            // 位置指令, 打印错误信息
            handleErrorInformation(0, RUNNING_ERROR, "code.c/runCode()",
                                   "Get an Unknown Instruction", NULL);
            exit(1);
        }
    }
}