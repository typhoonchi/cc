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
//    b[0] = add(1,2);
//    c = max(arr[0][0],b[0]);
//    return 0;
//}

int add(int a, int b) {
    int res;
    return a + b;
}

int main(){
    int x, y, z, h;
    char c;
    c = 'd';
    x = 1;
    y = add(4,add(x, 4));
    printf("y = %d\n",y);
    return 0;
}