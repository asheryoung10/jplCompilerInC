#pragma once
#include <stddef.h>

typedef enum TokenType {
TOKEN_ARRAY,
TOKEN_ASSERT,
TOKEN_BOOL,
TOKEN_COLON,
TOKEN_COMMA,
TOKEN_DOT,
TOKEN_ELSE,
TOKEN_END_OF_FILE,
TOKEN_EQUALS,
TOKEN_FALSE,
TOKEN_FLOAT,
TOKEN_FLOATVAL,
TOKEN_FN,
TOKEN_IF,
TOKEN_IMAGE,
TOKEN_INT,
TOKEN_INTVAL,
TOKEN_LCURLY,
TOKEN_LET,
TOKEN_LPAREN,
TOKEN_LSQUARE,
TOKEN_NEWLINE,
TOKEN_OP,
TOKEN_PRINT,
TOKEN_RCURLY,
TOKEN_READ,
TOKEN_RETURN,
TOKEN_RPAREN,
TOKEN_RSQUARE,
TOKEN_SHOW,
TOKEN_STRING,
TOKEN_STRUCT,
TOKEN_SUM,
TOKEN_THEN,
TOKEN_TIME,
TOKEN_TO,
TOKEN_TRUE,
TOKEN_VARIABLE,
TOKEN_VOID,
TOKEN_WRITE,
TOKEN_NOT_FOUND
} TokenType;

typedef struct {
    TokenType type;
    const char* originInSource;
    size_t lengthInSource;
    size_t line;
    size_t column;
} Token;

typedef struct {
    Token* tokens;
    size_t count;
    size_t capacity;
} TokenList;

void addToken(TokenList* tokenList, Token token);


void printToken(Token token);
void printTokenList(TokenList tokenList);
const char* getTokenTypeString(TokenType tokenType);