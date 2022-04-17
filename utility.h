//
// Created by zhangyukun on 2022/4/11.
//

#ifndef _UTILITY_H
#define _UTILITY_H

bool loadSrc(const char* fileName);
bool init(void);
void printToken(int lineNo, const struct symbol* ptr);
void printSource(int lineNo);

#endif //_UTILITY_H
