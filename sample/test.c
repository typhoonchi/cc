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

//int main() {
//    int a,b;
//    a = (b = 2);
//    printf("%d %d\n",a,b);
//    return 0;
//}
int arr[10];
int brr[5][5];

//int add(int a, int b) {
//    int res;
//    return a + b;
//}
//
//int mul(int a, int b) {
//    int res;
//    res = a * b;
//    int ret;
//    ret = res;
//    return ret;
//}

void fresh(int a[], int n) {
    int i;
    i = 0;
    while (i < n) {
        a[i] = i + 1;
        printf("%d ",a[i]);
        i = i + 1;
    }
    printf("whileend\n");
    i = 0;
    do {
        a[i] = i + 2;
        printf("%d ",a[i]);
        i = i + 1;
    } while (i < n);
    printf("dowhileend\n");
    for (i = 0; i < n; i = i + 1) {
        a[i] = i * 2 + 3;
        printf("%d ",a[i]);
    }
    printf("forend\n");
}

int main(){
    int ax, bx,cx[2][2], dx, ex[2];
    bx = ax;
    dx = 15;
    int crr[10];
//    crr[5] = 10, ax = 5;;;;;
//    printf("%d %d\n",crr[5], ax);
//    arr[1] = 5 * ax;
//    ex[1] = 6;
//    cx[1][1] = 13;
//    brr[4][4] = ax;
//    arr[2] = add(3,5);
//    printf("%d\n",arr[1]);
//    printf("%d\n",arr[2]);
//    printf("%d\n",brr[4][4]);;
    fresh(crr,10);
//    fresh(arr, 10);
//    int j;
//    j = 0;
//    while (j < 10) {
//        printf("%d ",crr[j]);
//        j = j + 1;
//    }
//    printf("\n");
//    j = 0;
//    while (j < 10) {
//        printf("%d ",arr[j]);
//        j = j + 1;
//    }
//    printf("\n");
    return 0;
}
