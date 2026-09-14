#include <stdio.h>
#include "sourceCode.h"
#include "lexer.h"
#include "token.h"



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

    TokenList tokenList = lexSourceCode(sourceCode);
    if(tokenList.tokens == NULL) {
        printf("Compilation Failed\n");
        return 0;
    }
    printTokenList(tokenList);

    printf("Compilation succeeded\n");

    freeTokenList(&tokenList);
    freeSourceCode(&sourceCode);
    return 0;
}