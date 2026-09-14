#include "ast.h"
#include <stdio.h>
#include <stdlib.h>

void printAST(AST ast) {
    for(int i = 0; i < ast.commandCount; i++) {
        Command current = *ast.commands[i];
        switch(current.type) {
            case(CMD_READ):
                printf("(ReadCmd %.*s(VarLValue %.*s))\n", (int)current.data.read.filenameLength, current.data.read.filename, (int)current.data.read.argument.length, current.data.read.argument.start);
            break;
            default:
            printf("Unknown command type.\n");
        }
    }
}

void addCommand(AST* ast, Command* command) {

    if (ast->commandCount >= ast->capacity) {

        size_t newCapacity = ast->capacity == 0
            ? 4096
            : ast->capacity * 2;

        Command** newList = realloc(
            ast->commands,
            sizeof(Command*) * newCapacity
        );

        if (newList == NULL) {
            printf("Realloc returned null. Aborting.");
            exit(EXIT_FAILURE);
            return;
        }

        ast->commands = newList;
        ast->capacity = newCapacity;
    }

    ast->commands[ast->commandCount] = command;
    ast->commandCount++;
}