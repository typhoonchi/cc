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
static long long* pc, * sp, * bp;           // pc 指针, sp 指针, bp 指针
static long long ax, cycle;                 // ax寄存器, 循环次数

static void generateFunctionCode(struct treeNode* node);
static void generateIfStatementCode(struct treeNode* node);
static void generateWhileStatementCode(struct treeNode* node);
static void generateForStatementCode(struct treeNode* node);
static void generateDoWhileStatementCode(struct treeNode* node);
static void generateExpressionStatementCode(struct treeNode* node);

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
        if (node->statementType == DeclareStatement) {
            // 不处理变量声明语句
        } else if (node->statementType == Function) {
            // 生成函数定义代码
            generateFunctionCode(node);
        } else if (node->statementType == ParameterStatement) {
            // 不处理参数列表
        } else if (node->statementType == IfStatement) {
            // 生成 If 语句代码
            generateIfStatementCode(node);
        } else if (node->statementType == WhileStatement) {
            // 生成 While 语句代码
            generateWhileStatementCode(node);
        } else if (node->statementType == ForStatement)  {
            // 生成 For 语句代码
            generateForStatementCode(node);
        } else if (node->statementType == DoWhileStatement) {
            // 生成 Do While 语句代码
            generateDoWhileStatementCode(node);
        } else if (node->statementType == ExpressStatement) {
            // 生成表达式代码
            generateExpressionStatementCode(node);
        } else if (node->statementType == ReturnStatement) {
            // 生成 Return 语句代码
            // 生成返回值代码
            generateExpressionStatementCode(node->children[0]);
            *code = RET;
            code++;
        } else {
            handleErrorInformation(0, CODE_ERROR, "code.c/generateCode()",
                                   "Get an Unknown AST node", NULL);
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
    struct treeNode *tempNode = NULL, *childNode = NULL;    // 临时节点
    long long size = 1, scale = 1;                          // 局部变量数组大小, int_64 与数组元素大小的比值
    int currentType = 1;                                    // 解析时变量当前的类型
    long long base = 1, currentAddress = 1;                 // 要填写的栈基地址, 当前栈偏移地址

    // 初始化局部变量空间大小
    localSize = 0;
    // 回填函数在代码段中地址
    *(long long*)(node->value) = (long long)code;
    // 计算偏移基准
    offsetBase = 1;
    tempNode = node->children[0];
    if (tempNode->identifierType != Void) {
        while (tempNode != NULL) {
            offsetBase++;
            tempNode = tempNode->sibling;
        }
    }
    // 计算局部变量占用空间大小
    tempNode = node->children[1];
    while (tempNode->statementType == DeclareStatement) {
        // 局部变量空间大小增加 1
        localSize++;
        // 初始化数组大小与比例值
        size = 1;
        scale = 1;
        // 保存当前变量类型
        currentType = tempNode->identifierType;
        // 定位到变量的孩子节点, 解析数组
        childNode = tempNode->children[0];
        // 解析数组
        while (childNode != NULL) {
            // 解析一层 [], 当前类型减小一个 Ptr
            currentType -= Ptr;
            // 用于最后一层空间分配
            if (childNode->sibling == NULL) {
                if (currentType == Int) {
                    scale = 2;
                } else if (currentType == Char) {
                    scale = 8;
                }
            }
            // 更新局部变量空间大小
            localSize = localSize + size * ((childNode->value % scale) ? (childNode->value / scale + 1) : (childNode->value / scale));
            // 更新数组大小
            size = size * childNode->value;
            // 继续解析数组
            childNode = childNode->sibling;
        }
        tempNode = tempNode->sibling;
    }
    // 为局部变量分配栈空间
    *code = NVAR;
    code++;
    *code = localSize;
    code++;
    // 初始化局部变量数组的指针关系
    tempNode = node->children[1];
    while (tempNode->statementType == DeclareStatement) {
        // 判断是否可能是数组
        if (tempNode->identifierType > Ptr) {
            // 初始化数组大小与比例
            size = 1;
            scale = 1;
            // 记录数组类型
            currentType = tempNode->identifierType;
            // 计算数组基地址指向栈偏移地址
            currentAddress = offsetBase - tempNode->value + size;
            // 定位到孩子节点, 解析数组
            childNode = tempNode->children[0];
            // 解析数组
            while (childNode != NULL) {
                // 记录要填写栈基准位置
                base = currentAddress;
                // 解析一层 [], 当前类型值减 Ptr
                currentType -= Ptr;
                // 用于最后一层空间分配
                if (childNode->sibling == NULL) {
                    if (currentType == Int) {
                        scale = 2;
                    } else if (currentType == Char) {
                        scale = 8;
                    }
                }
                // 填写指针地址
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
                    // 更新偏移地址
                    currentAddress += ((childNode->value % scale) ? (childNode->value / scale + 1) : (childNode->value / scale));
                }
                // 计算数组大小
                size = size * childNode->value;
                // 继续解析数组
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

/**
 * @brief 生成 If 语句代码
 *
 * 生成 If 语句代码
 *
 * @param   node    AST 节点
 * @return  void
 * */
static void generateIfStatementCode(struct treeNode* node) {
    long long* falseLabel = NULL;      // 记录分支失败跳转目标地址

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
        // 生成 Else 语句代码
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

/**
 * @brief 生成 While 语句代码
 *
 * 生成 While 语句代码
 *
 * @param   node    AST 节点
 * @return  void
 * */
static void generateWhileStatementCode(struct treeNode* node) {
    long long* conditionLabel = NULL, *endLabel = NULL;   // 记录循环跳转地址与结束地址

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

/**
 * @brief 生成 For 语句代码
 *
 * 生成 For 语句代码
 *
 * @param   node    AST 节点
 * @return  void
 * */
static void generateForStatementCode(struct treeNode* node) {
    long long* conditionLabel = NULL, *endLabel = NULL;   // 记录循环跳转地址与结束地址

    // 生成初始化部分代码
    generateCode(node->children[0]);
    // 记录循环跳转地址
    conditionLabel = code;
    // 生成条件判断表达式代码
    generateCode(node->children[1]);
    *code = JZ;
    code++;
    // 记录循环结束地址
    endLabel = code;
    code++;
    // 生成循环体代码
    generateCode(node->children[3]);
    // 生成更新循环条件代码
    generateCode(node->children[2]);
    *code = JMP;
    code++;
    // 回填循环跳转地址
    *code = (long long)conditionLabel;
    code++;
    // 回填循环结束地址
    *endLabel = (long long)code;
}

/**
 * @brief 生成 Do While 语句代码
 *
 * 生成 Do While 语句代码
 *
 * @param   node    AST 节点
 * @return  void
 * */
static void generateDoWhileStatementCode(struct treeNode* node) {
    long long* conditionLabel = NULL;      // 记录循环跳转地址

    // 记录循环跳转地址
    conditionLabel = code;
    // 生成循环体代码
    generateCode(node->children[0]);
    // 生成条件判断表达式代码
    generateCode(node->children[1]);
    *code = JNZ;
    code++;
    // 回填循环跳转地址
    *code = (long long)conditionLabel;
    code++;
}

/**
 * @brief 生成表达式代码
 *
 * 生成表达式代码
 *
 * @param   node    AST 节点
 * @return  void
 * */
void generateExpressionStatementCode(struct treeNode* node) {
    long long* point = NULL;                    // 记录逻辑与, 逻辑或的短路跳转地址
    long long num = 0;                          // 记录函数调用的参数
    int tempType = 0, record = 0;               // 记录数据类型与指令
    struct treeNode* tempNode = NULL;           // 临时节点

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
            *code = offsetBase - node->value;
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
    } else if (node->expressionType == Call) {
        // 处理函数调用
        tempNode = node->children[0];
        num = 0;
        // 处理参数列表
        while (tempNode != NULL) {
            // 处理参数表达式
            generateExpressionStatementCode(tempNode);
            *code = PUSH;
            code++;
            num++;
            tempNode = tempNode->sibling;
        }
        // 判断是系统调用还是自定义函数调用
        if (node->classType == Sys) {
            // 处理系统调用
            *code = *((long long*)(node->value));
            code++;
        } else {
            // 处理自定义函数调用
            *code = CALL;
            code++;
            *code = *((long long*)(node->value));
            code++;
        }
        // 回收参数空间
        if (num > 0) {
            *code = DARG;
            code++;
            *code = num;
            code++;
        }
        type = node->identifierType;
    } else if (node->expressionType == Operator) {
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
            generateExpressionStatementCode(node->children[0]);
            tempType = type;
            // 把装入变压栈
            if ((*(code - 1) == LI) || (*(code - 1) == LC) || (*(code - 1) == LA)) {
                record = (int)(*(code - 1) + SI - LI);
                *(code - 1) = PUSH;
            } else {
                exit(1);
            }
            // 生成右侧表达式代码
            generateExpressionStatementCode(node->children[1]);
            type = tempType;
            // 写回数据
            *code = record;
            code++;
        } else if (node->operatorType == Lor) {
            // 处理逻辑或 ||
            // 生成左侧表达式代码
            generateExpressionStatementCode(node->children[0]);
            // 短路处理, 左侧表达式为真, 直接跳转到后方语句
            *code = JNZ;
            code++;
            point = code;
            code++;
            // 生成右侧表达式代码
            generateExpressionStatementCode(node->children[1]);
            type = Int;
            // 回填跳转地址
            *point = (long long)code;
        } else if (node->operatorType == Land) {
            // 处理逻辑与 &&
            // 生成左侧表达式代码
            generateExpressionStatementCode(node->children[0]);
            // 短路处理, 左侧表达式为假, 直接跳转到后方语句
            *code = JZ;
            code++;
            point = code;
            code++;
            // 生成右侧表达式代码
            generateExpressionStatementCode(node->children[1]);
            type = Int;
            // 回填跳转地址
            *point = (long long)code;
        } else if (node->operatorType == Or) {
            // 处理按位或 |
            // 生成左侧表达式代码
            generateExpressionStatementCode(node->children[0]);
            tempType = type;
            *code = PUSH;
            code++;
            // 生成右侧表达式代码
            generateExpressionStatementCode(node->children[1]);
            type = tempType;
            *code = OR;
            code++;
        } else if (node->operatorType == Xor) {
            // 处理异或 ^
            // 生成左侧表达式代码
            generateExpressionStatementCode(node->children[0]);
            tempType = type;
            *code = PUSH;
            code++;
            // 生成右侧表达式代码
            generateExpressionStatementCode(node->children[1]);
            type = tempType;
            *code = XOR;
            code++;
        } else if (node->operatorType == And) {
            // 处理按位与 &
            // 生成左侧表达式代码
            generateExpressionStatementCode(node->children[0]);
            tempType = type;
            *code = PUSH;
            code++;
            // 生成右侧表达式代码
            generateExpressionStatementCode(node->children[1]);
            type = tempType;
            *code = AND;
            code++;
        } else if (node->operatorType == Eq) {
            // 处理等于 ==
            // 生成左侧表达式代码
            generateExpressionStatementCode(node->children[0]);
            *code = PUSH;
            code++;
            // 生成右侧表达式代码
            generateExpressionStatementCode(node->children[1]);
            type = Int;
            *code = EQ;
            code++;
        } else if (node->operatorType == Ne) {
            // 处理不等于 !+
            // 生成左侧表达式代码
            generateExpressionStatementCode(node->children[0]);
            *code = PUSH;
            code++;
            // 生成右侧表达式代码
            generateExpressionStatementCode(node->children[1]);
            type = Int;
            *code = NE;
            code++;
        } else if (node->operatorType == Lt) {
            // 处理小于 <
            // 生成左侧表达式代码
            generateExpressionStatementCode(node->children[0]);
            *code = PUSH;
            code++;
            // 生成右侧表达式代码
            generateExpressionStatementCode(node->children[1]);
            type = Int;
            *code = LT;
            code++;
        } else if (node->operatorType == Gt) {
            // 处理大于 >
            // 生成左侧表达式代码
            generateExpressionStatementCode(node->children[0]);
            *code = PUSH;
            code++;
            // 生成右侧表达式代码
            generateExpressionStatementCode(node->children[1]);
            type = Int;
            *code = GT;
            code++;
        } else if (node->operatorType == Le) {
            // 处理小于等于 <=
            // 生成左侧表达式代码
            generateExpressionStatementCode(node->children[0]);
            *code = PUSH;
            code++;
            // 生成右侧表达式代码
            generateExpressionStatementCode(node->children[1]);
            type = Int;
            *code = LE;
            code++;
        } else if (node->operatorType == Ge) {
            // 处理大于等于 >=
            // 生成左侧表达式代码
            generateExpressionStatementCode(node->children[0]);
            *code = PUSH;
            code++;
            // 生成右侧表达式代码
            generateExpressionStatementCode(node->children[1]);
            type = Int;
            *code = GE;
            code++;
        } else if (node->operatorType == Shl) {
            // 处理左移 <<
            // 生成左侧表达式代码
            generateExpressionStatementCode(node->children[0]);
            tempType = type;
            *code = PUSH;
            code++;
            // 生成右侧表达式代码
            generateExpressionStatementCode(node->children[1]);
            type = tempType;
            *code = SHL;
            code++;
        } else if (node->operatorType == Shr) {
            // 处理右移 >>
            // 生成左侧表达式代码
            generateExpressionStatementCode(node->children[0]);
            tempType = type;
            *code = PUSH;
            code++;
            // 生成右侧表达式代码
            generateExpressionStatementCode(node->children[1]);
            type = tempType;
            *code = SHR;
            code++;
        } else if (node->operatorType == Add) {
            // 处理加 +
            // 生成左侧表达式代码
            generateExpressionStatementCode(node->children[0]);
            tempType = type;
            *code = PUSH;
            code++;
            // 生成右侧表达式代码
            generateExpressionStatementCode(node->children[1]);
            type = tempType;
            // 判断是否是指针加
            if (type > Ptr) {
                // 判断是指针指针加还是 Int 指针加还是 Char 指针加
                if (type == Int + Ptr) {
                    // Int 指针加
                    *code = PUSH;
                    code++;
                    *code = IMM;
                    code++;
                    *code = 4;
                    code++;
                    *code = MUL;
                    code++;
                } else if (type == Char + Ptr) {
                    // Char 指针加
                    *code = PUSH;
                    code++;
                    *code = IMM;
                    code++;
                    *code = 1;
                    code++;
                    *code = MUL;
                    code++;
                } else {
                    // Ptr 指针加
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
            generateExpressionStatementCode(node->children[0]);
            tempType = type;
            *code = PUSH;
            code++;
            // 生成右侧表达式代码
            generateExpressionStatementCode(node->children[1]);
            type = tempType;
            // 判断是否是指针减
            if (type > Ptr) {
                // 判断是指针指针减还是 Int 指针减还是 Char 指针减
                if (type == Int + Ptr) {
                    // Int 指针减
                    *code = PUSH;
                    code++;
                    *code = IMM;
                    code++;
                    *code = 4;
                    code++;
                    *code = MUL;
                    code++;
                } else if (type == Char + Ptr) {
                    // Char 指针减
                    *code = PUSH;
                    code++;
                    *code = IMM;
                    code++;
                    *code = 1;
                    code++;
                    *code = MUL;
                    code++;
                } else {
                    // Ptr 指针减
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
            *code = SUB;
            code++;
        } else if (node->operatorType == Mul) {
            // 处理乘 *
            // 生成左侧表达式代码
            generateExpressionStatementCode(node->children[0]);
            tempType = type;
            *code = PUSH;
            code++;
            // 生成右侧表达式代码
            generateExpressionStatementCode(node->children[1]);
            type = tempType;
            *code = MUL;
            code++;
        } else if (node->operatorType == Div) {
            // 处理除 /
            // 生成左侧表达式代码
            generateExpressionStatementCode(node->children[0]);
            tempType = type;
            *code = PUSH;
            code++;
            // 生成右侧表达式代码
            generateExpressionStatementCode(node->children[1]);
            type = tempType;
            *code = DIV;
            code++;
        } else if (node->operatorType == Mod) {
            // 处理取模 %
            // 生成左侧表达式代码
            generateExpressionStatementCode(node->children[0]);
            tempType = type;
            *code = PUSH;
            code++;
            // 生成右侧表达式代码
            generateExpressionStatementCode(node->children[1]);
            type = tempType;
            *code = MOD;
            code++;
        } else if (node->operatorType == Bracket) {
            // 处理下标 []
            // 生成左侧表达式代码
            generateExpressionStatementCode(node->children[0]);
            *code = PUSH;
            code++;
            tempType = type;
            // 生成右侧表达式代码
            generateExpressionStatementCode(node->children[1]);
            type = tempType;
            type = type - Ptr;
            // 判断取下标后是什么类型
            if (type > Ptr) {
                // 指针类型
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
                    // Int 类型
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
                    // Char 类型
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
    } else {
        handleErrorInformation(0, CODE_ERROR, "code,c/generateExpressionStatementCode()",
                               "Get an Unknown Expression AST Node", NULL);
    }
}

/**
 * @brief 运行虚拟机代码
 *
 * 运行虚拟机代码
 *
 * @param   void
 * @return  void
 * */
void runCode(){
    long long op = 0;               // 记录指令
    long long* temp = NULL;         // 辅助指针

    // 初始化 sp 指针与 bp指针
    bp = sp = (long long*)((long long)stack + MAX_SIZE);
    // 初始化 ax
    ax = 0;
    // 获取 main 函数入口指令
    pc = (long long*)(*mainPtr);
    // 初始化栈空间内容
    *sp = EXIT;
    sp--;
    *sp = PUSH;
    temp = sp;
    sp--;
    *sp = (long long)temp;
    // 未找到 main 函数入口, 打印错误信息
    if (pc == NULL) {
        handleErrorInformation(0, RUNNING_ERROR, "code.c/runCode()",
                               "Could not Find main Function", NULL);
    }
    cycle = 0;
    // 执行指令
    while (true) {
        cycle++;
        // 取指令
        op = *pc;
        // pc 自增
        pc++;
        // 判断指令类型并执行相应操作
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
            handleErrorInformation(0, RUNNING_ERROR, "code.c/runCode()",
                                   "Get an Unknown Instruction", NULL);
            exit(1);
        }
    }
}