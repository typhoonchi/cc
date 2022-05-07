//
// Created by zhangyukun on 2022/4/11.
//
#include "globals.h"
#include "utility.h"
#include "parse.h"
#include "scan.h"

void match(int expected) {
    if (token != expected) {
        printf("line %d: unexpect token: %s\n", line, tokenString);
        exit(1);
    }
    getToken();
}

void checkNewId() {
    if (token != Id) {
        printf("line %d: invalid identifier: %s\n", line, tokenString);
        exit(1);
    }
    if (symbolPtr->class != 0) {
        printf("line %d: duplicate declaration: %s\n", line, tokenString);
        exit(1);
    }
}

void checkLocalId() {
    if (token != Id) {
        printf("line %d: invalid identifier: %s\n", line, tokenString);
        exit(1);
    }
    if (symbolPtr->class == Loc) {
        printf("line %d: duplicate declaration: %s\n", line, tokenString);
        exit(1);
    }
}

void hideGlobalId(){
    symbolPtr->gClass = symbolPtr->class;
    symbolPtr->gType = symbolPtr->type;
    symbolPtr->gValue = symbolPtr->value;
}

void recoverGlobalId(){
    symbolPtr->class = symbolPtr->gClass;
    symbolPtr->type = symbolPtr->gType;
    symbolPtr->value = symbolPtr->gValue;
}

int parseBaseType(){
    if (token == CHAR) {
        match(CHAR);
        return CHAR;
    } else if (token == INT) {
        match(INT);
        return INT;
    }
}

void push(struct treeNode* node) {
    symbolStack[top++] = node;
}
struct treeNode* pop(void) {
    if (top == 0) {
        printf("Expression Stack Error!\n");
        exit(1);
    }
    return symbolStack[--top];
}

// 解析函数
struct treeNode* parseFunction(int type, char* name){
    // 创建函数根节点
    struct treeNode* node = createNode(Function, 0, 0);
    node->identifierType = type;
    node->name = name;
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

// 解析参数列表
struct treeNode* parseParameters(void){
    // 创建参数列表根节点
    struct treeNode* node = createNode(ParameterStatement, 0, 0);
    node->identifierType = parseBaseType();
    node->name = tokenString;
    match(Id);
    if (token == Bracket) {
        // 匹配数组参数
        match(Bracket);
        match(']');
    }
    if (token == ',') {
        // 匹配后续参数
        match(',');
        node->sibling = parseParameters();
    }
    return node;
}

// 解析函数体
struct treeNode* parseFunctionBody(void){
    // 创建函数体根节点
    struct treeNode* node = NULL;
    struct treeNode* lastSibling;
    int type;
    char* identifierName;
    // 解析局部变量生命语句
    while (token == INT || token == CHAR) {
        type = parseBaseType();
        identifierName = tokenString;
        checkLocalId();
        hideGlobalId();
        symbolPtr->class = Loc;
        symbolPtr->type = type;
        match(Id);
        if (node == NULL) {
            // 空指针 直接指向第一条声明语句
            node = parseDeclaration(type, identifierName, 1);
        } else {
            // 非空指针 寻找到最后一个兄弟节点处
            lastSibling = node;
            while (lastSibling->sibling != NULL) {
                lastSibling = lastSibling->sibling;
            }
            // 该兄弟节点指向声明语句
            lastSibling->sibling = parseDeclaration(type,identifierName,1);
        }
        match(';');
    }
    // 解析非声明语句
    while (token != '}') {
        if (node ==  NULL) {
            // 空指针 直接指向语句
            node = parseStatement();
        } else {
            // 非空指针 寻找到最后一个兄弟结点处
            lastSibling = node;
            while (lastSibling->sibling != NULL) {
                lastSibling = lastSibling->sibling;
            }
            // 该兄弟节点指向语句
            lastSibling->sibling = parseStatement();
        }
    }
    return node;
}

// 解析声明语句
struct treeNode* parseDeclaration(int type, char* name, int mode){
    // 创建声明语句节点
    struct treeNode* node = createNode(DeclareStatement, 0, 0);
    char* identifierName;
    node->identifierType = type;
    node->name = name;
    if (token == Bracket) {
        // 解析数组声明
        if (symbolPtr->type == INT) {
            symbolPtr->type = INTARRAY;
            node->identifierType = INTARRAY;
        } else if (symbolPtr->type == CHAR) {
            symbolPtr->type = CHARARRAY;
            node->identifierType = CHARARRAY;
        }
        match(Bracket);
        node->isArray = true;
        node->size = tokenValue;
        match(Num);
        match(']');
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
        } else {
            symbolPtr->class = Glo;
            symbolPtr->type = type;
        }
        match(Id);
        // 继续解析声明语句
        node->sibling = parseDeclaration(node->identifierType,identifierName, mode);
    }
    return node;
}

// 解析语句
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

// 解析 If 语句
struct treeNode* parseIfStatement(){
    // 创建 If 语句根节点
    struct treeNode* node = createNode(IfStatement, 0, 0);
    struct treeNode* lastSibling;
    match(IF);
    match('(');
    // 解析条件表达式
    node->children[0] = parseExpressionStatement(Assign);
    match(')');
    match('{');
    while (token != '}') {
        // 解析成功分支语句
        if (node->children[1] == NULL) {
            node->children[1] = parseStatement();
        } else {
            lastSibling = node->children[1];
            while (lastSibling->sibling != NULL) {
                lastSibling = lastSibling->sibling;
            }
            lastSibling->sibling = parseStatement();
        }
    }
    match('}');
    if (token == ELSE) {
        // 如果有 else 语句
        match(ELSE);
        match('{');
        while (token != '}') {
            // 解析失败分支
            if (node->children[2] == NULL) {
                node->children[2] = parseStatement();
            } else {
                lastSibling = node->children[2];
                while (lastSibling->sibling != NULL) {
                    lastSibling= lastSibling->sibling;
                }
                lastSibling->sibling = parseStatement();
            }
        }
        match('}');
    }
    return node;
}

// 解析 While 语句
struct treeNode* parseWhileStatement(){
    struct treeNode* node = createNode(WhileStatement, 0, 0);
    struct treeNode* lastSibling;
    match(WHILE);
    match('(');
    // 解析条件语句
    node->children[0] = parseExpressionStatement(Assign);
    match(')');
    match('{');
    while (token != '}') {
        // 解析循环体语句
        if (node->children[1] == NULL) {
            node->children[1] = parseStatement();
        } else {
            lastSibling = node->children[1];
            while (lastSibling->sibling != NULL) {
                lastSibling = lastSibling->sibling;
            }
            lastSibling->sibling = parseStatement();
        }
    }
    match('}');
    return node;
}

// 解析返回语句
struct treeNode* parseReturnStatement(){
    struct treeNode* node = createNode(ReturnStatement,0, 0);
    match(RETURN);
    node->children[0] = parseExpressionStatement(Assign);
    match(';');
    return node;
}

// 解析表达式
struct treeNode* parseExpressionStatement(int operator){
    struct treeNode* node;
    struct treeNode* temp;
    if (token == Num) {
        node = createNode(ExpressStatement, Constant, 0);
        node->identifierType = INT;
        node->value = tokenValue;
        push(node);
        match(Num);
    } else if (token == Char){
        node = createNode(ExpressStatement, Constant, 0);
        node->identifierType = CHAR;
        node->value = tokenValue;
        push(node);
        match(Char);
    } else if (token == Id) {
        node = createNode(ExpressStatement, Identifier, 0);
        temp = node;
        node->name = tokenString;
        match(Id);
        if (token == Id){
            printf("line %d: invalid expression statement\n", line);
            exit(1);
        }
        if (token == '(') {
            if (symbolPtr->class != Fun) {
                printf("line %d: invalid function call\n",line);
                exit(1);
            }
            node->expressionType = Call;
            match('(');
            while (token != ')') {
                if (node->children[0] != NULL) {
                    temp = node->children[0];
                    while (temp->sibling != NULL) {
                        temp = temp->sibling;
                    }
                    temp->sibling = parseExpressionStatement(Assign);
                } else {
                    node->children[0] = parseExpressionStatement(Assign);
                }
                if (token == ',') {
                    match(',');
                }
            }
            match(')');
        } else {
            if (symbolPtr->class == Glo) {
                node->value = symbolPtr->gValue;
            } else if (symbolPtr->class == Loc) {
                node->value = symbolPtr->value;
            }
        }
        push(node);
    } else if (token == '(') {
        match('(');
        node = parseExpressionStatement(Assign);
        match(')');
        push(node);
    } else if (token == '!') {
        node = createNode(ExpressStatement, Operator, '!');
        match('!');
        node->children[0] = parseExpressionStatement(Inc);
        push(node);
    } else {
        printf("line %d: invalid expression\n", line);
        exit(1);
    }
    while (token >= operator) {
        if (token == Assign) {
            match(Assign);
            node = createNode(ExpressStatement, Operator, Assign);
            node->children[0] = pop();
            node->children[1] = parseExpressionStatement(Assign);
            push(node);
        } else if (token == Lor) {
            match(Lor);
            node = createNode(ExpressStatement, Operator, Lor);
            node->children[0] = pop();
            node->children[1] = parseExpressionStatement(Land);
            push(node);
        } else if (token == Land) {
            match(Land);
            node = createNode(ExpressStatement, Operator, Land);
            node->children[0] = pop();
            node->children[1] = parseExpressionStatement(Or);
            push(node);
        } else if (token == Or) {
            match(Or);
            node = createNode(ExpressStatement, Operator, Or);
            node->children[0] = pop();
            node->children[1] = parseExpressionStatement(Xor);
            push(node);
        } else if (token == Xor) {
            match(Xor);
            node = createNode(ExpressStatement, Operator, Xor);
            node->children[0] = pop();
            node->children[1] = parseExpressionStatement(And);
            push(node);
        } else if (token == And) {
            match(And);
            node = createNode(ExpressStatement, Operator, And);
            node->children[0] = pop();
            node->children[1] = parseExpressionStatement(Eq);
            push(node);
        } else if (token == Eq) {
            match(Eq);
            node = createNode(ExpressStatement, Operator, Eq);
            node->children[0] = pop();
            node->children[1] = parseExpressionStatement(Ne);
            push(node);
        } else if (token == Ne) {
            match(Ne);
            node = createNode(ExpressStatement, Operator, Ne);
            node->children[0] = pop();
            node->children[1] = parseExpressionStatement(Lt);
            push(node);
        } else if (token == Lt) {
            match(Lt);
            node = createNode(ExpressStatement, Operator, Lt);
            node->children[0] = pop();
            node->children[1] = parseExpressionStatement(Gt);
            push(node);
        } else if (token == Gt) {
            match(Gt);
            node = createNode(ExpressStatement, Operator, Gt);
            node->children[0] = pop();
            node->children[1] = parseExpressionStatement(Le);
            push(node);
        } else if (token == Le) {
            match(Le);
            node = createNode(ExpressStatement, Operator, Le);
            node->children[0] = pop();
            node->children[1] = parseExpressionStatement(Ge);
            push(node);
        } else if (token == Ge) {
            match(Ge);
            node = createNode(ExpressStatement, Operator, Ge);
            node->children[0] = pop();
            node->children[1] = parseExpressionStatement(Shl);
            push(node);
        } else if (token == Shl) {
            match(Shl);
            node = createNode(ExpressStatement, Operator, Shl);
            node->children[0] = pop();
            node->children[1] = parseExpressionStatement(Shr);
            push(node);
        } else if (token == Shr) {
            match(Shr);
            node = createNode(ExpressStatement, Operator, Shr);
            node->children[0] = pop();
            node->children[1] = parseExpressionStatement(Add);
            push(node);
        } else if (token == Add) {
            match(Add);
            node = createNode(ExpressStatement, Operator, Add);
            node->children[0] = pop();
            node->children[1] = parseExpressionStatement(Mul);
            push(node);
        } else if (token == Sub) {
            match(Sub);
            node = createNode(ExpressStatement, Operator, Sub);
            node->children[0] = pop();
            node->children[1] = parseExpressionStatement(Mul);
            push(node);
        } else if (token == Mul) {
            match(Mul);
            node = createNode(ExpressStatement, Operator, Mul);
            node->children[0] = pop();
            node->children[1] = parseExpressionStatement(Bracket);
            push(node);
        } else if (token == Div) {
            match(Div);
            node = createNode(ExpressStatement, Operator, Div);
            node->children[0] = pop();
            node->children[1] = parseExpressionStatement(Bracket);
            push(node);
        } else if (token == Mod) {
            match(Mod);
            node = createNode(ExpressStatement, Operator, Mod);
            node->children[0] = pop();
            node->children[1] = parseExpressionStatement(Bracket);
            push(node);
        } else if (token == Bracket) {
            match(Bracket);
            node = createNode(ExpressStatement, Operator, Bracket);
            node->children[0] = pop();
            node->children[1] = parseExpressionStatement(Bracket);
            push(node);
            match(']');
        } else {
            printf("%d: invalid token: %s\n", line, tokenString);
            exit(1);
        }
    }
    return pop();
}

void parse(void) {
    token = 1;
    int baseType;
    char* idName;
    struct treeNode* temp, * last;
    getToken();
    while (token > 0) {
        if (token == INT || token == CHAR) {
            // 获取标识符类型
            baseType = parseBaseType();
            // 获取标识符名
            idName = tokenString;
            checkNewId();
            match(Id);
            symbolPtr->type = baseType;
            if (token == '(') {
                // 函数类型
                symbolPtr->class = Fun;
                temp = parseFunction(baseType, idName);
                match('}');
            } else {
                // 变量类型
                symbolPtr->class = Glo;
                temp = parseDeclaration(baseType, idName, 0);
                match(';');
            }
        } else if (token == VOID) {
            // 获取标识符类型
            baseType = VOID;
            match(VOID);
            // 获取标识符名
            idName = tokenString;
            checkNewId();
            match(Id);
            symbolPtr->type = baseType;
            temp = parseFunction(baseType, idName);
        } else {
            printf("line %d: invalid statement\n", line);
            exit(1);
        }
        if (root != NULL) {
            last = root;
            while (last->sibling != NULL) {
                last = last->sibling;
            }
            last->sibling = temp;
        } else {
            root = temp;
        }
    }
}

