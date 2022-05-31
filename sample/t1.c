#include <stdio.h>


int main() {
    int i, j, k;
    int arr[10];
    i = 0, j = 1, k = 2;
    do {
        arr[i] = i;
        printf("%d ", arr[i]);
    } while (j = j + 1, k = k + 1, (i = i + 1) < 10);
    printf("\n%d %d %d\n", i, j, k);
    return 0;
}
