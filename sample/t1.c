#include <stdio.h>
enum e{
    DAY = 1, MON = 2, DSY
};

int main(){
    enum e x;
    int xx, y;
    xx = 3;
    y = -1;
    xx = xx - -y;
    printf("%d\n",xx);
    x = 3;
    printf("%d\n",x);
    return 0;
}
