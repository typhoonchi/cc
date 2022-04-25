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
        printf("line %d: invalid identifier\n", line);
        exit(1);
    }
    if (symbolPtr->class != 0) {
        printf("line %d: duplicate declaration\n", line);
        exit(1);
    }
}

void checkLocalId() {
    if (token != Id) {
        printf("line %d: invalid identifier\n", line);
        exit(1);
    }
    if (symbolPtr->class == Loc) {
        printf("line %d: duplicate declaration\n", line);
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

/*
 * int res
 *
 * (int a, int b[], char c){
 * int ret;
 * a = b[1] + c;
 * add(1,2);
 * return a;
 *
 * }
 * */
struct treeNode* parseFunction(int type, char* name){
    struct treeNode* t = createNode();
    match('(');
    t->type = type;
    t->name = name;
    t->nodeType = Function;
    // 匹配参数列表
    if (token == ')') {
        t->children[0] = createNode();
        t->children[0]->nodeType = ParameterStatement;
        t->children[0]->type = VOID;
    } else {
        t->children[0] = parseParameters();
    }
    match(')');
    match('{');
    // 匹配函数体
    t->children[1] = parseFunctionBody();
    symbolPtr = symbolTable;
    while (symbolPtr->token != 0) {
        if (symbolPtr->class == Loc) {
            recoverGlobalId();
        }
        symbolPtr++;
    }
    return t;
}

/*
 * (
 *
 * int a, int b[], char c
 *
 * )
 * */
struct treeNode* parseParameters(void){
    struct treeNode* t = createNode();
    char* idName;
    t->type = parseBaseType();
    idName = tokenString;
    match(Id);
    t->name = idName;
    t->nodeType = ParameterStatement;
    if (token == Bracket) {
        match(Bracket);
        match(']');
    }
    if (token == ',') {
        match(',');
        t->sibling = parseParameters();
    }
    return t;
}
/*
 * {
 *
 * int ret;
 * a = b[1] + c;
 * add(1,2);
 * return a;
 *
 * }
 * */
struct treeNode* parseFunctionBody(void){
    struct treeNode* t = NULL;
    struct treeNode* last;
    int type;
    char* idName;
    while (token == INT || token == CHAR) {
        type = parseBaseType();
        idName = tokenString;
        checkLocalId();
        hideGlobalId();
        symbolPtr->class = Loc;
        symbolPtr->type = type;
        match(Id);
        if (t == NULL) {
            t = parseDeclaration(type, idName, 1);
        } else {
            last = t;
            while (last->sibling != NULL) {
                last = last->sibling;
            }
            last->sibling = parseDeclaration(type,idName,1);
        }
        match(';');
    }
    while (token != '}') {
        if (t ==  NULL) {
            t = parseStatement();
        } else {
            last = t;
            while (last->sibling != NULL) {
                last = last->sibling;
            }
            last->sibling = parseStatement();
        }
    }
    return t;
}

/*
 * int a
 *
 * [5], b, c, d
 *
 * ;
 * */
struct treeNode* parseDeclaration(int type, char* name, int mode){
    struct treeNode* t = createNode();
    char* idName;
    t->type = type;
    t->name = name;
    t->nodeType = DeclareStatement;
    if (token == Bracket) {
        if (symbolPtr->type == INT) {
            symbolPtr->type = INTARRAY;
            t->type = INTARRAY;
        } else if (symbolPtr->type == CHAR) {
            symbolPtr->type = CHARARRAY;
            t->type = CHARARRAY;
        }
        match(Bracket);
        t->isArray = true;
        t->size = tokenValue;
        match(Num);
        match(']');
    }
    if (token == ',') {
        match(',');
        idName = tokenString;
        if (mode) {
            checkLocalId();
            hideGlobalId();
            symbolPtr->class = Loc;
            symbolPtr->type = type;
        }
        match(Id);
        t->sibling = parseDeclaration(t->type,idName, mode);
    }
    return t;
}
/*
 * a = b[1] + c;
 * if (a > 0) {
 *      c = 1;
 * }
 * while (a) {
 *      a--;
 * }
 * add(1,2);
 * return a;
 *
 * */
struct treeNode* parseStatement() {
    struct treeNode* t = NULL;
    if (token == IF) {
        t = parseIfStatement();
    } else if (token == WHILE) {
        t = parseWhileStatement();
    } else if (token == RETURN) {
        t = parseReturnStatement();
    } else if (token == ';') {
        match(';');
    } else {
        t = parseExpressionStatement(Assign);
    }
    return t;
}

/*
 * if (a > 0) {
 *      c = 1;
 * } else {
 *      c = 2;
 * }
 * */
struct treeNode* parseIfStatement(){
    struct treeNode* t = createNode();
    struct treeNode* last;
    t->nodeType = IfStatement;
    match(IF);
    match('(');
    t->children[0] = parseExpressionStatement(Assign);
    match(')');
    match('{');
    while (token != '}') {
        if (t->children[1] == NULL) {
            t->children[1] = parseStatement();
        } else {
            last = t->children[1];
            while (last->sibling != NULL) {
                last = last->sibling;
            }
            last->sibling = parseStatement();
        }
    }
    match('}');
    if (token == ELSE) {
        match(ELSE);
        match('{');
        while (token != '}') {
            if (t->children[2] == NULL) {
                t->children[2] = parseStatement();
            } else {
                last = t->children[2];
                while (last->sibling != NULL) {
                    last = last->sibling;
                }
                last->sibling = parseStatement();
            }
        }
        match('}');
    }
    return t;
}

/*
 * while (a) {
 *      a--;
 * }
 * */
struct treeNode* parseWhileStatement(){
    struct treeNode* t = createNode();
    struct treeNode* last;
    t->nodeType = WhileStatement;
    match(WHILE);
    match('(');
    t->children[0] = parseExpressionStatement(Assign);
    match(')');
    match('{');
    while (token != '}') {
        if (t->children[1] == NULL) {
            t->children[1] = parseStatement();
        } else {
            last = t->children[1];
            while (last->sibling != NULL) {
                last = last->sibling;
            }
            last->sibling = parseStatement();
        }
    }
    match('}');
    return t;
}

/*
 * return a + b;
 * */
struct treeNode* parseReturnStatement(){
    struct treeNode* t = createNode();
    t->nodeType = ReturnStatement;
    match(RETURN);
    t->children[0] = parseExpressionStatement(Assign);
    match(';');
    return t;
}

/*
 * a = b + c * d;
 * ch = 'a';
 * a++;
 * res = add(1,2) + c & d;
 * !(a > 0) && (b <= 0)
 * 3 <= a
 * ((a = 1) >= 0) || !((b = 2) < 0)
 * */
struct treeNode* parseExpressionStatement(int operator){
    struct treeNode* t;
    struct treeNode* temp;
    if (token == Num) {
        t = createNode();
        t->nodeType = ExpressStatement;
        t->type = INT;
        t->value = tokenValue;
        t->valType = Constant;
        symbolStack[top++] = t;
        match(Num);
    } else if (token == Id) {
        t = createNode();
        temp = t;
        t->nodeType = ExpressStatement;
        t->name = tokenString;
        t->valType = Identifier;
        match(Id);
        if (token == '(') {
            if (symbolPtr->class != Fun) {
                printf("line %d: invalid function call\n",line);
                exit(1);
            }
            t->valType = Call;
            match('(');
            while (token != ')') {
                if (t->children[0] != NULL) {
                    temp = t->children[0];
                    while (temp->sibling != NULL) {
                        temp = temp->sibling;
                    }
                    temp->sibling = parseExpressionStatement(Assign);
                } else {
                    t->children[0] = parseExpressionStatement(Assign);
                }
                if (token == ',') {
                    match(',');
                }
            }
            match(')');
        } else {
            if (symbolPtr->class == Glo) {
                t->value = symbolPtr->gValue;
            } else if (symbolPtr->class == Loc) {
                t->value = symbolPtr->value;
            }
        }
        symbolStack[top++] = t;
    } else if (token == '(') {
        match('(');
        t = parseExpressionStatement(Assign);
        match(')');
        symbolStack[top++] = t;
    } else if (token == '!') {
        t = createNode();
        t->valType = Operator;
        t->opType = '!';
        match('!');
        t->children[0] = parseExpressionStatement(Inc);
        symbolStack[top++] = t;
    } else {
        printf("line %d: invalid expression\n", line);
        exit(1);
    }
    while (token >= operator) {
        if (token == Assign) {
            match(Assign);
            t = createNode();
            t->nodeType = ExpressStatement;
            t->valType = Operator;
            t->opType = Assign;
            t->children[0] = symbolStack[--top];
            t->children[1] = parseExpressionStatement(Assign);
            symbolStack[top++] = t;
        } else if (token == Lor) {
            match(Lor);
            t = createNode();
            t->nodeType = ExpressStatement;
            t->valType = Operator;
            t->opType = Lor;
            t->children[0] = symbolStack[--top];
            t->children[1] = parseExpressionStatement(Land);
            symbolStack[top++] = t;
        } else if (token == Land) {
            match(Land);
            t = createNode();
            t->nodeType = ExpressStatement;
            t->valType = Operator;
            t->opType = Land;
            t->children[0] = symbolStack[--top];
            t->children[1] = parseExpressionStatement(Or);
            symbolStack[top++] = t;
        } else if (token == Or) {
            match(Or);
            t = createNode();
            t->nodeType = ExpressStatement;
            t->valType = Operator;
            t->opType = Or;
            t->children[0] = symbolStack[--top];
            t->children[1] = parseExpressionStatement(Xor);
            symbolStack[top++] = t;
        } else if (token == Xor) {
            match(Xor);
            t = createNode();
            t->nodeType = ExpressStatement;
            t->valType = Operator;
            t->opType = Xor;
            t->children[0] = symbolStack[--top];
            t->children[1] = parseExpressionStatement(And);
            symbolStack[top++] = t;
        } else if (token == And) {
            match(And);
            t = createNode();
            t->nodeType = ExpressStatement;
            t->valType = Operator;
            t->opType = And;
            t->children[0] = symbolStack[--top];
            t->children[1] = parseExpressionStatement(Eq);
            symbolStack[top++] = t;
        } else if (token == Eq) {
            match(Eq);
            t = createNode();
            t->nodeType = ExpressStatement;
            t->valType = Operator;
            t->opType = Eq;
            t->children[0] = symbolStack[--top];
            t->children[1] = parseExpressionStatement(Ne);
            symbolStack[top++] = t;
        } else if (token == Ne) {
            match(Ne);
            t = createNode();
            t->nodeType = ExpressStatement;
            t->valType = Operator;
            t->opType = Ne;
            t->children[0] = symbolStack[--top];
            t->children[1] = parseExpressionStatement(Lt);
            symbolStack[top++] = t;
        } else if (token == Lt) {
            match(Lt);
            t = createNode();
            t->nodeType = ExpressStatement;
            t->valType = Operator;
            t->opType = Lt;
            t->children[0] = symbolStack[--top];
            t->children[1] = parseExpressionStatement(Gt);
            symbolStack[top++] = t;
        } else if (token == Gt) {
            match(Gt);
            t = createNode();
            t->nodeType = ExpressStatement;
            t->valType = Operator;
            t->opType = Gt;
            t->children[0] = symbolStack[--top];
            t->children[1] = parseExpressionStatement(Le);
            symbolStack[top++] = t;
        } else if (token == Le) {
            match(Le);
            t = createNode();
            t->nodeType = ExpressStatement;
            t->valType = Operator;
            t->opType = Le;
            t->children[0] = symbolStack[--top];
            t->children[1] = parseExpressionStatement(Ge);
            symbolStack[top++] = t;
        } else if (token == Ge) {
            match(Ge);
            t = createNode();
            t->nodeType = ExpressStatement;
            t->valType = Operator;
            t->opType = Ge;
            t->children[0] = symbolStack[--top];
            t->children[1] = parseExpressionStatement(Shl);
            symbolStack[top++] = t;
        } else if (token == Shl) {
            match(Shl);
            t = createNode();
            t->nodeType = ExpressStatement;
            t->valType = Operator;
            t->opType = Shl;
            t->children[0] = symbolStack[--top];
            t->children[1] = parseExpressionStatement(Shr);
            symbolStack[top++] = t;
        } else if (token == Shr) {
            match(Shr);
            t = createNode();
            t->nodeType = ExpressStatement;
            t->valType = Operator;
            t->opType = Shr;
            t->children[0] = symbolStack[--top];
            t->children[1] = parseExpressionStatement(Add);
            symbolStack[top++] = t;
        } else if (token == Add) {
            match(Add);
            t = createNode();
            t->nodeType = ExpressStatement;
            t->valType = Operator;
            t->opType = Add;
            t->children[0] = symbolStack[--top];
            t->children[1] = parseExpressionStatement(Mul);
            symbolStack[top++] = t;
        } else if (token == Sub) {
            match(Sub);
            t = createNode();
            t->nodeType = ExpressStatement;
            t->valType = Operator;
            t->opType = Sub;
            t->children[0] = symbolStack[--top];
            t->children[1] = parseExpressionStatement(Mul);
            symbolStack[top++] = t;
        } else if (token == Mul) {
            match(Mul);
            t = createNode();
            t->nodeType = ExpressStatement;
            t->valType = Operator;
            t->opType = Mul;
            t->children[0] = symbolStack[--top];
            t->children[1] = parseExpressionStatement(Bracket);
            symbolStack[top++] = t;
        } else if (token == Div) {
            match(Div);
            t = createNode();
            t->nodeType = ExpressStatement;
            t->valType = Operator;
            t->opType = Div;
            t->children[0] = symbolStack[--top];
            t->children[1] = parseExpressionStatement(Bracket);
            symbolStack[top++] = t;
        } else if (token == Mod) {
            match(Mod);
            t = createNode();
            t->nodeType = ExpressStatement;
            t->valType = Operator;
            t->opType = Mod;
            t->children[0] = symbolStack[--top];
            t->children[1] = parseExpressionStatement(Bracket);
            symbolStack[top++] = t;
        } else if (token == Bracket) {
            match(Bracket);
            t = createNode();
            t->nodeType = ExpressStatement;
            t->valType = Operator;
            t->opType = Bracket;
            t->children[0] = symbolStack[--top];
            t->children[1] = parseExpressionStatement(Bracket);
            symbolStack[top++] = t;
            match(']');
        } else {
            printf("%d: invalid token\n", line);
            exit(1);
        }
    }
    return symbolStack[--top];
}

void parse(void) {
    token = 1;
    int baseType;
    char* idName;
    struct treeNode* temp, * last;
    getToken();
    while (token > 0) {
        temp = createNode();
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

