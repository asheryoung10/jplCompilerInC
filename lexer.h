#pragma once
#include "token.h"
#include "sourceCode.h"
#include <stdbool.h>


TokenList lexSourceCode(SourceCode sourceCode);
void freeTokenList(TokenList* tokenList);
TokenType isKeyword(const char *start, size_t length);
TokenType isSingleCharacterTokenType(char character);
bool isValidCharacter(char character);
bool isDigit(char character);
bool isLetter(char character);
Token isKeywordOrVariable(
    const char *start,
    size_t line,
    size_t column);
