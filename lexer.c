#include "lexer.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#define LEX_FAILURE_TOKEN_LIST \
    (TokenList) { .capacity = 0, .count = 0, .tokens = NULL }

TokenList lexSourceCode(SourceCode sourceCode)
{
    size_t currentLine = 1;
    size_t currentColumn = 1;
    size_t currentIndex = 0;
    TokenList tokenList = (TokenList){.capacity = 0, .count = 0, .tokens = NULL};

    // Check for invalid characters.
    for (int i = 0; i < sourceCode.characterCount; i++)
    {
        const char *currentLocation = &sourceCode.characters[i];
        if (!isValidCharacter(*currentLocation))
        {
            //printf("Invalid character '%c'[%d] at line %lu column %lu.\n", *currentLocation, *currentLocation, currentLine, currentColumn);
            printf("Invalid character\n");
            freeTokenList(&tokenList);
            return LEX_FAILURE_TOKEN_LIST;
        }
    }

    // Lex tokens.
    while (currentIndex < sourceCode.characterCount)
    {
        const char *currentLocation = &sourceCode.characters[currentIndex];

        // Handle space
        if (*currentLocation == ' ')
        {
            currentIndex++;
            currentColumn++;
            continue;
        }

        // Handle single character exclusive tokens.
        TokenType type = isSingleCharacterTokenType(*currentLocation);
        if (type != TOKEN_NOT_FOUND)
        {
            bool isNewline = type == TOKEN_NEWLINE;
            bool isConsectiveNewline = isNewline && (tokenList.count > 0 && tokenList.tokens[tokenList.count - 1].type == TOKEN_NEWLINE);
            if (!isConsectiveNewline)
            {
                addToken(&tokenList, (Token){.column = currentColumn, .line = currentLine, .type = type, .originInSource = currentLocation, .lengthInSource = 1});
            }
            if (isNewline)
            {
                currentLine++;
                currentIndex++;
                currentColumn = 1;
            }
            else
            {
                currentIndex += 1;
                currentColumn += 1;
            }
            continue;
        }

        // Handle multi-line comments
        if (strncmp(currentLocation, "/*", 2) == 0)
        {
            currentIndex += 2;
            currentColumn += 2;
            bool closing = false;

            while (
                currentIndex < sourceCode.characterCount)
            {
                if(!(!(sourceCode.characters[currentIndex] == '*' &&
                  sourceCode.characters[currentIndex + 1] == '/'))){
                    closing = true;
                    break;
                  }
                if (sourceCode.characters[currentIndex] == '\n')
                {
                    currentLine++;
                    currentColumn = 0;
                }
                else
                {
                    currentColumn++;
                }

                currentIndex++;
            }
            if(!closing) {
                printf("Missing closing\n");
                return LEX_FAILURE_TOKEN_LIST;
            }
            // Skip the closing */
            if (currentIndex < sourceCode.characterCount)
            {
                currentIndex += 2;
                currentColumn += 2;
            }
            if(tokenList.count == 0) {
                tokenList.tokens = malloc(sizeof(Token)); // indicate not null
            }
            continue;
        }

        // Handle strings.
        if (*currentLocation == '\"')
        {
            const char *beginning = currentLocation;
            size_t length = 1;
            while (currentIndex < sourceCode.characterCount)
            {
                currentIndex++;
                length++;
                if (sourceCode.characters[currentIndex] == '\n')
                {
                    printf("%s:%lu:%lu: Could not find closing \" for string sequence.\n", sourceCode.filename, currentLine, currentColumn);
                    freeTokenList(&tokenList);
                    return LEX_FAILURE_TOKEN_LIST;
                }
                if (sourceCode.characters[currentIndex] == '"')
                {
                    break;
                }
            }
            addToken(&tokenList, (Token){.column = currentColumn, .line = currentLine, .type = TOKEN_STRING, .originInSource = currentLocation, .lengthInSource = length});
            currentColumn += length;
            currentIndex++;
            continue;
        }
        // Handle dot
        if (*currentLocation == '.' && !isDigit(currentLocation[1]))
        {
            addToken(&tokenList, (Token){.column = currentColumn, .line = currentLine, .type = TOKEN_DOT, .originInSource = currentLocation, .lengthInSource = 1});
            currentIndex++;
            currentColumn++;
            continue;
        }

        // Handle floats with leading decimal point.
        if (*currentLocation == '.' && isDigit(currentLocation[1]))
        {
            currentIndex++;
            size_t length = 1;
            while (currentIndex < sourceCode.characterCount)
            {
                currentIndex++;
                length++;
                if (!isDigit(sourceCode.characters[currentIndex]))
                {
                    break;
                }
            }
            addToken(&tokenList, (Token){.column = currentColumn, .line = currentLine, .type = TOKEN_FLOATVAL, .originInSource = currentLocation, .lengthInSource = length});
            currentColumn += length;
            continue;
        }

        // Handle floats with leading digits
        if (isDigit(*currentLocation))
        {
            size_t length = 1;
            bool encounteredDecimalPoint = false;
            while (currentIndex < sourceCode.characterCount)
            {
                currentIndex++;
                if (sourceCode.characters[currentIndex] == '.')
                {
                    if (encounteredDecimalPoint)
                    {
                        length++;
                        currentIndex++;
                        break;
                    }
                    encounteredDecimalPoint = true;
                }
                else if (!isDigit(sourceCode.characters[currentIndex]))
                {
                    break;
                }
                length++;
            }
            addToken(&tokenList, (Token){.column = currentColumn, .line = currentLine, .type = encounteredDecimalPoint ? TOKEN_FLOATVAL : TOKEN_INTVAL, .originInSource = currentLocation, .lengthInSource = length});
            currentColumn += length;
            continue;
        }

        // Handle newline escapes
        if (strncmp(currentLocation, "\\\n", 2) == 0)
        {
            currentIndex += 2;
            currentColumn = 1;
            currentLine++;
            continue;
        }
        // Handle single-line comments
        if (strncmp(currentLocation, "//", 2) == 0)
        {
            currentIndex += 2;
            currentColumn += 2;

            // Skip everything until the newline
            while (
                currentIndex < sourceCode.characterCount &&
                sourceCode.characters[currentIndex] != '\n')
            {
                currentIndex++;
                currentColumn++;
            }

            // Do not consume the newline here.
            // Let the lexer create a TOKEN_NEWLINE for it.
            continue;
        }

        // Handle >=
        if (strncmp(currentLocation, ">=", 2) == 0)
        {
            addToken(&tokenList, (Token){
                                     .column = currentColumn,
                                     .line = currentLine,
                                     .type = TOKEN_OP,
                                     .originInSource = currentLocation,
                                     .lengthInSource = 2});

            currentIndex += 2;
            currentColumn += 2;
            continue;
        }

        // Handle <=
        if (strncmp(currentLocation, "<=", 2) == 0)
        {
            addToken(&tokenList, (Token){
                                     .column = currentColumn,
                                     .line = currentLine,
                                     .type = TOKEN_OP,
                                     .originInSource = currentLocation,
                                     .lengthInSource = 2});

            currentIndex += 2;
            currentColumn += 2;
            continue;
        }

        // Handle ||
        if (strncmp(currentLocation, "||", 2) == 0)
        {
            addToken(&tokenList, (Token){
                                     .column = currentColumn,
                                     .line = currentLine,
                                     .type = TOKEN_OP,
                                     .originInSource = currentLocation,
                                     .lengthInSource = 2});

            currentIndex += 2;
            currentColumn += 2;
            continue;
        }

        // Handle &&
        if (strncmp(currentLocation, "&&", 2) == 0)
        {
            addToken(&tokenList, (Token){
                                     .column = currentColumn,
                                     .line = currentLine,
                                     .type = TOKEN_OP,
                                     .originInSource = currentLocation,
                                     .lengthInSource = 2});

            currentIndex += 2;
            currentColumn += 2;
            continue;
        }
        // Handle ==
        if (strncmp(currentLocation, "==", 2) == 0)
        {
            addToken(&tokenList, (Token){
                                     .column = currentColumn,
                                     .line = currentLine,
                                     .type = TOKEN_OP,
                                     .originInSource = currentLocation,
                                     .lengthInSource = 2});

            currentIndex += 2;
            currentColumn += 2;
            continue;
        }

        // Handle !=
        if (strncmp(currentLocation, "!=", 2) == 0)
        {
            addToken(&tokenList, (Token){
                                     .column = currentColumn,
                                     .line = currentLine,
                                     .type = TOKEN_OP,
                                     .originInSource = currentLocation,
                                     .lengthInSource = 2});

            currentIndex += 2;
            currentColumn += 2;
            continue;
        }
        // Handle other operators
        if (
            *currentLocation == '<' ||
            *currentLocation == '>' ||
            *currentLocation == '!' ||
            *currentLocation == '/')
        {
            addToken(&tokenList, (Token){
                                     .column = currentColumn,
                                     .line = currentLine,
                                     .type = TOKEN_OP,
                                     .originInSource = currentLocation,
                                     .lengthInSource = 1});

            currentIndex++;
            currentColumn++;
            continue;
        }

        // Handle =
        if (*currentLocation == '=')
        {
            addToken(&tokenList, (Token){
                                     .column = currentColumn,
                                     .line = currentLine,
                                     .type = TOKEN_EQUALS,
                                     .originInSource = currentLocation,
                                     .lengthInSource = 1});

            currentIndex++;
            currentColumn++;
            continue;
        }

        Token tokenToAdd = isKeywordOrVariable(
            currentLocation,
            currentLine,
            currentColumn);

        if (tokenToAdd.type != TOKEN_NOT_FOUND)
        {
            addToken(&tokenList, tokenToAdd);

            currentIndex += tokenToAdd.lengthInSource;
            currentColumn += tokenToAdd.lengthInSource;

            continue;
        }

        printf("%s:%lu:%lu: Out of place char: %c\n", sourceCode.filename, currentLine, currentColumn, *currentLocation);
        freeTokenList(&tokenList);
        return LEX_FAILURE_TOKEN_LIST;
    }

    return tokenList;
}
void freeTokenList(TokenList *tokenList)
{
    free(tokenList->tokens);
    tokenList->count = 0;
}
bool isDigit(char character)
{
    return character >= '0' && character <= '9';
}
bool isValidCharacter(char character)
{
    return !(character < 32 || character > 126) || character == 10;
}
TokenType isKeyword(const char *start, size_t length)
{
    if (length == 5 && strncmp(start, "array", 5) == 0)
        return TOKEN_ARRAY;

    if (length == 6 && strncmp(start, "assert", 6) == 0)
        return TOKEN_ASSERT;

    if (length == 4 && strncmp(start, "bool", 4) == 0)
        return TOKEN_BOOL;

    if (length == 4 && strncmp(start, "else", 4) == 0)
        return TOKEN_ELSE;

    if (length == 5 && strncmp(start, "false", 5) == 0)
        return TOKEN_FALSE;

    if (length == 5 && strncmp(start, "float", 5) == 0)
        return TOKEN_FLOAT;

    if (length == 2 && strncmp(start, "fn", 2) == 0)
        return TOKEN_FN;

    if (length == 2 && strncmp(start, "if", 2) == 0)
        return TOKEN_IF;

    if (length == 5 && strncmp(start, "image", 5) == 0)
        return TOKEN_IMAGE;

    if (length == 3 && strncmp(start, "int", 3) == 0)
        return TOKEN_INT;

    if (length == 3 && strncmp(start, "let", 3) == 0)
        return TOKEN_LET;

    if (length == 5 && strncmp(start, "print", 5) == 0)
        return TOKEN_PRINT;

    if (length == 4 && strncmp(start, "read", 4) == 0)
        return TOKEN_READ;

    if (length == 6 && strncmp(start, "return", 6) == 0)
        return TOKEN_RETURN;

    if (length == 4 && strncmp(start, "show", 4) == 0)
        return TOKEN_SHOW;

    if (length == 6 && strncmp(start, "struct", 6) == 0)
        return TOKEN_STRUCT;

    if (length == 3 && strncmp(start, "sum", 3) == 0)
        return TOKEN_SUM;

    if (length == 4 && strncmp(start, "then", 4) == 0)
        return TOKEN_THEN;

    if (length == 4 && strncmp(start, "time", 4) == 0)
        return TOKEN_TIME;

    if (length == 2 && strncmp(start, "to", 2) == 0)
        return TOKEN_TO;

    if (length == 4 && strncmp(start, "true", 4) == 0)
        return TOKEN_TRUE;

    if (length == 4 && strncmp(start, "void", 4) == 0)
        return TOKEN_VOID;

    if (length == 5 && strncmp(start, "write", 5) == 0)
        return TOKEN_WRITE;

    return TOKEN_NOT_FOUND;
}

TokenType isSingleCharacterTokenType(char character)
{
    switch (character)
    {
    case ':':
        return TOKEN_COLON;
    case ',':
        return TOKEN_COMMA;
    case '{':
        return TOKEN_LCURLY;
    case '}':
        return TOKEN_RCURLY;
    case '(':
        return TOKEN_LPAREN;
    case ')':
        return TOKEN_RPAREN;
    case '[':
        return TOKEN_LSQUARE;
    case ']':
        return TOKEN_RSQUARE;
    case '\n':
        return TOKEN_NEWLINE;

    case '+':
    case '-':
    case '*':
    case '%':
        return TOKEN_OP;

    default:
        return TOKEN_NOT_FOUND;
    }
}

Token isKeywordOrVariable(
    const char *start,
    size_t line,
    size_t column)
{
    if (!isLetter(*start))
    {
        return (Token){
            .type = TOKEN_NOT_FOUND};
    }

    size_t length = 1;

    while (
        isLetter(start[length]) ||
        isDigit(start[length]) ||
        start[length] == '_')
    {
        length++;
    }

    TokenType type = isKeyword(start, length);

    if (type == TOKEN_NOT_FOUND)
    {
        type = TOKEN_VARIABLE;
    }

    return (Token){
        .column = column,
        .line = line,
        .type = type,
        .originInSource = start,
        .lengthInSource = length};
}

bool isLetter(char character)
{
    return (character >= 'a' && character <= 'z') ||
           (character >= 'A' && character <= 'Z');
}
