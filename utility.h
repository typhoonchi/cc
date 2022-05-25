//
// Created by zhangyukun on 2022/4/11.
//

#ifndef UTILITY_H
#define UTILITY_H

void loadSrc(const char* fileName);
void init(void);
void printToken(int lineNo);
void printTree(sTreeNode* node, int n);
void handleErrorInformation(int lineNo, eErrorCode errorCode , const char* location, const char* error, const char* message);
void printAssemble();

#endif // UTILITY_H
