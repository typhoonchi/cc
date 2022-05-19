//
// Created by zhangyukun on 2022/4/11.
//
#include "globals.h"
#include "utility.h"
#include "parser.h"
#include "scanner.h"

/* 匹配 token
 *
 * @param expected  期待词素
 *
 * */
void match(int expected) {
    if (token != expected) {
        // token 不匹配
        printErrorInformation("Find Unexpect Token",tokenString);
        exit(1);
    }
    getToken();
}

/* 检查是否是已声明的标识符
 *
 * */
void checkNewId() {
    if (token != Id) {
        // 非标识符
        printErrorInformation("Get Invalid Identifier",tokenString);
        exit(1);
    }
    if (symbolPtr->class != 0) {
        // 已声明标识符
        printErrorInformation("Get Duplicate Declaration",tokenString);
        exit(1);
    }
}

/* 检查是否是未声明的标识符
 *
 * */
void checkDeclaredId() {
    if (token != Id) {
        // 非标识符
        printErrorInformation("Get Invalid Identifier",tokenString);
        exit(1);
    }
    if (symbolPtr->class == 0) {
        // 未声明标识符
        printErrorInformation("Get Undeclared Identifier",tokenString);
        exit(1);
    }
}

/* 检查局部变量
 *
 * */
void checkLocalId() {
    if (token != Id) {
        // 非标识符
        printErrorInformation("Get Invalid Identifier",tokenString);
        exit(1);
    }
    if (symbolPtr->class == Loc) {
        // 已声明标识符
        printErrorInformation("Get Duplicate Declaration",tokenString);
        exit(1);
    }
}

/* 隐藏全局变量
 *
 * */
void hideGlobalId(){
    symbolPtr->gClass = symbolPtr->class;
    symbolPtr->gType = symbolPtr->type;
    symbolPtr->gValue = symbolPtr->value;
}

/* 恢复全局变量
 *
 * */
void recoverGlobalId(){
    symbolPtr->class = symbolPtr->gClass;
    symbolPtr->type = symbolPtr->gType;
    symbolPtr->value = symbolPtr->gValue;
}

/* 解析基本变量类型
 *
 * @return 基本变量类型 :Int Char
 *
 * */
int parseBaseType(){
    if (token == CHAR) {
        match(CHAR);
        return Char;
    } else if (token == INT) {
        match(INT);
        return Int;
    } else {
        printErrorInformation("Get Base Type Error",NULL);
        exit(1);
    }
}

/* 入栈
 *
 * @param node 抽象语法树节点指针
 *
 * */
void push(struct treeNode* node) {
    nodeStack[top++] = node;
}

/* 出栈
 *
 * @return 抽象语法树节点指针
 *
 * */
struct treeNode* pop() {
    if (top == 0) {
        printErrorInformation("Get Expression Stack Error", NULL);
        exit(1);
    }
    return nodeStack[--top];
}

/* 解析函数
 *
 * @param type 函数返回值
 * @param name 函数值
 * @return 抽象语法树节点指针, 指向函数定义子树
 *
 * */
struct treeNode* parseFunction(int type, char* name){
    // 创建函数根节点
    struct treeNode* node = createNode(Function, 0, 0);
    node->identifierType = type;
    node->name = name;
    node->value = (long long)&(symbolPtr->value);
    match('(');
    // 解析参数列表
    if (token == ')') {
        // 空参列表 void
        node->children[0] = createNode(ParameterStatement, 0, 0);
        node->children[0]->identifierType = VOID;
    } else {
        // 有参列表 解析参数列表
        node->children[0] = parseParameters();
    }
    match(')');
    match('{');
    // 解析函数体
    node->children[1] = parseFunctionBody();
//    if (*code != RET) {
//        *++code = RET;
//    }
    // 恢复全局变量
    symbolPtr = symbolTable;
    while (symbolPtr->token != 0) {
        if (symbolPtr->class == Loc) {
            recoverGlobalId();
        }
        symbolPtr++;
    }
    return node;
}

/* 解析参数列表
 *
 * @return 抽象语法树节点指针, 指向参数列表子树
 * */
struct treeNode* parseParameters(){
    // 参数计数器
    long long num = 0;
    // 创建参数列表根节点
    struct treeNode* node = NULL;
    struct treeNode* lastSibling;
    struct treeNode* tempNode;
    while (token != ')') {
        tempNode = createNode(ParameterStatement, 0, 0);
        // 获取参数类型
        tempNode->identifierType = parseBaseType();
        // 获取参数名称
        tempNode->name = tokenString;
        // 形参作为局部变量使用
        checkLocalId();
        hideGlobalId();
        match(Id);
        while (token != ',' && token!= ')') {
            if (token == Bracket) {
                // 匹配数组参数
                match(Bracket);
                tempNode->identifierType += Ptr;
                match(']');
            }
        }
        symbolPtr->class = Loc;
        symbolPtr->type = tempNode->identifierType;
        symbolPtr->value = num++;
        if (token == ',') {
            // 匹配后续参数
            match(',');
        }
        if (node != NULL) {
            lastSibling = node;
            while (lastSibling->sibling != NULL) {
                lastSibling = lastSibling->sibling;
            }
            lastSibling->sibling = tempNode;
        } else {
            node = tempNode;
        }
    }
    ibp = ++num;
    return node;
}

/* 解析函数体
 *
 * @return 抽象语法树节点指针, 指向函数体子树
 * */
struct treeNode* parseFunctionBody(){
    //
    long long num = ibp;
    // 创建函数体根节点
    struct treeNode* node = NULL;
    // 定位最后一个兄弟节点指针
    struct treeNode* lastSibling;
    // 标识符类别
    int type;
    // 标识符名称
    char* identifierName;
    // 解析局部变量声明语句
    while (token == INT || token == CHAR) {
        type = parseBaseType();
        identifierName = tokenString;
        // 检查是否已声明过该局部变量
        checkLocalId();
        // 隐藏全局变量
        hideGlobalId();
        symbolPtr->class = Loc;
        symbolPtr->type = type;
        symbolPtr->value = ++num;
        match(Id);
        if (node != NULL) {
            // 非空指针 寻找到最后一个兄弟节点处
            lastSibling = node;
            while (lastSibling->sibling != NULL) {
                lastSibling = lastSibling->sibling;
            }
            // 该兄弟节点指向声明语句
            lastSibling->sibling = parseDeclaration(type,identifierName,1, &num);
        } else {
            // 空指针 直接指向第一条声明语句
            node = parseDeclaration(type, identifierName, 1, &num);
        }
        match(';');
    }
//    *++code = NVAR;
//    *++code = num - ibp;
    // 解析非声明语句
    while (token != '}') {
        if (node !=  NULL) {
            // 非空指针 寻找到最后一个兄弟结点处
            lastSibling = node;
            while (lastSibling->sibling != NULL) {
                lastSibling = lastSibling->sibling;
            }
            // 该兄弟节点指向语句
            lastSibling->sibling = parseStatement();
        } else {
            // 空指针 直接指向语句
            node = parseStatement();
        }
    }
    return node;
}

/* 解析声明语句
 *
 * @param type 变量类型
 * @param name 变量名
 * @param mode 声明模式 0 全局模式  1 局部模式
 * @param num 指向局部变量计数器指针
 * @return 抽象语法树节点指针, 指向一条声明语句子树
 * */
struct treeNode* parseDeclaration(int type, char* name, int mode, long long* num){
    // 创建声明语句节点
    struct treeNode* node = createNode(DeclareStatement, 0, 0);
    char* identifierName;
    long long size = 1;
    long long* base;
    node->identifierType = type;
    node->name = name;
    node->value = symbolPtr->value;
    while (token != ',' && token != ';') {
        // 解析数组声明
        if (token == Bracket) {
            symbolPtr->type += Ptr;
            node->identifierType += Ptr;
            match(Bracket);
            base = (data - 1);
            for (int i = 1; i <= size; i++) {
                *(base - size + i) = (long long)data;
                data += tokenValue;
            }
            node->isArray = true;
            node->size *= tokenValue;
            size *= tokenValue;
            match(Num);
            match(']');
        }
    }
    if (token == ',') {
        // 解析逗号
        match(',');
        identifierName = tokenString;
        if (mode) {
            /*
             * 0 全局变量
             * 1 局部变量
             */
            checkLocalId();
            hideGlobalId();
            symbolPtr->class = Loc;
            symbolPtr->type = type;
            symbolPtr->value = ++(*num);
        } else {
            symbolPtr->class = Glo;
            symbolPtr->type = type;
            symbolPtr->value = (long long)data;
            data++;
        }
        match(Id);
        // 继续解析声明语句
        node->sibling = parseDeclaration(node->identifierType,identifierName, mode, num);
    }
    return node;
}

/* 解析语句
 *
 * @return 抽象语法树节点指针, 指向语句子树
 * */
struct treeNode* parseStatement() {
    // 创建语句根节点
    struct treeNode* node = NULL;
    if (token == IF) {
        // 匹配 If 语句
        node = parseIfStatement();
    } else if (token == WHILE) {
        // 匹配 While 语句
        node = parseWhileStatement();
    } else if (token == RETURN) {
        // 匹配 Return 语句
        node = parseReturnStatement();
    } else if (token == ';') {
        // 匹配空语句 直接跳过
        match(';');
    } else {
        // 匹配表达式语句
        node = parseExpressionStatement(Assign);
    }
    return node;
}

/* 解析 If 语句
 *
 * @return 抽象语法树节点指针, 指向 If 语句子树
 * */
struct treeNode* parseIfStatement(){
    //
//    long long *branchPoint;
    // 创建 If 语句根节点
    struct treeNode* node = createNode(IfStatement, 0, 0);
    struct treeNode* lastSibling;
    match(IF);
    match('(');
    // 解析条件表达式
    node->children[0] = parseExpressionStatement(Assign);
    match(')');
    match('{');
//    *++code = JZ;
//    branchPoint = ++code;
    while (token != '}') {
        // 解析成功分支语句
        if (node->children[1] != NULL) {
            lastSibling = node->children[1];
            while (lastSibling->sibling != NULL) {
                lastSibling = lastSibling->sibling;
            }
            lastSibling->sibling = parseStatement();
        } else {
            node->children[1] = parseStatement();
        }
    }
    match('}');
    if (token == ELSE) {
        // 如果有 else 语句
        match(ELSE);
        match('{');
//        *branchPoint = (long long)(code + 3);
//        *++code = JMP;
//        branchPoint = ++code;
        while (token != '}') {
            // 解析失败分支
            if (node->children[2] != NULL) {
                lastSibling = node->children[2];
                while (lastSibling->sibling != NULL) {
                    lastSibling= lastSibling->sibling;
                }
                lastSibling->sibling = parseStatement();
            } else {
                node->children[2] = parseStatement();
            }
        }
        match('}');
    }
//    *branchPoint = (long long)(code + 1);
    return node;
}

/* 解析 While 语句
 *
 * @return 返回抽象语法树节点指针, 指向 While 语句子树
 * */
struct treeNode* parseWhileStatement(){
//    long long* loopPoint, *endPoint;
    // 创建 While 语句根节点
    struct treeNode* node = createNode(WhileStatement, 0, 0);
    struct treeNode* lastSibling;
    match(WHILE);
//    loopPoint = code + 1;
    match('(');
    // 解析条件语句
    node->children[0] = parseExpressionStatement(Assign);
    match(')');
    match('{');
//    *++code = JZ;
//    endPoint = ++code;
    while (token != '}') {
        // 解析循环体语句
        if (node->children[1] != NULL) {
            lastSibling = node->children[1];
            while (lastSibling->sibling != NULL) {
                lastSibling = lastSibling->sibling;
            }
            lastSibling->sibling = parseStatement();
        } else {
            node->children[1] = parseStatement();
        }
    }
    match('}');
//    *++code = JMP;
//    *++code = (long long)loopPoint;
//    *endPoint = (long long)(code + 1);
    return node;
}

/* 解析返回语句
 *
 * @return 返回抽象语法树节点指针, 指向 Return 语句子树
 * */
struct treeNode* parseReturnStatement(){
    // 创建 Return 语句根节点
    struct treeNode* node = createNode(ReturnStatement,0, 0);
    match(RETURN);
    // 解析返回表达式
    node->children[0] = parseExpressionStatement(Assign);
    match(';');
//    *++code = RET;
    return node;
}

/* 解析表达式语句
 *
 * @param operator 运算符
 * @return 返回抽象语法树节点指针, 指向表达式语句子树
 * */
static int type;
struct treeNode* parseExpressionStatement(int operator){
    struct symbol* tempSymbolPtr;
    long long * tempPtr, num;
    int tempType;
    // 表达式语句节点
    struct treeNode* node;
    // 临时节点
    struct treeNode* lastSibling;
    if (token == Num) {
        // 为数值常量创建节点
        node = createNode(ExpressStatement, Constant, 0);
        node->identifierType = Int;
        node->value = tokenValue;
//        *++code = IMM;
//        *++code = tokenValue;
        push(node);
        match(Num);
        type = Int;
    } else if (token == Char){
        // 为字符常量创建节点
        node = createNode(ExpressStatement, Constant, 0);
        node->identifierType = Char;
        node->value = tokenValue;
//        *++code = IMM;
//        *++code = tokenValue | 0x7F7F7F7F7F7F7F00;
        push(node);
        match(Char);
        type = Int;
    } else if (token == Char + Ptr) {
        // 为字符常量创建节点
        node = createNode(ExpressStatement, Constant, 0);
        node->identifierType = Char + Ptr;
        node->value = tokenValue;
//        *++code = IMM;
//        *++code = tokenValue;
        push(node);
        match(Char + Ptr);
        type = Int;
    } else if (token == Id) {
        // 为标识符创建节点
        node = createNode(ExpressStatement, Identifier, 0);
        node->name = tokenString;
        node->value = symbolPtr->value;
        checkDeclaredId();
        tempSymbolPtr = symbolPtr;
        match(Id);
        if (token == Id){
            printErrorInformation("Get Invalid Expression Statement", NULL);
            exit(1);
        } else if (token == '(') {
            num = 0;
            // 处理函数调用
            if (symbolPtr->class != Fun && symbolPtr->class != Sys) {
                printErrorInformation("Get Invalid Function Call", NULL);
                exit(1);
            }
            node->expressionType = Call;
            node->value = (long long)&(tempSymbolPtr->value);
            match('(');
            while (token != ')') {
                if (node->children[0] != NULL) {
                    lastSibling = node->children[0];
                    while (lastSibling->sibling != NULL) {
                        lastSibling = lastSibling->sibling;
                    }
                    lastSibling->sibling = parseExpressionStatement(Assign);
                } else {
                    node->children[0] = parseExpressionStatement(Assign);
                }
//                *++code = PUSH;
                num++;
                if (token != ')') {
                    match(',');
                }
            }
            match(')');
            if (tempSymbolPtr->class == Sys) {
                node->classType = Sys;
//                *++code = tempSymbolPtr->value;
            } else {
//                *++code = CALL;
//                *++code = tempSymbolPtr->value;
            }
//            if (num > 0) {
//                *++code = DARG;
//                *++code = num;
//            }
            type = tempSymbolPtr->type;
        } else {
            // 为标识符节点赋值
            node->value = tempSymbolPtr->value;
            node->identifierType = tempSymbolPtr->type;
            node->classType = tempSymbolPtr->class;
//            if (tempSymbolPtr->class == Glo) {
//                *++code = IMM;
//                *++code = tempSymbolPtr->value;
//            } else if (tempSymbolPtr->class == Loc) {
//                *++code = LEA;
//                *++code = ibp - tempSymbolPtr->value;
//            }
            type = tempSymbolPtr->type;
//            *++code = LI;
        }
        push(node);
    } else if (token == '(') {
        // 处理表达式中括号内的子表达式
        match('(');
        node = parseExpressionStatement(Assign);
        match(')');
        push(node);
    } else if (token == '!') {
        // 处理单目运算符逻辑非 !
        node = createNode(ExpressStatement, Operator, '!');
        match('!');
        node->children[0] = parseExpressionStatement(Inc);
//        *++code = PUSH;
//        *++code = IMM;
//        *++code = 0;
//        *++code = EQ;
        push(node);
        type = Int;
    } else {
        // 处理错误信息
        printErrorInformation("Get Invalid Expression", NULL);
        exit(1);
    }
    // 优先级爬山处理表达式运算
    while (token >= operator) {
        tempType = type;
        // 仅处理优先级不低于当前优先级的运算子表达式
        if (token == Assign) {
            // 处理赋值 =
            match(Assign);
//            if (*code == LI) {
//                *code = PUSH;
//            }
            node = createNode(ExpressStatement, Operator, Assign);
            // 处理左值
            node->children[0] = pop();
            // 处理右值子表达式
            node->children[1] = parseExpressionStatement(Assign);
//            *++code = SI;
            push(node);
            type = tempType;
        } else if (token == Lor) {
            // 处理逻辑或 ||
            match(Lor);
//            *++code = JNZ;
//            tempPtr = ++code;
            node = createNode(ExpressStatement, Operator, Lor);
            node->children[0] = pop();
            node->children[1] = parseExpressionStatement(Land);
//            *tempPtr = (long long)(code + 1);
            push(node);
            type = Int;
        } else if (token == Land) {
            // 处理逻辑与 &&
            match(Land);
//            *++code = JZ;
//            tempPtr = ++code;
            node = createNode(ExpressStatement, Operator, Land);
            node->children[0] = pop();
            node->children[1] = parseExpressionStatement(Or);
//            *tempPtr = (long long)(code + 1);
            push(node);
            type = Int;
        } else if (token == Or) {
            // 处理按位或 |
            match(Or);
//            *++code = PUSH;
            node = createNode(ExpressStatement, Operator, Or);
            node->children[0] = pop();
            node->children[1] = parseExpressionStatement(Xor);
//            *++code = OR;
            push(node);
            type = Int;
        } else if (token == Xor) {
            // 处理异或 ^
            match(Xor);
//            *++code = PUSH;
            node = createNode(ExpressStatement, Operator, Xor);
            node->children[0] = pop();
            node->children[1] = parseExpressionStatement(And);
//            *++code = XOR;
            push(node);
            type = Int;
        } else if (token == And) {
            // 处理按位与 &
            match(And);
//            *++code = PUSH;
            node = createNode(ExpressStatement, Operator, And);
            node->children[0] = pop();
            node->children[1] = parseExpressionStatement(Eq);
//            *++code = AND;
            push(node);
            type = Int;
        } else if (token == Eq) {
            // 处理相等 ==
            match(Eq);
//            *++code = PUSH;
            node = createNode(ExpressStatement, Operator, Eq);
            node->children[0] = pop();
            node->children[1] = parseExpressionStatement(Ne);
//            *++code = EQ;
            push(node);
            type = Int;
        } else if (token == Ne) {
            // 处理不等 !=
            match(Ne);
//            *++code = PUSH;
            node = createNode(ExpressStatement, Operator, Ne);
            node->children[0] = pop();
            node->children[1] = parseExpressionStatement(Lt);
//            *++code = NE;
            push(node);
            type = Int;
        } else if (token == Lt) {
            // 处理小于 <
            match(Lt);
//            *++code = PUSH;
            node = createNode(ExpressStatement, Operator, Lt);
            node->children[0] = pop();
            node->children[1] = parseExpressionStatement(Gt);
//            *++code = LT;
            push(node);
            type = Int;
        } else if (token == Gt) {
            // 处理大于 >
            match(Gt);
//            *++code = PUSH;
            node = createNode(ExpressStatement, Operator, Gt);
            node->children[0] = pop();
            node->children[1] = parseExpressionStatement(Le);
//            *++code = GT;
            push(node);
            type = Int;
        } else if (token == Le) {
            // 处理小于等于 <=
            match(Le);
//            *++code = PUSH;
            node = createNode(ExpressStatement, Operator, Le);
            node->children[0] = pop();
            node->children[1] = parseExpressionStatement(Ge);
//            *++code = LE;
            push(node);
            type = Int;
        } else if (token == Ge) {
            // 处理大于等于 >=
            match(Ge);
//            *++code = PUSH;
            node = createNode(ExpressStatement, Operator, Ge);
            node->children[0] = pop();
            node->children[1] = parseExpressionStatement(Shl);
//            *++code = GE;
            push(node);
            type = Int;
        } else if (token == Shl) {
            // 处理左移 <<
            match(Shl);
//            *++code = PUSH;
            node = createNode(ExpressStatement, Operator, Shl);
            node->children[0] = pop();
            node->children[1] = parseExpressionStatement(Shr);
//            *++code = SHL;
            push(node);
            type = Int;
        } else if (token == Shr) {
            // 处理右移 >>
            match(Shr);
//            *++code = PUSH;
            node = createNode(ExpressStatement, Operator, Shr);
            node->children[0] = pop();
            node->children[1] = parseExpressionStatement(Add);
//            *++code = SHR;
            push(node);
            type = Int;
        } else if (token == Add) {
            // 处理加 +
            match(Add);
//            *++code = PUSH;
            node = createNode(ExpressStatement, Operator, Add);
            node->children[0] = pop();
            node->children[1] = parseExpressionStatement(Mul);
//            *++code = ADD;
            push(node);
            type = tempType;
        } else if (token == Sub) {
            // 处理减 -
            match(Sub);
//            *++code = PUSH;
            node = createNode(ExpressStatement, Operator, Sub);
            node->children[0] = pop();
            node->children[1] = parseExpressionStatement(Mul);
//            *++code = SUB;
            push(node);
        } else if (token == Mul) {
            // 处理乘 *
            match(Mul);
//            *++code = PUSH;
            node = createNode(ExpressStatement, Operator, Mul);
            node->children[0] = pop();
            node->children[1] = parseExpressionStatement(Bracket);
//            *++code = MUL;
            push(node);
            type = Int;
        } else if (token == Div) {
            // 处理除 /
            match(Div);
//            *++code = PUSH;
            node = createNode(ExpressStatement, Operator, Div);
            node->children[0] = pop();
            node->children[1] = parseExpressionStatement(Bracket);
//            *++code = DIV;
            push(node);
            type = Int;
        } else if (token == Mod) {
            // 处理取模 %
            match(Mod);
//            *++code = PUSH;
            node = createNode(ExpressStatement, Operator, Mod);
            node->children[0] = pop();
            node->children[1] = parseExpressionStatement(Bracket);
//            *++code = MOD;
            push(node);
            type = Int;
        } else if (token == Bracket) {
            // 处理下标 []
            match(Bracket);
//            *++code = PUSH;
            node = createNode(ExpressStatement, Operator, Bracket);
            node->children[0] = pop();
            node->children[1] = parseExpressionStatement(Assign);
//            if (tempType > Ptr) {
//                *++code = PUSH;
//                *++code = IMM;
//                *++code = 8;
//                *++code = MUL;
//            }
//            *++code = ADD;
//            *++code = LI;
            push(node);
            match(']');
            type = tempType - Ptr;
        } else {
            printErrorInformation("Get Invalid Token", tokenString);
            exit(1);
        }
    }
    return pop();
}

/* 语法分析
 *
 * */
void parse(void) {
    // 存放标识符类型
    int baseType;
    // 存放标识符名称
    char* idName;
    // 临时节点
    struct treeNode* node, * lastSibling;
    // 取 token
    getToken();
    while (token > 0) {
        // 语法分析直到结束或出错
        if (token == INT || token == CHAR) {
            // 处理声明语句
            // 获取标识符类型
            baseType = parseBaseType();
            // 获取标识符名
            idName = tokenString;
            // 检查是否重复声明
            checkNewId();
            symbolPtr->type = baseType;
            match(Id);
            if (token == '(') {
                // 处理函数定义
                symbolPtr->class = Fun;

//                symbolPtr->value = (long long)(code + 1);
                if (!memcmp("main", idName, strlen("main"))) {
                    mainPtr = &(symbolPtr->value);
                }
                node = parseFunction(baseType, idName);
                match('}');
            } else {
                // 处理全局变量声明
                symbolPtr->class = Glo;
                symbolPtr->value = (long long)data;
                data++;
                node = parseDeclaration(baseType, idName, 0, NULL);
                match(';');
            }
        } else if (token == VOID) {
            // 处理空返回值函数定义
            // 获取标识符类型
            baseType = VOID;
            match(VOID);
            // 获取标识符名
            idName = tokenString;
            // 检查是否重复声明
            checkNewId();
            symbolPtr->type = baseType;
            match(Id);
            node = parseFunction(baseType, idName);
        } else {
            printErrorInformation("Get Invalid Statement", NULL);
            exit(1);
        }
        if (root != NULL) {
            lastSibling = root;
            while (lastSibling->sibling != NULL) {
                lastSibling = lastSibling->sibling;
            }
            lastSibling->sibling = node;
        } else {
            root = node;
        }
    }
}

