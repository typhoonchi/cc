//
// Created by zhangyukun on 2022/5/7.
//
#include <stdio.h>
//int arr[5][5],b[5],c;
//int add(int a, int b) {
//    int res; res = a + b; return res;
//}
//int max(int a, int b) {
//    int max;
//    if ((a > b) && (a > 0)) {
//        max = a;
//    } else {
//        if (b > 0){max = b;}
//        else {max = 0;}
//    }
//    return max;
//}
//int multen(int a) {
//    int res,i;  i = 0;
//    while (i < 10) {
//        i = i + 1; res = res + a;
//    }
//    return res;
//}
//
//int main(){
//    int temp; temp = 5;
//    arr[0][0] = multen(temp);
//    b[0] = add(50,2);
//    c = max(arr[0][0],b[0]);
//    printf("%d\n",c);
//    return 0;
//}
int arr[10];

int add(int a, int b) {
    int res;
    return a + b;
}

int mul(int a, int b) {
    int res;
    res = a * b;
    return res;
}

int main(){
    int ax, bx,cx[2][2], dx, ex[2];
    ax = 5;
    bx = ax;
    dx = 15;
    arr[1] = 5 * ax;
    ex[1] = 6;
    cx[1][1] = 13;
    printf("%d\n",dx);
    printf("%d\n",arr[1]);
    printf("%d\n",ex[1]);
    printf("%d\n",cx[1][1]);
    return 0;
}