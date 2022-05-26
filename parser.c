//
// Created by zhangyukun on 2022/4/11.
//

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "globals.h"
#include "utility.h"
#include "parser.h"
#include "scanner.h"

static sTreeNode **expressionStack;             // 表达式栈.
static int top;                                 // 栈顶.
static long long offsetAddress;                 // 参数与局部变量的偏移地址.

static void initExpressionStack();
static sTreeNode *createNode(eStatementType statementType, eType identifierType, char *name, eExpressionType expressionType,
                             eClass classType, eToken operatorType, long long value);
static void match(int expected);
static void checkGlobalId();
static void checkLocalId();
static void checkDeclaredId();
static void hideGlobalId();
static void recoverGlobalId();
static int parseBaseType();
static void push(sTreeNode *node);
static sTreeNode *pop();
static sTreeNode *parseFunction(int type, char *name);
static sTreeNode *parseGlobalDeclaration(int type, char *name);
static sTreeNode *parseParameters();
static sTreeNode *parseFunctionBody();
static sTreeNode *parseLocalDeclaration(int type);
static sTreeNode *parseStatement();
static sTreeNode *parseIfStatement();
static sTreeNode *parseWhileStatement();
static sTreeNode *parseForStatement();
static sTreeNode *parseDoWhileStatement();
static sTreeNode *parseReturnStatement();
static sTreeNode *parseExpressionStatement(int operator);

/**
 * @brief 语法分析.
 *
 * 根据词法分析传入的 token, tokenValue, tokenString 进行语法分析.
 *
 * @param   void
 * @return  void
 * */
void parse() {
    int baseType = 0;                               // 存放标识符类型.
    char* idName = NULL;                            // 存放标识符名称.
    sTreeNode *node = NULL;                         // 临时节点.
    sTreeNode *lastSibling = NULL;                  // 最后一个兄弟节点.

    // 初始化表达式栈.
    initExpressionStack();
    // 取 token.
    getToken();
    // 持续语法分析直到结束或出错.
    while (token > 0) {
        // 判断是否是全局变量声明或函数定义.
        if ((INT == token) || (CHAR ==  token) || (VOID == token)) {
            // 处理全局变量声明或函数定义.
            // 获取标识符类型.
            baseType = parseBaseType();
            // 获取标识符名.
            idName = (char*)tokenValue;
            // 检查是否重复声明全局变量或重复定义函数.
            checkGlobalId();
            symbolPtr->type = baseType;
            match(Id);
            // 判断是否是函数定义.
            if ((Void == baseType) || ('(' == token)) {
                // 处理函数定义.
                symbolPtr->class = Fun;
                // 判断是否是 main 函数.
                if (!memcmp("main", idName, strlen("main"))) {
                    // 记录 main 函数入口地址.
                    mainPtr = &(symbolPtr->address);
                }
                // 解析函数定义.
                node = parseFunction(baseType, idName);
            } else {
                // 处理全局变量声明.
                symbolPtr->class = Glo;
                // 记录全局变量在数据段中地址.
                symbolPtr->address = (long long)dataPtr;
                dataPtr++;
                // 解析全局变量声明.
                node = parseGlobalDeclaration(baseType, idName);
                match(';');
            }
        } else {
            // 打印错误信息.
            handleErrorInformation(line, PARSE_ERROR, "parser.c/parse()",
                                   "Get Invalid Global Declaration", NULL);
        }
        // 将节点加入根节点上.
        if (root != NULL) {
            // 根节点非空, 加到最后一个非空节点的兄弟节点上.
            while (lastSibling->sibling != NULL) {
                lastSibling = lastSibling->sibling;
            }
            lastSibling->sibling = node;
        } else {
            // 根节点为空, 当前节点作为根节点.
            root = node;
            lastSibling = root;
        }
    }
}

/**
 * @brief 初始化表达式节点栈.
 *
 * 为解析表达式分配节点栈空间, 并初始化栈顶指针.
 *
 * @param   void
 * @return  void
 * */
static void initExpressionStack() {
    // 分配表达式栈存储区.
    expressionStack = (sTreeNode**) malloc(STACK_SIZE * sizeof(sTreeNode*));
    // 判断分配表达式栈存储区是否成功.
    if (NULL == expressionStack) {
        // 分配表达式栈存储区失败, 打印错误信息.
        handleErrorInformation(0, PARSE_ERROR, "parser.c/initExpressionStack()",
                               "Could Not Malloc Space for Expression Stack", NULL);
    }
    // 初始化表达式栈存储区.
    memset(expressionStack, 0, STACK_SIZE * sizeof(sTreeNode*));
    // 初始化栈顶.
    top = 0;
}

/**
 * @brief 创建抽象语法树节点.
 *
 * 创建 AST 节点, 并初始化内部值.
 *
 * @param   statementType   语句类型 If, While, Return,... .
 * @param   identifierType   标识符类型 Int, Char, Void, Ptr.
 * @param   name            标识符名称.
 * @param   expressionType  表达式类型 Operator, Constant, Identifier, Call.
 * @param   classType       标识符类别 Glo, Loc, Fun, Sys.
 * @param   operatorType    运算符类型 << >> ... .
 * @param   value           常量值或变量地址.
 * @return  node            抽象语法树节点指针.
 * */
static sTreeNode *createNode(eStatementType statementType, eType identifierType, char *name, eExpressionType expressionType,
                             eClass classType, eToken operatorType, long long value) {
    sTreeNode *node = (sTreeNode*) malloc(sizeof(sTreeNode));     // 创建空节点.

    // 判断 AST 节点是否创建成功.
    if (NULL == node) {
        // 创建节点失败, 打印错误信息.
        handleErrorInformation(line, PARSE_ERROR, "parse.c/createNode()",
                               "Could Not Malloc Space for AST Node", NULL);
    }
    // 初始化孩子节点.
    for (int i = 0; i < MAX_CHILDREN; i++) {
        node->children[i] = NULL;
    }
    // 初始化兄弟结点.
    node->sibling = NULL;
    // 初始化语句类型.
    node->statementType = statementType;
    // 初始化标识符类型.
    node->identifierType = identifierType;
    // 初始化标识符名称.
    node->name = name;
    //初始化数组相关内容.
    node->size = 1;
    // 初始化表达式类型 , 仅当语句类型为表达式时有效.
    node->expressionType = expressionType;
    // 初始化标识符类型 , 仅当表达式类型为 Id 或 Call 时有效.
    node->classType = classType;
    // 初始化运算符类型 , 仅当表达式类型为 Operator 时有效.
    node->operatorType = operatorType;
    // 初始化常量值或变量地址.
    node->value = value;
    // 返回节点指针.
    return node;
}

/**
 * @brief 匹配 token.
 *
 * 判断当前 token 是否与预期 token 匹配.
 *
 * @param   expected  期待词素.
 * @return  void
 * */
void match(int expected) {
    // 判断 token  是否匹配.
    if (token != expected) {
        // token 不匹配, 打印错误信息
        handleErrorInformation(line, PARSE_ERROR, "parser.c/match()",
                               "Get an Unexpect Token",NULL);
    }
    // 取下一个词素
    getToken();
}


/**
 * @brief 检查当前标识符是否是已声明的全局标识符.
 *
 * 检查获取 token 是否为 Id, 并且检查对应符号表项的类型是否为初始值, token 为 Id 并且表项为初始值,
 * 则说明当前标识符是未声明过的全局标识符.
 *
 * @note 在声明全局变量或定义函数时检查.
 *
 * @param   void
 * @return  void
 * */
void checkGlobalId() {
    if (token != Id) {
        // 非标识符, 打印错误信息.
        handleErrorInformation(line, PARSE_ERROR, "parser.c/checkGlobalId()",
                               "Get an Invalid Identifier",NULL);
    }
    if (symbolPtr->class == Glo) {
        // 已声明全局标识符, 打印错误信息.
        handleErrorInformation(line, PARSE_ERROR, "parser.c/checkGlobalId()",
                               "Get Duplicate Global Declaration",(char*)tokenValue);
    }
}

/**
 * @brief 检查当前标识符是否是已声明的局部变量.
 *
 * 检查获取 token 是否为 Id, 并且检查对应符号表项的类型是否为 Loc, token 为 Id 并且表项为 Loc,
 * 则说明当前标识符是已声明过的局部变量.
 *
 * @note 在声明局部变量时检查.
 *
 * @param   void
 * @return  void
 * */
void checkLocalId() {
    if (token != Id) {
        // 非标识符, 打印错误信息.
        handleErrorInformation(line, PARSE_ERROR, "parser.c/checkLocalId()",
                               "Get an Invalid Identifier", NULL);
    }
    if (symbolPtr->class == Loc) {
        // 已声明局部变量, 打印错误信息.
        handleErrorInformation(line, PARSE_ERROR, "parser.c/checkLocalId()",
                               "Get Duplicate Local Declaration", (char*)tokenValue);
    }
}

/**
 * @brief 检查是否是已声明的标识符.
 *
 * 检查获取 token 是否为 Id, 并且检查对应符号表项的类型是否为初始值, token 为 Id 并且表项不为初始值,
 * 则说明当前标识符是已声明过的标识符.
 *
 * @note 在使用变量或调用函数时检查.
 *
 * @param   void
 * @return  void
 * */
void checkDeclaredId() {
    if (token != Id) {
        // 非标识符, 打印错误信息.
        handleErrorInformation(line, PARSE_ERROR, "parser.c/checkLocalId()",
                               "Get an Invalid Identifier", NULL);
    }
    if (symbolPtr->class == 0) {
        // 未声明标识符, 打印错误信息.
        handleErrorInformation(line, PARSE_ERROR, "parser.c/checkLocalId()",
                               "Get an Undeclared Identifier", (char*)tokenValue);
    }
}

/**
 * @brief 隐藏全局变量.
 *
 * 遮罩全局变量.
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
 * @brief 恢复全局变量.
 *
 * 恢复被遮罩的全局变量.
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
 * @brief 解析基本变量类型.
 *
 * 检查基本变量类型, 并返回对应的值.
 *
 * @param   void
 * @return  type    基本变量类型: Int, Char, Void.
 * */
int parseBaseType(){
    if (CHAR == token) {
        match(CHAR);
        return Char;
    } else if (INT == token) {
        match(INT);
        return Int;
    } else if (VOID == token) {
        match(VOID);
        return Void;
    } else {
        // 位置基本类型, 打印错误信息.
        handleErrorInformation(line, PARSE_ERROR, "parser.c/parseBaseType()",
                               "Get an Unknown Base Type",NULL);
    }
}

/**
 * @brief 入栈.
 *
 * 将 AST 节点指针放入表达式栈.
 *
 * @param   node    抽象语法树节点指针.
 * @return  void
 * */
void push(sTreeNode *node) {
    expressionStack[top] = node;
    top++;
}

/**
 * @brief 出栈.
 *
 * 返回栈顶的表达式节点指针.
 *
 * @param   void
 * @return  node    抽象语法树节点指针.
 *
 * */
sTreeNode *pop() {
    // 检查下标是否超出表达式栈范围.
    if (top == 0) {
        // 下标越界, 打印错误信息.
        handleErrorInformation(line, PARSE_ERROR, "parser.c/pop()",
                               "Get an Expression Stack Error", NULL);
    }
    top--;
    return expressionStack[top];
}

/**
 * @brief 解析函数.
 *
 * 解析函数定义, 返回指向函数名的 AST 节点指针.
 *
 * @param   type    函数返回值.
 * @param   name    函数名.
 * @return  node    抽象语法树节点指针, 指向函数定义子树.
 * */
sTreeNode *parseFunction(int type, char *name){
    sTreeNode *node = createNode(Function, type, name, 0,
                                 0, 0, (long long)&(symbolPtr->address));   // 创建函数根节点

    // 重置偏移地址.
    offsetAddress = 0;
    match('(');
    // 解析参数列表.
    if (')' == token) {
        // 空参列表 void.
        node->children[0] = createNode(ParameterStatement, 0, NULL, 0,
                                       0, 0, 0);
        node->children[0]->identifierType = Void;
    } else {
        // 有参列表 解析参数列表.
        node->children[0] = parseParameters();
    }
    // 更新偏移地址.
    offsetAddress++;
    match(')');
    match('{');
    // 解析函数体.
    node->children[1] = parseFunctionBody();
    // 恢复全局变量.
    symbolPtr = symbolTable;
    while (symbolPtr->token != 0) {
        // 判断是否是局部变量.
        if (Loc == symbolPtr->class) {
            // 恢复局部变量到声明该变量前的状态.
            recoverGlobalId();
        }
        symbolPtr++;
    }
    match('}');
    return node;
}

/**
 * @brief 解析全局变量声明语句.
 *
 * 解析全局变量声明语句.
 *
 * @param   type    变量类型.
 * @param   name    变量名.
 * @return  node    抽象语法树节点指针, 指向一条声明语句子树.
 * */
static sTreeNode *parseGlobalDeclaration(int type, char *name) {
    sTreeNode *node = createNode(DeclareStatement,type, name, 0,
                                 0, 0, symbolPtr->address);    // 声明语句节点.
    char *identifierName = NULL;             // 变量名.
    long long size = 1;                     // 数组大小.
    long long scale = 1;                    // long long 与数组元素大小的比值.
    long long currentSize = 1;              // 解析时变量当前的类型.
    long long *base = NULL;                 // 数据段指针, 声明数组变量时用于定位数据段初始位置.

    // 持续解析全局变量声明.
    while ((token != ',') && (token != ';')) {
        // 解析数组声明.
        if (Bracket == token) {
            symbolPtr->type += Ptr;
            match(Bracket);
            currentSize = tokenValue;
            match(Num);
            match(']');
            base = dataPtr;
            // 判断是否是最后一组 [].
            if (token != Bracket) {
                // 是最后一组 [], 根据基本类型确定 scale.
                if (Int == type) {
                    // Int.
                    scale = 2;
                } else if (Char == type){
                    // Char.
                    scale = 8;
                }
            }
            // 在数据段中为全局变量分配空间.
            for (int i = 0; i < size; i++) {
                *(base - size + i) = (long long)dataPtr;
                // 计算移动后数据段地址.
                dataPtr += ((currentSize % scale) ? (currentSize / scale + 1) : (currentSize / scale));
            }
            // 计算数组大小.
            size *= currentSize;
        }
    }
    // 更新变量类型与数组大小.
    node->identifierType = symbolPtr->type;
    node->size = size;
    // 处理逗号及后续变量声明.
    if (',' == token) {
        // 解析逗号.
        match(',');
        // 记录变量名.
        identifierName = (char*)tokenValue;
        // 全局变量声明.
        symbolPtr->class = Glo;
        symbolPtr->type = type;
        symbolPtr->address = (long long)dataPtr;
        dataPtr++;
        match(Id);
        // 继续解析声明语句.
        node->sibling = parseGlobalDeclaration(type,identifierName);
    }
    // 返回全局变量声明语句根节点.
    return node;
}

/**
 * @brief 解析局部变量声明语句.
 *
 * 解析局部变量声明语句.
 *
 * @param   type    变量类型.
 * @return  node    抽象语法树节点指针, 指向一条声明语句子树.
 * */
static sTreeNode *parseLocalDeclaration(int type) {
    sTreeNode *node = createNode(DeclareStatement, type, (char*)tokenValue, 0,
                                 0, 0, 0);       // 声明语句节点.
    sTreeNode *lastChildSibling = NULL;             // 指向孩子节点的最后一个兄弟节点.
    long long size = 1;                             // 数组大小.
    long long currentSize = 1;                      // 解析时变量当前的类型.
    long long spaceSize = 1;                        // 数组空间大小.
    long long scale = 1;                            // long long 与数组元素大小的比值.

    match(Id);
    // 持续解析变量声明.
    while ((token != ',') && (token != ';')) {
        // 解析数组声明.
        if (Bracket == token) {
            symbolPtr->type += Ptr;
            match(Bracket);
            currentSize = tokenValue;
            match(Num);
            match(']');
            // 判断是否是最后一组 [].
            if (token != Bracket) {
                // 是最后一组 [], 根据基本类型确定 scale.
                if (Int == type) {
                    // Int.
                    scale = 2;
                } else if (Char == type) {
                    // Char.
                    scale = 8;
                }
            }
            // 计算数组大小.
            size *= currentSize;
            spaceSize *= ((currentSize % scale) ? (currentSize / scale + 1) : (currentSize / scale));
            // 计算偏移地址.
            offsetAddress += spaceSize;
            // 将数组每一维的大小添加进声明语句的孩子节点, 用于代码生成时初始化数组.
            if (node->children[0] != NULL) {
                lastChildSibling->sibling = createNode(DeclareStatement, Int, NULL,  0,
                                                       0, 0, currentSize);
                lastChildSibling = lastChildSibling->sibling;
            } else {
                node->children[0] = createNode(DeclareStatement, Int, NULL, 0,
                                               0, 0, currentSize);
                lastChildSibling = node->children[0];
            }
        }
    }
    // 回填局部变量的偏移地址.
    symbolPtr->address = offsetAddress;
    // 更新变量地址, 变量类型与数组大小.
    node->value = symbolPtr->address;
    node->identifierType = symbolPtr->type;
    node->size = size;
    // 处理逗号及后续变量声明.
    if (',' == token) {
        // 解析逗号.
        match(',');
        // 局部变量声明.
        checkLocalId();
        hideGlobalId();
        symbolPtr->class = Loc;
        symbolPtr->type = type;
        offsetAddress++;
        // 继续解析声明语句.
        node->sibling = parseLocalDeclaration(type);
    }
    // 返回局部变量声明语句根节点.
    return node;
}

/**
 * @brief 解析参数列表.
 *
 * 解析参数列表中参数信息.
 *
 * @param   void
 * @return  node    抽象语法树节点指针, 指向参数列表子树.
 * */
sTreeNode *parseParameters(){
    sTreeNode *node = createNode(ParameterStatement,parseBaseType(), (char*)tokenValue, 0,
                                 0, 0, 0);                 // 参数列表根节点.

    // 形参作为局部变量使用.
    checkLocalId();
    hideGlobalId();
    match(Id);
    // 处理数组参数.
    while ((token != ',') && (token!= ')')) {
        if (token == Bracket) {
            // 匹配数组参数.
            match(Bracket);
            node->identifierType += Ptr;
            // 过滤数组参数中的常量.
            if (token == Num) {
                match(Num);
            }
            match(']');
        } else {
            exit(1);
        }
    }
    // 更新符号表中局部变量信息.
    symbolPtr->class = Loc;
    symbolPtr->type = node->identifierType;
    symbolPtr->address = offsetAddress;
    // 更新偏移地址.
    offsetAddress++;
    if (',' == token) {
        // 匹配后续参数.
        match(',');
        // 后续参数作为兄弟节点, 依次加入到参数列表根节点中.
        node->sibling = parseParameters();
    }
    // 返回参数列表根节点.
    return node;
}

/**
 * @brief 解析函数体.
 *
 * 解析函数体中语句.
 *
 * @param   void
 * @return  node    抽象语法树节点指针, 指向函数体子树.
 * */
sTreeNode *parseFunctionBody(){
    sTreeNode *node = NULL;                     // 函数体根节点.
    sTreeNode *statementNode = NULL;            // 非声明语句根节点.
    sTreeNode *lastSibling = NULL;              // 最后一个兄弟节点.
    sTreeNode *lastStatementNodeSibling =NULL;  // 非证明语句最后一个兄弟节点.
    int baseType = 0;                           // 标识符类别.

    // 持续解析非声明语句.
    while (token != '}') {
        if ((INT == token) || (CHAR == token)) {
            // 获取局部变量类型.
            baseType = parseBaseType();
            // 检查是否已声明过该局部变量.
            checkLocalId();
            // 隐藏全局变量.
            hideGlobalId();
            symbolPtr->class = Loc;
            symbolPtr->type = baseType;
            offsetAddress++;
            // 优先将声明语句节点加入函数体根节点上.
            if (node != NULL) {
                // 函数体根节点非空, 加到最后一个非空节点的兄弟节点上.
                while (lastSibling->sibling != NULL) {
                    lastSibling = lastSibling->sibling;
                }
                lastSibling->sibling = parseLocalDeclaration(baseType);
            } else {
                // 函数体根节点为空, 当前节点作为函数体根节点.
                node = parseLocalDeclaration(baseType);
                lastSibling = node;
            }
            match(';');
        } else {
            // 将非声明语句节点加入语句根节点上.
            if (statementNode !=  NULL) {
                // 函数体根节点非空, 加到最后一个非空节点的兄弟节点上.
                lastStatementNodeSibling->sibling = parseStatement();
                lastStatementNodeSibling = lastStatementNodeSibling->sibling;
            } else {
                // 语句根节点为空, 当前节点作为语句根节点.
                statementNode = parseStatement();
                lastStatementNodeSibling = statementNode;
            }
        }
    }
    // 将非声明语句根节点加入函数体根节点上.
    if (node !=  NULL) {
        // 函数体根节点非空, 加到最后一个非空节点的兄弟节点上.
        lastSibling = node;
        while (lastSibling->sibling != NULL) {
            lastSibling = lastSibling->sibling;
        }
        lastSibling->sibling = statementNode;
    } else {
        // 函数体根节点为空, 当前节点作为函数体根节点.
        node = statementNode;
    }
    // 返回函数体根节点.
    return node;
}

/**
 * @brief 解析语句.
 *
 * 解析 If 语句, While 语句, For 语句, Do While 语句, Return 语句和表达式语句.
 *
 * @param   void
 * @return  node    抽象语法树节点指针, 指向语句子树.
 * */
sTreeNode *parseStatement() {
    sTreeNode *node = NULL;         // 语句根节点.

    // 过滤空语句.
    while (';' == token) {
        match(';');
    }
    // 判断语句类型.
    if (IF == token) {
        // 匹配 If 语句.
        node = parseIfStatement();
    } else if (WHILE == token) {
        // 匹配 While 语句.
        node = parseWhileStatement();
    } else if (FOR == token) {
        // 匹配 For 语句.
        node = parseForStatement();
    } else if (DO == token) {
        // 匹配 Do While 语句.
        node = parseDoWhileStatement();
    } else if (RETURN == token) {
        // 匹配 Return 语句.
        node = parseReturnStatement();
    } else {
        // 匹配表达式语句.
        node = parseExpressionStatement(Assign);
        if (',' == token) {
            match(',');
        } else {
            match(';');
        }
    }
    return node;
}

/**
 * @brief 解析 If 语句.
 *
 * 解析 If 语句.
 *
 * @param   void
 * @return  node    抽象语法树节点指针, 指向 If 语句子树.
 * */
sTreeNode *parseIfStatement(){
    sTreeNode *node = createNode(IfStatement, 0, NULL, 0,
                                 0, 0, 0);    // If 语句根节点.
    sTreeNode *lastSibling = NULL;      // 临时节点.

    match(IF);
    match('(');
    // 解析条件表达式.
    node->children[0] = parseExpressionStatement(Assign);
    match(')');
    match('{');
    // 持续解析成功分支语句.
    while (token != '}') {
        // 将节点加入 If 语句根节点上.
        if (node->children[1] != NULL) {
            // If 语句根节点非空, 加到最后一个非空节点的兄弟节点上.
            while (lastSibling->sibling != NULL) {
                lastSibling = lastSibling->sibling;
            }
            lastSibling->sibling = parseStatement();
        } else {
            // If 语句根节点为空, 当前节点作为 If 语句根节点.
            node->children[1] = parseStatement();
            lastSibling = node->children[1];
        }
    }
    match('}');
    // 解析可选 Else 语句.
    if (ELSE == token) {
        // 存在 Else 语句.
        match(ELSE);
        match('{');
        // 持续解析失败分支语句.
        while (token != '}') {
            // 将节点加入 If 语句根节点上.
            if (node->children[2] != NULL) {
                // If 语句根节点非空, 加到最后一个非空节点的兄弟节点上.
                while (lastSibling->sibling != NULL) {
                    lastSibling= lastSibling->sibling;
                }
                lastSibling->sibling = parseStatement();
            } else {
                // If 语句根节点为空, 当前节点作为 If 语句根节点.
                node->children[2] = parseStatement();
                lastSibling = node->children[2];
            }
        }
        match('}');
    }
    // 返回 If 语句根节点.
    return node;
}

/**
 * @brief 解析 While 语句.
 *
 * 解析 While 语句.
 *
 * @param   void
 * @return  node    返回抽象语法树节点指针, 指向 While 语句子树.
 * */
sTreeNode *parseWhileStatement(){
    sTreeNode *node = createNode(WhileStatement, 0, NULL, 0,
                                 0, 0, 0);     // While 语句根节点.
    sTreeNode *lastSibling = NULL;     // 临时节点.

    match(WHILE);
    match('(');
    // 解析条件语句.
    node->children[0] = parseExpressionStatement(Assign);
    match(')');
    match('{');
    // 持续解析循环体语句.
    while (token != '}') {
        // 将节点加入 While 语句根节点上.
        if (node->children[1] != NULL) {
            // While 语句根节点非空, 加到最后一个非空节点的兄弟节点上.
            while (lastSibling->sibling != NULL) {
                lastSibling = lastSibling->sibling;
            }
            lastSibling->sibling = parseStatement();
        } else {
            // While 语句根节点为空, 当前节点作为 While 语句根节点.
            node->children[1] = parseStatement();
            lastSibling = node->children[1];
        }
    }
    match('}');
    // 返回 While 语句根节点.
    return node;
}

/**
 * @brief 解析 For 语句.
 *
 * 解析 For 语句.
 *
 * @param   void
 * @return  node    返回抽象语法树节点指针, 指向 For 语句子树.
 * */
static sTreeNode *parseForStatement() {
    sTreeNode *node = createNode(ForStatement, 0, NULL, 0,
                                 0, 0, 0);     // For 语句根节点.
    sTreeNode *lastSibling = NULL;     // 临时节点.

    match(FOR);
    match('(');
    // 解析初始化语句.
    node->children[0] = parseExpressionStatement(Assign);
    lastSibling = node->children[0];
    // 继续解析 ',' 后的初始化语句.
    while (token == ',') {
        match(',');
        lastSibling->sibling = parseExpressionStatement(Assign);
        lastSibling = lastSibling->sibling;
    }
    match(';');
    // 解析条件语句.
    node->children[1] = parseExpressionStatement(Assign);
    match(';');
    // 解析条件更新语句.
    node->children[2] = parseExpressionStatement(Assign);
    lastSibling = node->children[2];
    // 继续解析 ',' 后的条件更新语句.
    while (',' == token) {
        match(',');
        lastSibling->sibling = parseExpressionStatement(Assign);
        lastSibling = lastSibling->sibling;
    }
    match(')');
    match('{');
    // 持续解析循环体语句.
    while (token != '}') {
        // 将节点加入 For 语句根节点上.
        if (node->children[3] != NULL) {
            // For 语句根节点非空, 加到最后一个非空节点的兄弟节点上.
            while (lastSibling->sibling != NULL) {
                lastSibling = lastSibling->sibling;
            }
            lastSibling->sibling = parseStatement();
        } else {
            // For 语句根节点为空, 当前节点作为 While 语句根节点.
            node->children[3] = parseStatement();
            lastSibling = node->children[3];
        }
    }
    match('}');
    // 返回 For 语句根节点.
    return node;
}

/**
 * @brief 解析 Do While 语句.
 *
 * 解析 Do While 语句.
 *
 * @param   void
 * @return  node    返回抽象语法树节点指针, 指向 Do While 语句子树.
 * */
static sTreeNode *parseDoWhileStatement() {
    sTreeNode *node = createNode(DoWhileStatement, 0, NULL, 0,
                                 0, 0, 0);     // While 语句根节点.
    sTreeNode *lastSibling = NULL;     // 临时节点.

    match(DO);
    match('{');
    // 持续解析循环体语句.
    while (token != '}') {
        // 将节点加入 Do While 语句根节点上.
        if (node->children[0] != NULL) {
            // Do While 语句根节点非空, 加到最后一个非空节点的兄弟节点上.
            while (lastSibling->sibling != NULL) {
                lastSibling = lastSibling->sibling;
            }
            lastSibling->sibling = parseStatement();
        } else {
            // Do While 语句根节点为空, 当前节点作为 While 语句根节点.
            node->children[0] = parseStatement();
            lastSibling = node->children[0];
        }
    }
    match('}');
    match(WHILE);
    match('(');
    // 解析条件语句.
    node->children[1] = parseExpressionStatement(Assign);
    match(')');
    match(';');
    // 返回 Do While 语句根节点.
    return node;
}

/**
 * @brief 解析 Return 语句.
 *
 * 解析 Return 语句.
 *
 * @param   void
 * @return  node    返回抽象语法树节点指针, 指向 Return 语句子树.
 * */
sTreeNode *parseReturnStatement(){
    sTreeNode *node = createNode(ReturnStatement, 0, NULL, 0,
                                 0, 0, 0);    // Return 语句根节点.

    match(RETURN);
    // 判断是否有返回值.
    if (token != ';') {
        // 解析返回表达式.
        node->children[0] = parseExpressionStatement(Assign);
    }
    match(';');
    // 返回 Return 语句根节点.
    return node;
}

/**
 * @brief 解析表达式语句.
 *
 * 解析表达式语句.
 *
 * @param   operator    当前运算符.
 * @return  node        返回抽象语法树节点指针, 指向表达式语句子树.
 * */
sTreeNode *parseExpressionStatement(int operator){
    sTreeNode *node = NULL;             // 表达式语句节点.
    sTreeNode *lastSibling = NULL;      // 临时节点.
    long long tempToken = 0;            // 记录 token 类型.

    // 处理运算分量.
    if (Num == token) {
        // 处理数值常量.
        // 为数值常量创建节点.
        node = createNode(ExpressStatement, Int, NULL, Constant,
                          0, 0, tokenValue);
        push(node);
        match(Num);
    } else if (Char == token){
        // 处理字符常量.
        // 为字符常量创建节点.
        node = createNode(ExpressStatement, Char, NULL, Constant,
                          0, 0, tokenValue);
        push(node);
        match(Char);
    } else if (Char + Ptr == token) {
        // 处理字符串常量.
        // 为字符串常量创建节点.
        node = createNode(ExpressStatement,Char + Ptr, NULL, Constant,
                          0, 0, tokenValue);
        push(node);
        match(Char + Ptr);
    } else if (Id == token) {
        // 处理标识符.
        // 检查标识符是否已声明.
        checkDeclaredId();
        // 判断是函数调用还是变量.
        if ((Fun == symbolPtr->class) || (Sys == symbolPtr->class)) {
            // 处理函数调用.
            node = createNode(ExpressStatement,symbolPtr->type, (char*)tokenValue, Call,
                              symbolPtr->class, 0, (long long)&(symbolPtr->address));
            match(Id);
            match('(');
            // 持续解析参数列表.
            while (token != ')') {
                // 将节点加入函数调用根节点上.
                if (node->children[0] != NULL) {
                    // 函数调用根节点非空, 加到最后一个非空节点的兄弟节点上.
                    while (lastSibling->sibling != NULL) {
                        lastSibling = lastSibling->sibling;
                    }
                    lastSibling->sibling = parseExpressionStatement(Assign);
                } else {
                    // 函数调用根节点为空, 当前节点作为函数调用根节点.
                    node->children[0] = parseExpressionStatement(Assign);
                    lastSibling = node->children[0];
                }
                if (token != ')') {
                    match(',');
                }
            }
            match(')');
        } else {
            // 处理变量.
            // 为标识符创建节点.
            node = createNode(ExpressStatement, symbolPtr->type, (char*)tokenValue, Variable,
                              symbolPtr->class, 0, symbolPtr->address);
            match(Id);
        }
        // 判断是否出现非法情况.
        if (Id == token){
            // 标识符后跟标识符, 非法情况, 打印错误信息.
            handleErrorInformation(line, PARSE_ERROR, "parser.c/parseExpressionStatement",
                                   "Get Two Identifiers Together", NULL);
            exit(1);
        }
        push(node);
    } else if ('(' == token) {
        // 处理表达式中括号内的子表达式.
        match('(');
        node = parseExpressionStatement(Assign);
        match(')');
        push(node);
    } else if ('!' == token) {
        // 处理单目运算符逻辑非 !.
        node = createNode(ExpressStatement, 0, NULL, Operator,
                          0, '!', 0);
        match('!');
        node->children[0] = parseExpressionStatement(Inc);
        push(node);
    } else if (Add == token) {
        // 处理单目运算符正号 +.
        match(Add);
    } else if (Sub == token) {
        // 处理单目运算符负号 -.
        node = createNode(ExpressStatement, 0, NULL, Operator,
                          0, Sub, 0);
        match(Sub);
        node->children[0] = createNode(ExpressStatement, Int, NULL, Constant,
                0, 0, 0);
        node->children[1] = parseExpressionStatement(Inc);
        push(node);
    } else {
        // 处理错误信息, 打印错误信息.
        handleErrorInformation(line, PARSE_ERROR, "parser.c/parseExpressionStatement()",
                               "Get an Invalid Expression", NULL);
    }
    // 优先级爬山处理表达式运算.
    while (token >= operator) {
        // 仅处理优先级不低于当前优先级的运算子表达式.
        if (Assign == token) {
            // 处理赋值 =.
            match(Assign);
            node = createNode(ExpressStatement, 0, NULL, Operator,
                              0, Assign, 0);
            node->children[0] = pop();
            node->children[1] = parseExpressionStatement(Assign);
            push(node);
        } else if ((token >= Lor) && (token <= Shr)) {
            // 处理从逻辑或 || 到右移 >>.
            // 记录当前 token.
            tempToken = token;
            // 处理逻辑或 || 到 右移 >>.
            match((int)token);
            node = createNode(ExpressStatement, 0, NULL, Operator,
                              0, tempToken, 0);
            node->children[0] = pop();
            node->children[1] = parseExpressionStatement((int)(tempToken + 1));
            push(node);
        } else if ((token >= Add) && (token <= Sub)) {
            // 处理加 + 到 减 -.
            // 记录当前 token.
            tempToken = token;
            match((int)token);
            node = createNode(ExpressStatement, 0, NULL, Operator,
                              0, tempToken, 0);
            node->children[0] = pop();
            node->children[1] = parseExpressionStatement(Mul);
            push(node);
        } else if ((token >= Mul) && (token <= Mod)) {
            // 处理乘 * 到 取模 %.
            // 记录当前 token.
            tempToken = token;
            match((int)token);
            node = createNode(ExpressStatement, 0, NULL, Operator,
                              0, tempToken, 0);
            node->children[0] = pop();
            node->children[1] = parseExpressionStatement(Bracket);
            push(node);
        } else if (Bracket == token) {
            // 处理下标 [].
            match(Bracket);
            node = createNode(ExpressStatement, 0, NULL, Operator,
                              0, Bracket, 0);
            node->children[0] = pop();
            node->children[1] = parseExpressionStatement(Assign);
            push(node);
            match(']');
        } else {
            // 处理错误信息, 打印错误信息.
            handleErrorInformation(line, PARSE_ERROR, "parser.c/parseExpressionStatement()",
                                   "Get an Invalid Operator", NULL);
        }
    }
    return pop();
}