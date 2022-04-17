#include <stdio.h>

#define TRUE 1
#define FALSE 0

int add(int a, int b) {
    int ret;
    ret = a + b;
    ret = a % b;
    a++;
    b--;
    ret = a * b;
    if (a >= 0) {
        b = b << 3;
        if (b == 2) {
            ret = b / a;
        } else {
            if (a - b < 8) {
                ret = a & b;
            }
        }
    }
/*    if (!a) {
        b = ret | a;
        if (ret != 0) {
            a = b >> 1;
        }
    }*/
    while (a--) {
        b++;
    }
    b = 0x12;
    a = 012;
    return ret;
}
/*
 * main function
 * */
int main() {
    printf("%d + %d = %d\n", 1, 3, add(1, 4));
    // add(1, 2);
    return 0;
}