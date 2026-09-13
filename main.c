#include <stdio.h>
#include "sourceCode.h"



int main(int argumentCount, char* argumentVector[]) {
    if(argumentCount == 1) {
        printf("Usage: %s <FLAGS> <FILENAME>\n", argumentVector[0]);
        return 0;
    }
    SourceCode sourceCode = readSourceCode(argumentVector[argumentCount-1]);
    if(sourceCode.characters == NULL) {
        printf("Failed to open file \"%s\".\n", argumentVector[argumentCount-1]);
        return 0;
    }
    printSourceCodeCharacters(sourceCode);


    freeSourceCode(&sourceCode);
    return 0;
}