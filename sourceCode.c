#include "sourceCode.h"
#include <stdio.h>
#include <stdlib.h>

SourceCode readSourceCode(const char* filename) {
    FILE* sourceCodeFile = fopen(filename, "r");
    if(sourceCodeFile == NULL) {
        return (SourceCode){.characters=NULL, .characterCount=0, .filename = NULL};
    }

    fseek(sourceCodeFile, 0, SEEK_END);
    size_t byteCount = ftell(sourceCodeFile);
    rewind(sourceCodeFile);

    char* characterBuffer = malloc(byteCount+1);

    if(characterBuffer == NULL) {
        fclose(sourceCodeFile);
        return (SourceCode){.characters=NULL, .characterCount=0, .filename=NULL};
    }

    size_t bytesRead = fread(characterBuffer, 1, byteCount, sourceCodeFile);
    characterBuffer[bytesRead] = '\0';

    fclose(sourceCodeFile);

    return (SourceCode){.characters = characterBuffer, .characterCount = bytesRead, .filename = filename};
    




}
void freeSourceCode(SourceCode* sourceCode) {
    free(sourceCode->characters);
    sourceCode->characterCount = 0;
}
void printSourceCode(const SourceCode sourceCode) {
    printf("%s\n", sourceCode.characters);
};


void printSourceCodeCharacters(SourceCode sourceCode) {
    size_t line = 0;
    size_t column = 0;
    for(size_t i = 0; i < sourceCode.characterCount; i++) {
        char current = sourceCode.characters[i];
        if(current == '\n') {
            printf("Character: '\\n', Code: [%d], \tLine: [%lu], \tColumn: [%lu]\n", current, line, column);
            line++;
            column = 0;
        }else {
            printf("Character: '%c',  Code: [%d], \tLine: [%lu], \tColumn: [%lu]\n", current, current, line, column);
            column++;
        }
    }
}

