//
// Created by zhangyukun on 2022/4/11.
//

#ifndef UTILITY_H
#define UTILITY_H

void init(const char *fileName);
void destroy();
void printToken(int lineNo);
void printTree();
void printAssemble();
void handleErrorInformation(int lineNo, eErrorCode errorCode , const char *location, const char *error, const char *message);

#endif // UTILITY_H
