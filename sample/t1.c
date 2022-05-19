#include <stdio.h>

int main(){
    long long ch;
    ch = 0x7F7F7F7F7F7F7F66;
    char c[10] = {0};
    c[0] = '%';
    c[1]=  '\n';
    c[2] = 127;
    c[3] = 'c';
    c[4] = 127;
    c[5] = '\n';
    c[6] = '\0';
    printf((char*)c,ch);
    return 0;

}
