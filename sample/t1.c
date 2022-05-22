#include <stdio.h>
enum e{
    DAY = 1, MON = 2, DSY
};

int main(){
    enum e x;
    x = 3;
    printf("%d\n",x);
    return 0;
}
