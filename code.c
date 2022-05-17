//
// Created by zhangyukun on 2022/5/15.
//

#include "code.h"

void runCode(int argc, char** argv){
    long long op;
    long long* temp;
    bp = sp = (long long*)((long long)stack + MAXSIZE);
    *--sp = EXIT;
    *--sp = PUSH;
    temp = sp;
    *--sp = argc;
    *--sp = (long long)argv;
    *--sp = (long long)temp;
    pc = mainPtr;
    if (pc == NULL) {
        printErrorInformation("Fail to Find main Function", NULL);
        exit(1);
    }
    cycle = 0;
    while (true) {
        cycle++;
        op = *pc++;
        if (op == IMM) {
            ax = *pc++;
        } else if (op == LEA) {
            ax = (long long)(bp + *pc++);
        } else if (op == LC) {
            ax = *(long long*)ax;
        } else if (op == LI) {
            ax = *(long long*)ax;
        } else if (op == SC) {
            *(long long*)*sp++ = ax;
        } else if (op == SI) {
            *(long long*)*sp++ = ax;
        } else if (op == PUSH) {
            *--sp = ax;
        } else if (op == JMP) {
            pc = (long long*)*pc;
        } else if (op == JZ) {
            if (ax == 0) {
                pc = (long long*)*pc;
            } else {
                pc = pc + 1;
            }
        } else if (op == JNZ) {
            if (ax != 0) {
                pc = (long long*)*pc;
            } else {
                pc = pc + 1;
            }
        } else if (op == OR) {
            ax = *sp++ | ax;
        } else if (op == XOR) {
            ax = *sp++ ^ ax;
        } else if (op == AND) {
            ax = *sp++ & ax;
        } else if (op == EQ) {
            ax = *sp++ == ax;
        } else if (op == NE) {
            ax = *sp++ != ax;
        } else if (op == LT) {
            ax = *sp++ < ax;
        } else if (op == LE) {
            ax = *sp++ <= ax;
        } else if (op == GT) {
            ax = *sp++ > ax;
        } else if (op == GE) {
            ax = *sp++ >= ax;
        } else if (op == SHL) {
            ax = *sp++ << ax;
        } else if (op == SHR) {
            ax = *sp++ >> ax;
        } else if (op == ADD) {
            ax = *sp++ + ax;
        } else if (op == SUB) {
            ax = *sp++ - ax;
        } else if (op == MUL) {
            ax = *sp++ * ax;
        } else if (op == DIV) {
            ax = *sp++ / ax;
        } else if (op == MOD) {
            ax = *sp++ % ax;
        } else if (op == CALL) {
            *--sp = (long long)(pc + 1);
            pc = (long long*)*pc;
        } else if (op == NVAR) {
            *--sp = (long long)bp;
            bp = sp;
            sp = sp - *pc++;
        } else if (op == DARG) {
            sp = sp + *pc++;
        } else if (op == RET) {
            sp = bp;
            bp = (long long*)*sp++;
            pc = (long long*)*sp++;
        } else if (op == EXIT) {
            printf("exit(%lld)\n",*sp);
            return;
        } else {
            printErrorInformation("Get Unknown Instruction", NULL);
            exit(1);
        }
    }
}