#include <stdio.h>
#include "sourceCode.h"
#include "lexer.h"
#include "token.h"
#include <string.h>
#include "parse.h"

bool contains(const char* lookup, int argumentCount, char* argumentVector[]) {
    for(int i = 0; i < argumentCount; i++) {
        if(strcmp(lookup, argumentVector[i]) == 0) return true;
    }
    return false;
}

int main(int argumentCount, char* argumentVector[]) {
    if(argumentCount == 1) {
        printf("Usage: %s <FLAG> <FILENAME>\n", argumentVector[0]);
        return 0;
    }
    if(argumentCount > 3) {
        printf("Maximum of 3 arguments.\n");
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
    if(contains("-l", argumentCount, argumentVector)) {
        printTokenList(tokenList);
        printf("Compilation succeeded\n");
        return 0;
    }
    AST ast = parse(tokenList);
    printAST(ast);




    freeTokenList(&tokenList);
    freeSourceCode(&sourceCode);
    return 0;
}