//
// Created by zhangyukun on 2022/5/7.
//
#include <stdio.h>

int main() {
    int a,b;
    b = 0;
    if (0 == b) {
        a = 0;
    } else if (b == 1) {
        a = 1;
    } else if (b == 2) {
        a = 2;
    }
    printf("%d\n",a);
    return 0;
}
