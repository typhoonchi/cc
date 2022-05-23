//
// Created by zhangyukun on 2022/4/11.
//
#include "globals.h"
#include "utility.h"
#include "parser.h"
#include "scanner.h"

static sTreeNode** expressionStack;             // 表达式栈
static int top;                                 // 栈顶
static long long ibp;                           //
static long long num;                           // 计数器

static void initExpressionStack();
static sTreeNode* createNode(eStatementType statementType, eExpressionType expressionType, eClass classType, eToken operatorType);
static void match(int expected);
static void checkGlobalId();
static void checkLocalId();
static void checkDeclaredId();
static void hideGlobalId();
static void recoverGlobalId();
static int parseBaseType();
static void push(sTreeNode* node);
static sTreeNode* pop();
static sTreeNode* parseFunction(int type, char* name);
static sTreeNode* parseParameters();
static sTreeNode* parseFunctionBody();
static sTreeNode* parseDeclaration(int type, char* name, int mode);
static sTreeNode* parseStatement();
static sTreeNode* parseIfStatement();
static sTreeNode* parseWhileStatement();
static sTreeNode* parseReturnStatement();
static sTreeNode* parseExpressionStatement(int operator);

/**
 * @brief 语法分析
 *
 * 根据词法分析传入的 token, tokenValue, tokenString 进行语法分析
 *
 * @param   void
 * @return  void
 * */
void parse() {
    int baseType;                   // 存放标识符类型
    char* idName;                   // 存放标识符名称
    sTreeNode* node, * lastSibling; // 临时节点

    // 初始化表达式栈
    initExpressionStack();
    // 取 token
    getToken();
    // 持续语法分析直到结束或出错
    while (token > 0) {
        // 判断是否是全局变量声明或函数定义
        if (token == INT || token == CHAR || token == VOID) {
            // 处理全局变量声明或函数定义
            // 获取标识符类型
            baseType = parseBaseType();
            // 获取标识符名
            idName = tokenString;
            // 检查是否重复声明全局变量或重复定义函数
            checkGlobalId();
            symbolPtr->type = baseType;
            match(Id);
            // 判断是否是函数定义
            if ((baseType == Void) || (token == '(')) {
                // 处理函数定义
                symbolPtr->class = Fun;
                if (!memcmp("main", idName, strlen("main"))) {
                    mainPtr = &(symbolPtr->address);
                }
                node = parseFunction(baseType, idName);
            } else {
                // 处理全局变量声明
                symbolPtr->class = Glo;
                // 记录全局变量在数据段中地址
                symbolPtr->address = (long long)data;
                data++;
                node = parseDeclaration(baseType, idName, 0);
                match(';');
            }
        } else {
            printErrorInformation("Get Invalid Statement", NULL);
            exit(1);
        }
        // 将节点加入根节点上
        if (root != NULL) {
            // 根节点非空, 加到最后一个非空节点的兄弟节点上
            while (lastSibling->sibling != NULL) {
                lastSibling = lastSibling->sibling;
            }
            lastSibling->sibling = node;
        } else {
            // 根节点为空, 当前节点作为根节点
            root = node;
            lastSibling = root;
        }
    }
}

/**
 * @brief 初始化表达式节点栈
 *
 * 为解析表达式分配节点栈空间, 并初始化栈顶指针
 *
 * @param   void
 * @return  void
 * */
static void initExpressionStack() {
    // 分配表达式栈存储区
    expressionStack = (sTreeNode**) malloc(STACK_SIZE * sizeof(sTreeNode*));
    // 判断初始化是否成功
    if (expressionStack == NULL) {
        // 初始化失败, 打印错误信息
        printErrorInformation("Fail to Init", NULL);
        exit(1);
    }
    memset(expressionStack, 0, STACK_SIZE * sizeof(sTreeNode*));
    // 初始化栈顶
    top = 0;
}

/**
 * @brief 创建抽象语法树节点
 *
 * 创建 AST 节点, 并初始化内部值
 *
 * @param   statementType   语句类型 If, While, Return,...
 * @param   expressionType  表达式类型 Operator, Constant, Identifier, Call
 * @param   operatorType    运算符类型 << >> ...
 * @return  node            抽象语法树节点指针
 * */
static sTreeNode* createNode(eStatementType statementType, eExpressionType expressionType, eClass classType, eToken operatorType) {
    sTreeNode* node = (sTreeNode*) malloc(sizeof(sTreeNode));     // 创建空节点

    // 判断 AST 节点是否创建成功
    if (node == NULL) {
        // 创建节点失败
        printErrorInformation("Fail to Create AST Node", NULL);
        exit(1);
    }
    // 初始化节点
    memset(node, 0, sizeof(sTreeNode));
    // 初始化语句类型
    node->statementType = statementType;
    // 初始化孩子节点
    for (int i = 0; i < MAX_CHILDREN; i++) {
        node->children[i] = NULL;
    }
    // 初始化兄弟结点
    node->sibling = NULL;
    //初始化数组相关内容
    node->size = 1;
    // 初始化表达式类型 , 仅当语句类型为表达式时有效
    node->expressionType = expressionType;
    // 初始化标识符类型 , 仅当表达式类型为 Id 或 Call 时有效
    node->classType = classType;
    // 初始化运算符类型 , 仅当表达式类型为 Operator 时有效
    node->operatorType = operatorType;
    // 初始化常量值与变量名
    node->value = 0;
    node->name = NULL;
    return node;
}

/**
 * @brief 匹配 token
 *
 * 判断当前 token 是否与预期 token 匹配
 *
 * @param   expected  期待词素
 * @return  void
 * */
void match(int expected) {
    if (token != expected) {
        // token 不匹配
        printErrorInformation("Find Unexpect Token",tokenString);
        exit(1);
    }
    getToken();
}


/**
 * @brief 检查当前标识符是否是已声明的全局标识符
 *
 * 检查获取 token 是否为 Id, 并且检查对应符号表项的类型是否为初始值, token 为 Id 并且表项为初始值,
 * 则说明当前标识符是未声明过的全局标识符.
 *
 * @note 在声明全局变量或定义函数时检查
 *
 * @param   void
 * @return  void
 * */
void checkGlobalId() {
    if (token != Id) {
        // 非标识符
        printErrorInformation("Get Invalid Identifier",tokenString);
        exit(1);
    }
    if (symbolPtr->class == Glo) {
        // 已声明全局标识符
        printErrorInformation("Get Duplicate Declaration",tokenString);
        exit(1);
    }
}

/**
 * @brief 检查当前标识符是否是已声明的局部变量
 *
 * 检查获取 token 是否为 Id, 并且检查对应符号表项的类型是否为 Loc, token 为 Id 并且表项为 Loc,
 * 则说明当前标识符是已声明过的局部变量
 *
 * @note 在声明局部变量时检查
 *
 * @param   void
 * @return  void
 * */
void checkLocalId() {
    if (token != Id) {
        // 非标识符
        printErrorInformation("Get Invalid Identifier",tokenString);
        exit(1);
    }
    if (symbolPtr->class == Loc) {
        // 已声明局部变量
        printErrorInformation("Get Duplicate Declaration",tokenString);
        exit(1);
    }
}

/**
 * @brief 检查是否是已声明的标识符
 *
 * 检查获取 token 是否为 Id, 并且检查对应符号表项的类型是否为初始值, token 为 Id 并且表项不为初始值,
 * 则说明当前标识符是已声明过的标识符
 *
 * @note 在使用变量或调用函数时检查
 *
 * @param   void
 * @return  void
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

/**
 * @brief 隐藏全局变量
 *
 * 遮罩全局变量
 *
 * @param   void
 * @return  void
 * */
void hideGlobalId(){
    symbolPtr->gloClass = symbolPtr->class;
    symbolPtr->gloType = symbolPtr->type;
    symbolPtr->gloAddress = symbolPtr->address;
}

/**
 * @brief 恢复全局变量
 *
 * 恢复被遮罩的全局变量
 *
 * @param   void
 * @return  void
 * */
void recoverGlobalId(){
    symbolPtr->class = symbolPtr->gloClass;
    symbolPtr->type = symbolPtr->gloType;
    symbolPtr->address = symbolPtr->gloAddress;
}

/**
 * @brief 解析基本变量类型
 *
 * 检查基本变量类型, 并返回对应的值
 *
 * @param   void
 * @return  type    基本变量类型: Int, Char
 * */
int parseBaseType(){
    if (token == CHAR) {
        match(CHAR);
        return Char;
    } else if (token == INT) {
        match(INT);
        return Int;
    } else if (token == VOID) {
        match(VOID);
        return Void;
    } else {
        printErrorInformation("Get Base Type Error",NULL);
        exit(1);
    }
}

/**
 * @brief 入栈
 *
 * 将 AST 节点指针放入表达式栈
 *
 * @param   node    抽象语法树节点指针
 * @return  void
 * */
void push(sTreeNode* node) {
    expressionStack[top] = node;
    top++;
}

/**
 * @brief 出栈
 *
 * 返回栈顶的表达式节点指针
 *
 * @param   void
 * @return  node    抽象语法树节点指针
 *
 * */
sTreeNode* pop() {
    if (top == 0) {
        printErrorInformation("Get Expression Stack Error", NULL);
        exit(1);
    }
    top--;
    return expressionStack[top];
}

/**
 * @brief 解析函数
 *
 * 解析函数定义, 返回指向函数名的 AST 节点指针
 *
 * @param   type    函数返回值
 * @param   name    函数名
 * @return  node    抽象语法树节点指针, 指向函数定义子树
 * */
sTreeNode* parseFunction(int type, char* name){
    sTreeNode* node = createNode(Function, 0, 0, 0);   // 创建函数根节点

    // 记录函数返回值类型与函数名
    node->identifierType = type;
    node->name = name;
    // 记录符号表中存放函数在代码段中地址的数据的地址, 代码生成时进行回填
    node->value = (long long)&(symbolPtr->address);
    // 重置计数器
    num = 0;
    match('(');
    // 解析参数列表
    if (token == ')') {
        // 空参列表 void
        node->children[0] = createNode(ParameterStatement, 0, 0, 0);
        node->children[0]->identifierType = Void;
    } else {
        // 有参列表 解析参数列表
        node->children[0] = parseParameters();
    }
    num++;
    ibp = num;
    match(')');
    match('{');
    // 解析函数体
    node->children[1] = parseFunctionBody();
    // 恢复全局变量
    symbolPtr = symbolTable;
    while (symbolPtr->token != 0) {
        // 判断是否是局部变量
        if (symbolPtr->class == Loc) {
            // 恢复局部变量到声明该变量前的状态
            recoverGlobalId();
        }
        symbolPtr++;
    }
    match('}');
    return node;
}


/**
 * @brief 解析参数列表
 *
 * 解析参数列表中参数信息
 *
 * @param   void
 * @return  node    抽象语法树节点指针, 指向参数列表子树
 * */
sTreeNode* parseParameters(){
    sTreeNode* node = createNode(ParameterStatement, 0, 0, 0);                 // 参数列表根节点
    // 获取参数类型
    node->identifierType = parseBaseType();
    // 获取参数名称
    node->name = tokenString;
    // 形参作为局部变量使用
    checkLocalId();
    hideGlobalId();
    match(Id);
    // 处理数组参数
    while ((token != ',') && (token!= ')')) {
        if (token == Bracket) {
            // 匹配数组参数
            match(Bracket);
            node->identifierType += Ptr;
            // 过滤数组参数中的常量
            if (token == Num) {
                match(Num);
            }
            match(']');
        } else {

        }
    }
    // 更新符号表中局部变量信息
    symbolPtr->class = Loc;
    symbolPtr->type = node->identifierType;
    symbolPtr->address = num;
    num++;
    if (token == ',') {
        // 匹配后续参数
        match(',');
        // 后续参数作为兄弟节点, 依次加入到参数列表根节点中
        node->sibling = parseParameters();
    }
    // 返回参数列表根节点
    return node;
}

/**
 * @brief 解析函数体
 *
 * 解析函数体中语句
 *
 * @param   void
 * @return  node    抽象语法树节点指针, 指向函数体子树
 * */
sTreeNode* parseFunctionBody(){
    sTreeNode* node = NULL;         // 函数体根节点
    sTreeNode* lastSibling;         // 临时节点
    int baseType;                   // 标识符类别
    char* identifierName;            // 标识符名称

    // 持续解析局部变量声明语句
    while (token == INT || token == CHAR) {
        // 获取局部变量类型
        baseType = parseBaseType();
        // 获取局部变量名
        identifierName = tokenString;
        // 检查是否已声明过该局部变量
        checkLocalId();
        // 隐藏全局变量
        hideGlobalId();
        symbolPtr->class = Loc;
        symbolPtr->type = baseType;
        num++;
        symbolPtr->address = num;
        match(Id);
        // 将节点加入函数体根节点上
        if (node != NULL) {
            // 函数体根节点非空, 加到最后一个非空节点的兄弟节点上
            while (lastSibling->sibling != NULL) {
                lastSibling = lastSibling->sibling;
            }
            lastSibling->sibling = parseDeclaration(baseType,identifierName,1);
        } else {
            // 函数体根节点为空, 当前节点作为函数体根节点
            node = parseDeclaration(baseType, identifierName, 1);
            lastSibling = node;
        }
        match(';');
    }
    // 持续解析非声明语句
    while (token != '}') {
        // 将节点加入函数体根节点上
        if (node !=  NULL) {
            // 函数体根节点非空, 加到最后一个非空节点的兄弟节点上
            while (lastSibling->sibling != NULL) {
                lastSibling = lastSibling->sibling;
            }
            lastSibling->sibling = parseStatement();
        } else {
            // 函数体根节点为空, 当前节点作为函数体根节点
            node = parseStatement();
            lastSibling = node;
        }
    }
    // 返回函数体根节点
    return node;
}

/**
 * @brief 解析声明语句
 *
 * 解析全局变量与局部变量声明语句, 根据不同类型的变量采用不同的声明方式
 *
 * @param   type    变量类型
 * @param   name    变量名
 * @param   mode    声明模式 0 全局模式  1 局部模式
 * @return  node    抽象语法树节点指针, 指向一条声明语句子树
 * */
sTreeNode* parseDeclaration(int type, char* name, int mode){
    sTreeNode* node = createNode(DeclareStatement, 0, 0, 0);       // 声明语句节点
    sTreeNode* lastChild;
    char* identifierName = NULL;         // 变量名
    long long size = 1;                 // 数组大小
    long long* base;                    // 数据段指针, 声明数组变量时用于定位数据段初始位置

    // 记录变量名
    node->name = name;
    // 记录变量地址
    node->value = symbolPtr->address;
    // 持续解析变量声明
    while ((token != ',') && (token != ';')) {
        // 解析数组声明
        if (token == Bracket) {
            symbolPtr->type += Ptr;
            match(Bracket);
            if (mode) {
                num += tokenValue;
            } else {
                base = (data - 1);
                for (int i = 1; i <= size; i++) {
                    *(base - size + i) = (long long)data;
                    data += tokenValue;
                }
            }
            size *= tokenValue;
            if (node->children[0] != NULL) {
                while (lastChild->children[0] != NULL) {
                    lastChild = lastChild->children[0];
                }
                lastChild->children[0] = createNode(DeclareStatement, 0, 0, 0);
                lastChild->children[0]->value = tokenValue;
            } else {
                node->children[0] = createNode(DeclareStatement, 0, 0, 0);
                node->children[0]->value = tokenValue;
                lastChild = node->children[0];
            }
            match(Num);
            match(']');
        }
    }
    // 记录变量类型与数组大小
    node->identifierType = symbolPtr->type;
    node->size = size;
    // 处理逗号及后续变量声明
    if (token == ',') {
        // 解析逗号
        match(',');
        // 记录变量名
        identifierName = tokenString;
        // 判断是全局变量声明还是局部变量声明
        if (mode) {
            // 局部变量声明
            checkLocalId();
            hideGlobalId();
            symbolPtr->class = Loc;
            symbolPtr->type = type;
            num++;
            symbolPtr->address = num;
        } else {
            // 全局变量声明
            symbolPtr->class = Glo;
            symbolPtr->type = type;
            symbolPtr->address = (long long)data;
            data++;
        }
        match(Id);
        // 继续解析声明语句
        node->sibling = parseDeclaration(node->identifierType,identifierName, mode);
    }
    // 返回声明语句根节点
    return node;
}

/**
 * @brief 解析语句
 *
 * 解析 If 语句, While 语句, Return 语句, 空语句和表达式语句
 *
 * @param   void
 * @return  node    抽象语法树节点指针, 指向语句子树
 * */
sTreeNode* parseStatement() {
    sTreeNode* node = NULL;         // 语句根节点

    // 判断语句类型
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

/**
 * @brief 解析 If 语句
 *
 * 解析 If 语句
 *
 * @param   void
 * @return  node    抽象语法树节点指针, 指向 If 语句子树
 * */
sTreeNode* parseIfStatement(){
    sTreeNode* node = createNode(IfStatement, 0, 0, 0);    // If 语句根节点
    sTreeNode* lastSibling;     // 临时节点

    match(IF);
    match('(');
    // 解析条件表达式
    node->children[0] = parseExpressionStatement(Assign);
    match(')');
    match('{');
    // 持续解析成功分支语句
    while (token != '}') {
        // 将节点加入 If 语句根节点上
        if (node->children[1] != NULL) {
            // If 语句根节点非空, 加到最后一个非空节点的兄弟节点上
            while (lastSibling->sibling != NULL) {
                lastSibling = lastSibling->sibling;
            }
            lastSibling->sibling = parseStatement();
        } else {
            // If 语句根节点为空, 当前节点作为 If 语句根节点
            node->children[1] = parseStatement();
            lastSibling = node->children[1];
        }
    }
    match('}');
    // 解析可选 Else 语句
    if (token == ELSE) {
        // 存在 Else 语句
        match(ELSE);
        match('{');
        // 持续解析失败分支语句
        while (token != '}') {
            // 将节点加入 If 语句根节点上
            if (node->children[2] != NULL) {
                // If 语句根节点非空, 加到最后一个非空节点的兄弟节点上
                while (lastSibling->sibling != NULL) {
                    lastSibling= lastSibling->sibling;
                }
                lastSibling->sibling = parseStatement();
            } else {
                // If 语句根节点为空, 当前节点作为 If 语句根节点
                node->children[2] = parseStatement();
                lastSibling = node->children[2];
            }
        }
        match('}');
    }
    // 返回 If 语句根节点
    return node;
}

/**
 * @brief 解析 While 语句
 *
 * 解析 While 语句
 *
 * @param   void
 * @return  node    返回抽象语法树节点指针, 指向 While 语句子树
 * */
sTreeNode* parseWhileStatement(){
    sTreeNode* node = createNode(WhileStatement, 0, 0, 0);     // While 语句根节点
    sTreeNode* lastSibling;     // 临时节点

    match(WHILE);
    match('(');
    // 解析条件语句
    node->children[0] = parseExpressionStatement(Assign);
    match(')');
    match('{');
    // 持续解析循环体语句
    while (token != '}') {
        // 将节点加入 While 语句根节点上
        if (node->children[1] != NULL) {
            // While 语句根节点非空, 加到最后一个非空节点的兄弟节点上
            while (lastSibling->sibling != NULL) {
                lastSibling = lastSibling->sibling;
            }
            lastSibling->sibling = parseStatement();
        } else {
            // While 语句根节点为空, 当前节点作为 While 语句根节点
            node->children[1] = parseStatement();
            lastSibling = node->children[1];
        }
    }
    match('}');
    // 返回 While 语句根节点
    return node;
}

/**
 * @brief 解析 Return 语句
 *
 * 解析 Return 语句
 *
 * @param   void
 * @return  node    返回抽象语法树节点指针, 指向 Return 语句子树
 * */
sTreeNode* parseReturnStatement(){
    sTreeNode* node = createNode(ReturnStatement,0, 0, 0);    // Return 语句根节点

    match(RETURN);
    // 解析返回表达式
    node->children[0] = parseExpressionStatement(Assign);
    match(';');
    // 返回 Return 语句根节点
    return node;
}

/**
 * @brief 解析表达式语句
 *
 * 解析表达式语句
 *
 * @param   operator    当前运算符
 * @return  node        返回抽象语法树节点指针, 指向表达式语句子树
 * */
sTreeNode* parseExpressionStatement(int operator){
    sTreeNode* node;            // 表达式语句节点
    sTreeNode* lastSibling;     // 临时节点

    // 处理运算分量
    if (token == Num) {
        // 处理数值常量
        // 为数值常量创建节点
        node = createNode(ExpressStatement, Constant, 0, 0);
        node->identifierType = Int;
        node->value = tokenValue;
        push(node);
        match(Num);
    } else if (token == Char){
        // 处理字符常量
        // 为字符常量创建节点
        node = createNode(ExpressStatement, Constant, 0, 0);
        node->identifierType = Char;
        node->value = tokenValue;
        push(node);
        match(Char);
    } else if (token == Char + Ptr) {
        // 处理字符串常量
        // 为字符串常量创建节点
        node = createNode(ExpressStatement, Constant, 0, 0);
        node->identifierType = Char + Ptr;
        node->value = tokenValue;
        push(node);
        match(Char + Ptr);
    } else if (token == Id) {
        // 处理标识符
        // 检查标识符是否已声明
        checkDeclaredId();
        // 判断是函数调用还是变量
        if (symbolPtr->class == Fun || symbolPtr->class == Sys) {
            // 处理函数调用
            node = createNode(ExpressStatement, Call, symbolPtr->class, 0);
            // 记录函数名, 返回值类型与代码段地址
            node->name = tokenString;
            node->identifierType = symbolPtr->type;
            node->value = (long long)&(symbolPtr->address);
            match(Id);
            match('(');
            // 持续解析参数列表
            while (token != ')') {
                // 将节点加入函数调用根节点上
                if (node->children[0] != NULL) {
                    // 函数调用根节点非空, 加到最后一个非空节点的兄弟节点上
                    while (lastSibling->sibling != NULL) {
                        lastSibling = lastSibling->sibling;
                    }
                    lastSibling->sibling = parseExpressionStatement(Assign);
                } else {
                    // 函数调用根节点为空, 当前节点作为函数调用根节点
                    node->children[0] = parseExpressionStatement(Assign);
                    lastSibling = node->children[0];
                }
                if (token != ')') {
                    match(',');
                }
            }
            match(')');
        } else {
            // 处理变量
            // 为标识符创建节点
            node = createNode(ExpressStatement, Variable, symbolPtr->class, 0);
            // 记录变量名, 变量类型与变量地址
            node->name = tokenString;
            node->identifierType = symbolPtr->type;
            node->value = symbolPtr->address;
            match(Id);
        }
        // 判断是否出现非法情况
        if (token == Id){
            // 标识符后跟标识符, 非法情况
            printErrorInformation("Get Invalid Expression Statement", NULL);
            exit(1);
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
        node = createNode(ExpressStatement, Operator, 0, '!');
        match('!');
        node->children[0] = parseExpressionStatement(Inc);
        push(node);
    } else {
        // 处理错误信息
        printErrorInformation("Get Invalid Expression", NULL);
        exit(1);
    }
    // 优先级爬山处理表达式运算
    while (token >= operator) {
        // 仅处理优先级不低于当前优先级的运算子表达式
        if (token == Assign) {
            // 处理赋值 =
            match(Assign);
            node = createNode(ExpressStatement, Operator, 0, Assign);
            // 处理左值
            node->children[0] = pop();
            // 处理右值子表达式
            node->children[1] = parseExpressionStatement(Assign);
            push(node);
        } else if (token == Lor) {
            // 处理逻辑或 ||
            match(Lor);
            node = createNode(ExpressStatement, Operator, 0, Lor);
            node->children[0] = pop();
            node->children[1] = parseExpressionStatement(Land);
            push(node);
        } else if (token == Land) {
            // 处理逻辑与 &&
            match(Land);
            node = createNode(ExpressStatement, Operator, 0, Land);
            node->children[0] = pop();
            node->children[1] = parseExpressionStatement(Or);
            push(node);
        } else if (token == Or) {
            // 处理按位或 |
            match(Or);
            node = createNode(ExpressStatement, Operator, 0, Or);
            node->children[0] = pop();
            node->children[1] = parseExpressionStatement(Xor);
            push(node);
        } else if (token == Xor) {
            // 处理异或 ^
            match(Xor);
            node = createNode(ExpressStatement, Operator, 0, Xor);
            node->children[0] = pop();
            node->children[1] = parseExpressionStatement(And);
            push(node);
        } else if (token == And) {
            // 处理按位与 &
            match(And);
            node = createNode(ExpressStatement, Operator, 0, And);
            node->children[0] = pop();
            node->children[1] = parseExpressionStatement(Eq);
            push(node);
        } else if (token == Eq) {
            // 处理相等 ==
            match(Eq);
            node = createNode(ExpressStatement, Operator, 0, Eq);
            node->children[0] = pop();
            node->children[1] = parseExpressionStatement(Ne);
            push(node);
        } else if (token == Ne) {
            // 处理不等 !=
            match(Ne);
            node = createNode(ExpressStatement, Operator, 0, Ne);
            node->children[0] = pop();
            node->children[1] = parseExpressionStatement(Lt);
            push(node);
        } else if (token == Lt) {
            // 处理小于 <
            match(Lt);
            node = createNode(ExpressStatement, Operator, 0, Lt);
            node->children[0] = pop();
            node->children[1] = parseExpressionStatement(Gt);
            push(node);
        } else if (token == Gt) {
            // 处理大于 >
            match(Gt);
            node = createNode(ExpressStatement, Operator, 0, Gt);
            node->children[0] = pop();
            node->children[1] = parseExpressionStatement(Le);
            push(node);
        } else if (token == Le) {
            // 处理小于等于 <=
            match(Le);
            node = createNode(ExpressStatement, Operator, 0, Le);
            node->children[0] = pop();
            node->children[1] = parseExpressionStatement(Ge);
            push(node);
        } else if (token == Ge) {
            // 处理大于等于 >=
            match(Ge);
            node = createNode(ExpressStatement, Operator, 0, Ge);
            node->children[0] = pop();
            node->children[1] = parseExpressionStatement(Shl);
            push(node);
        } else if (token == Shl) {
            // 处理左移 <<
            match(Shl);
            node = createNode(ExpressStatement, Operator, 0, Shl);
            node->children[0] = pop();
            node->children[1] = parseExpressionStatement(Shr);
            push(node);
        } else if (token == Shr) {
            // 处理右移 >>
            match(Shr);
            node = createNode(ExpressStatement, Operator, 0, Shr);
            node->children[0] = pop();
            node->children[1] = parseExpressionStatement(Add);
            push(node);
        } else if (token == Add) {
            // 处理加 +
            match(Add);
            node = createNode(ExpressStatement, Operator, 0, Add);
            node->children[0] = pop();
            node->children[1] = parseExpressionStatement(Mul);
            push(node);
        } else if (token == Sub) {
            // 处理减 -
            match(Sub);
            node = createNode(ExpressStatement, Operator, 0, Sub);
            node->children[0] = pop();
            node->children[1] = parseExpressionStatement(Mul);
            push(node);
        } else if (token == Mul) {
            // 处理乘 *
            match(Mul);
            node = createNode(ExpressStatement, Operator, 0, Mul);
            node->children[0] = pop();
            node->children[1] = parseExpressionStatement(Bracket);
            push(node);
        } else if (token == Div) {
            // 处理除 /
            match(Div);
            node = createNode(ExpressStatement, Operator, 0, Div);
            node->children[0] = pop();
            node->children[1] = parseExpressionStatement(Bracket);
            push(node);
        } else if (token == Mod) {
            // 处理取模 %
            match(Mod);
            node = createNode(ExpressStatement, Operator, 0, Mod);
            node->children[0] = pop();
            node->children[1] = parseExpressionStatement(Bracket);
            push(node);
        } else if (token == Bracket) {
            // 处理下标 []
            match(Bracket);
            node = createNode(ExpressStatement, Operator, 0, Bracket);
            node->children[0] = pop();
            node->children[1] = parseExpressionStatement(Assign);
            push(node);
            match(']');
        } else {
            printErrorInformation("Get Invalid Token", tokenString);
            exit(1);
        }
    }
    return pop();
}

