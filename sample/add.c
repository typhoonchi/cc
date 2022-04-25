#include <stdio.h>
#define TRUE 1
#define FALSE 0
int a[5],b,c[3],d;
void test() {
    char ret[5];
    ret[0] = 'c';
    ret[1] = 'h';
    ret[2] = 'a';
    ret[3] = 'r';
    ret[4] = '\0';
}
int add(int a, int b) {
    int y[10];
    int ret; ret = a % b + a ^ b;
    a++;  //  b--;
    ret = a * b - a & b; if (a >= 0) {
        b = b << 3; if (b == 2 || !(a != 2 && b <= a)) {ret = b / a;
            y[9] = y[1] + y[2];} else {
            if (a - b < 8) {ret = a | b;}
        }
    }
    while (a--) {b++;}
    b = 0x12;a = 012;return ret;
}
/*
 * main function
 * */
int main() {
//    printf("%d + %d = %d\n", 1, 3, add(1, 4)); // add(1, 2);
    return 0;
}