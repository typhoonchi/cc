#include <stdio.h>


int add(int x, int y){
    int ret;
    ret = x + y;
    return ret;
}

int mul(int x, int y, int z) {
    int ret;
    ret = x * y + y / (z + x);
    return ret;
}

int main(int x) {
    int a;
    int b[5];
    a = mul(1,2,1);
    if (!(a > 0) && (a < 5)) {
        a = (a + 1) * 5;
        b[2] = b[1] / 2;
    } else {
        if (b[0] > 9 || a != 0) {
            a = a & b[0] + add(1, 5);
        } else {
            a = a | b[2];
        }
    }
    return 0;
}