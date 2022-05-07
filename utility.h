//
// Created by zhangyukun on 2022/4/11.
//

#ifndef _UTILITY_H
#define _UTILITY_H

bool loadSrc(const char* fileName);
bool init(void);
struct treeNode* createNode(int statementType, int expressionType, int operatorType);
void printToken(int lineNo);
void printSource(int lineNo);
void printTree(struct treeNode* node, int n);
void printType(int type);
void printOperator(int op);
void printTab(int n);
#endif //_UTILITY_H
