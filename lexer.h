#pragma once
#include "token.h"
#include "sourceCode.h"


TokenList lexSourceCode(SourceCode sourceCode);
void freeTokenList(TokenList* tokenList);
TokenType isKeyword(const char *start);