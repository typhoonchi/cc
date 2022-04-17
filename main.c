#include "globals.h"
#include "utility.h"
#include "scan.h"

char* source = NULL;
char* sourceDump = NULL;
char* sourcePtr = NULL;
char* tokenString = NULL;
FILE* filePtr = NULL;
long long token, tokenValue;
int line = 1;
int scanTrace = 1;
int sourceTrace = 1;


// symbol table & pointer
struct symbol * symbolTable,
            * symbolPtr;

int main(int argc, char** argv) {
    if (loadSrc(*(argv + 1))) {
        if (!init()) {
            exit(1);
        }
        printf("%s\n",source);
        initKeywords();
        if (scanTrace && sourceTrace) {
            printSource(line);
        }
        while ((*source) != '\0') {
            getToken();
        }
    } else {
        exit(1);
    }
    return 0;
}
