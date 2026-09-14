#include "token.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

const char *getTokenTypeString(TokenType tokenType)
{
    switch (tokenType)
    {
    case TOKEN_ARRAY:
        return "ARRAY";
    case TOKEN_ASSERT:
        return "ASSERT";
    case TOKEN_BOOL:
        return "BOOL";
    case TOKEN_COLON:
        return "COLON";
    case TOKEN_COMMA:
        return "COMMA";
    case TOKEN_DOT:
        return "DOT";
    case TOKEN_ELSE:
        return "ELSE";
    case TOKEN_END_OF_FILE:
        return "END_OF_FILE";
    case TOKEN_EQUALS:
        return "EQUALS";
    case TOKEN_FALSE:
        return "FALSE";
    case TOKEN_FLOAT:
        return "FLOAT";
    case TOKEN_FLOATVAL:
        return "FLOATVAL";
    case TOKEN_FN:
        return "FN";
    case TOKEN_IF:
        return "IF";
    case TOKEN_IMAGE:
        return "IMAGE";
    case TOKEN_INT:
        return "INT";
    case TOKEN_INTVAL:
        return "INTVAL";
    case TOKEN_LCURLY:
        return "LCURLY";
    case TOKEN_LET:
        return "LET";
    case TOKEN_LPAREN:
        return "LPAREN";
    case TOKEN_LSQUARE:
        return "LSQUARE";
    case TOKEN_NEWLINE:
        return "NEWLINE";
    case TOKEN_OP:
        return "OP";
    case TOKEN_PRINT:
        return "PRINT";
    case TOKEN_RCURLY:
        return "RCURLY";
    case TOKEN_READ:
        return "READ";
    case TOKEN_RETURN:
        return "RETURN";
    case TOKEN_RPAREN:
        return "RPAREN";
    case TOKEN_RSQUARE:
        return "RSQUARE";
    case TOKEN_SHOW:
        return "SHOW";
    case TOKEN_STRING:
        return "STRING";
    case TOKEN_STRUCT:
        return "STRUCT";
    case TOKEN_SUM:
        return "SUM";
    case TOKEN_THEN:
        return "THEN";
    case TOKEN_TIME:
        return "TIME";
    case TOKEN_TO:
        return "TO";
    case TOKEN_TRUE:
        return "TRUE";
    case TOKEN_VARIABLE:
        return "VARIABLE";
    case TOKEN_VOID:
        return "VOID";
    case TOKEN_WRITE:
        return "WRITE";
    default:
        return "UNKNOWN";
    }
}

void printTokenList(TokenList tokenList)
{
    for (int i = 0; i < tokenList.count; i++)
    {
        Token token = tokenList.tokens[i];
        if (token.type == TOKEN_NEWLINE)
        {
            printf("NEWLINE\n");
            continue;
        }

        printf("%s '%.*s'\n",
               getTokenTypeString(token.type),
               (int)token.lengthInSource,
               token.originInSource);
    }
    printf("END_OF_FILE\n");
}

void addToken(TokenList* tokenList, Token token) {

    if (tokenList->count >= tokenList->capacity) {

        size_t newCapacity = tokenList->capacity == 0
            ? 4096
            : tokenList->capacity * 2;

        Token* newList = realloc(
            tokenList->tokens,
            sizeof(Token) * newCapacity
        );

        if (newList == NULL) {
            printf("Realloc returned null. Aborting.");
            exit(EXIT_FAILURE);
            return;
        }

        tokenList->tokens = newList;
        tokenList->capacity = newCapacity;
    }

    tokenList->tokens[tokenList->count] = token;
    tokenList->count++;
}