#pragma once
#include <stddef.h>

typedef struct {
    char* characters;
    size_t characterCount;
} SourceCode;


SourceCode readSourceCode(const char* filename);
void freeSourceCode(SourceCode* sourceCode);
void printSourceCode(SourceCode sourceCode);
void printSourceCodeCharacters(SourceCode sourceCode);
