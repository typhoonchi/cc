//
// Created by zhangyukun on 2022/5/7.
//
#include <stdio.h>

int main() {
    int i, j, k;
    int arr[10];
    for (i = 0, j = 1, k = 2; i < 10; j = j +1, k = k + 1, i = i + 1) {
        arr[i] = i;
        printf("%d ", arr[i]);
    }
    printf("\n%d %d %d\n", i, j, k);
    return 0;
}
