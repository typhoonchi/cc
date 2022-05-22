//
// Created by zhangyukun on 2022/4/11.
//

#ifndef _UTILITY_H
#define _UTILITY_H

void loadSrc(const char* fileName);
void init(void);
void printToken(int lineNo);
void printSource(int lineNo);
void printTree(sTreeNode* node, int n);
void printType(int type);
void printOperator(int op);
void printTab(int n);
void printErrorInformation(char* error, const char* message);
void printAssemble();

#endif //_UTILITY_H
