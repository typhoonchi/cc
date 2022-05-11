//
// Created by zhangyukun on 2022/4/11.
//

#ifndef _UTILITY_H
#define _UTILITY_H

void loadSrc(const char* fileName);
void init(void);
struct treeNode* createNode(int statementType, int expressionType, int operatorType);
void printToken(int lineNo);
void printSource(int lineNo);
void printTree(struct treeNode* node, int n);
void printType(int type);
void printOperator(int op);
void printTab(int n);
void printErrorInformation(char* error, const char* message);
#endif //_UTILITY_H
