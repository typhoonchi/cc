//
// Created by zhangyukun on 2022/5/15.
//

#ifndef _CODE_H
#define _CODE_H

#include "globals.h"
#include "utility.h"

void generateCode(struct treeNode* node);
void generateFunctionCode(struct treeNode* node);
void generateExpressionStatementCode(struct treeNode* node, int offset);
void runCode(int argc, char** argv);

#endif //_CODE_H
