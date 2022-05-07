//
// Created by zhangyukun on 2022/4/11.
//

#ifndef _PARSE_H
#define _PARSE_H


void match(int expected);
void checkNewId();
void checkLocalId();
void hideGlobalId();
void recoverGlobalId();
void parse(void);
int parseBaseType(void);
void push(struct treeNode* node);
struct treeNode* pop(void);

struct treeNode* parseStatement(void);
struct treeNode* parseFunction(int type, char* name);
struct treeNode* parseParameters(void);
struct treeNode* parseFunctionBody(void);
struct treeNode* parseDeclaration(int type, char* name, int mode);
struct treeNode* parseIfStatement();
struct treeNode* parseWhileStatement();
struct treeNode* parseReturnStatement();
struct treeNode* parseExpressionStatement(int operator);
#endif //_PARSE_H
