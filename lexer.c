#include "lexer.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#define LEX_FAILURE_TOKEN_LIST \
    (TokenList) { .capacity = 0, .count = 0, .tokens = NULL }

TokenList lexSourceCode(SourceCode sourceCode)
{
    size_t currentLine = 0;
    size_t currentColumn = 0;
    size_t currentIndex = 0;
    TokenList tokenList = (TokenList){.capacity = 0, .count = 0, .tokens = NULL};

    while (currentIndex < sourceCode.characterCount)
    {
        const char *currentLocation = &sourceCode.characters[currentIndex];
        TokenType type = isKeyword(currentLocation);
        if (type != TOKEN_NOT_FOUND)
        {
            size_t length = strlen(getTokenTypeString(type));
            Token tokenToAdd = (Token){.column = currentColumn, .line = currentLine, .type = type, .originInSource = currentLocation, .lengthInSource = length};
            addToken(&tokenList, tokenToAdd);
        }

        currentIndex++;
    }

    return tokenList;
}
void freeTokenList(TokenList *tokenList)
{
    free(tokenList->tokens);
    tokenList->count = 0;
}

TokenType isKeyword(const char *start)
{
    if (strncmp(start, "array", 5) == 0)
        return TOKEN_ARRAY;

    if (strncmp(start, "assert", 6) == 0)
        return TOKEN_ASSERT;

    if (strncmp(start, "bool", 4) == 0)
        return TOKEN_BOOL;

    if (strncmp(start, "else", 4) == 0)
        return TOKEN_ELSE;

    if (strncmp(start, "false", 5) == 0)
        return TOKEN_FALSE;

    if (strncmp(start, "float", 5) == 0)
        return TOKEN_FLOAT;

    if (strncmp(start, "fn", 2) == 0)
        return TOKEN_FN;

    if (strncmp(start, "if", 2) == 0)
        return TOKEN_IF;

    if (strncmp(start, "image", 5) == 0)
        return TOKEN_IMAGE;

    if (strncmp(start, "int", 3) == 0)
        return TOKEN_INT;

    if (strncmp(start, "let", 3) == 0)
        return TOKEN_LET;

    if (strncmp(start, "print", 5) == 0)
        return TOKEN_PRINT;

    if (strncmp(start, "read", 4) == 0)
        return TOKEN_READ;

    if (strncmp(start, "return", 6) == 0)
        return TOKEN_RETURN;

    if (strncmp(start, "show", 4) == 0)
        return TOKEN_SHOW;

    if (strncmp(start, "struct", 6) == 0)
        return TOKEN_STRUCT;

    if (strncmp(start, "sum", 3) == 0)
        return TOKEN_SUM;

    if (strncmp(start, "then", 4) == 0)
        return TOKEN_THEN;

    if (strncmp(start, "time", 4) == 0)
        return TOKEN_TIME;

    if (strncmp(start, "to", 2) == 0)
        return TOKEN_TO;

    if (strncmp(start, "true", 4) == 0)
        return TOKEN_TRUE;

    if (strncmp(start, "void", 4) == 0)
        return TOKEN_VOID;

    if (strncmp(start, "write", 5) == 0)
        return TOKEN_WRITE;

    return TOKEN_NOT_FOUND;
}
