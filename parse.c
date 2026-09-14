#include "parse.h"
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#define PARSE_INVALID_AST (AST){.commandCount = 0, .commands = NULL};

bool matchToken(int* currentIndex, TokenList tokenList, TokenType type) {
    if(*currentIndex > tokenList.count) return false;
    if(tokenList.tokens[*currentIndex].type == type) {
        (*currentIndex)++;
        return true;
    }
    return false;
}
AST parse(TokenList tokenList) {
    int currentIndex = 0;
    AST ast = {.commandCount = 0, .commands = NULL};
    while(currentIndex < tokenList.count) {
        if(tokenList.tokens[currentIndex].type == TOKEN_READ) {
            Command* readCommand = malloc(sizeof(Command));
            readCommand->type = CMD_READ;
            if(!matchToken(&currentIndex, tokenList, TOKEN_IMAGE)) return PARSE_INVALID_AST;
            if(!matchToken(&currentIndex, tokenList, TOKEN_STRING)) return PARSE_INVALID_AST;
            readCommand->data.read.filename = tokenList.tokens[currentIndex-1].originInSource;
            readCommand->data.read.filenameLength = tokenList.tokens[currentIndex-1].lengthInSource;
            if(!matchToken(&currentIndex, tokenList, TOKEN_TO)) return PARSE_INVALID_AST;
            if(!matchToken(&currentIndex, tokenList, TOKEN_VARIABLE)) return PARSE_INVALID_AST;
            readCommand->data.read.argument.length = tokenList.tokens[currentIndex-1].lengthInSource;
            readCommand->data.read.argument.start = tokenList.tokens[currentIndex-1].originInSource;
            readCommand->data.read.argument.type = LVALUE_VARIABLE;
            addCommand(&ast, readCommand);
            continue;
        }

        printf("Unhandled parse case.\n");
        break;
    }

    return ast;
}