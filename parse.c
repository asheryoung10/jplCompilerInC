#include "parse.h"
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#define PARSE_INVALID_AST (AST){.commandCount = 0, .commands = NULL};

bool matchToken(int *currentIndex, TokenList tokenList, TokenType type)
{
    if (*currentIndex > tokenList.count)
        return false;
    if (tokenList.tokens[*currentIndex].type == type)
    {
        (*currentIndex)++;
        return true;
    }
    printf("Did not match token for %s, instead got: ", getTokenTypeString(type));
    printToken(tokenList.tokens[*currentIndex]);
    return false;
}
bool peekToken(int *currentIndex, TokenList tokenList, TokenType type)
{
    if (*currentIndex > tokenList.count)
        return false;
    if (tokenList.tokens[*currentIndex].type == type)
    {
        return true;
    }
    return false;
}
bool peekNextToken(int *currentIndex, TokenList tokenList, TokenType type)
{
    if (*currentIndex+1 > tokenList.count)
        return false;
    if (tokenList.tokens[*currentIndex+1].type == type)
    {
        return true;
    }
    return false;
}
Type parseType(int *currentIndex, TokenList tokenList)
{
    if (*currentIndex >= tokenList.count)
        return (Type){.kind = TYPE_INVALID};

    Token current = tokenList.tokens[*currentIndex];
    if(peekNextToken(currentIndex, tokenList, TOKEN_LSQUARE)) {
        // TODO: Parse types
    }

    switch (current.type)
    {
    case TOKEN_INT:
        (*currentIndex)++;
        return (Type){.kind = TYPE_INT};

    case TOKEN_BOOL:
        (*currentIndex)++;
        return (Type){.kind = TYPE_BOOL};

    case TOKEN_FLOAT:
        (*currentIndex)++;
        return (Type){.kind = TYPE_FLOAT};

    case TOKEN_VOID:
        (*currentIndex)++;
        return (Type){.kind = TYPE_VOID};

    case TOKEN_VARIABLE:
    {
        Type type = {
            .kind = TYPE_VARIABLE,
            .data.variable.start = current.originInSource,
            .data.variable.length = current.lengthInSource
        };

        (*currentIndex)++;
        return type;
    }

    case TOKEN_LSQUARE:
    {
        printf("Unhandled lsquare type\n");
        return (Type){.kind = TYPE_INVALID};
    }
    default:
        printf("Invalid type\n");
        printToken(current);
        return (Type){.kind = TYPE_INVALID};
    }
}
#define PARSE_INVALID_EXPR \
    (Expr) { .type = EXPR_INVALID }
Expr parseExpr(int *currentIndex, TokenList tokenList)
{
    while (*currentIndex < tokenList.count)
    {
        Token current = tokenList.tokens[*currentIndex];
        switch (current.type)
        {
        case (TOKEN_VARIABLE):
        {
            Expr expr = (Expr){.type = EXPR_VARIABLE, .data.variable.length = current.lengthInSource, .data.variable.start = current.originInSource};
            (*currentIndex)++;
            return expr;
        }
        case (TOKEN_INTVAL):
        {
            errno = 0;
            char *string = strndup(current.originInSource, current.lengthInSource);
            long value = strtol(string, NULL, 10);
            if (errno == ERANGE)
            {
                printf("OVERFLOW INT");
                return PARSE_INVALID_EXPR;
            }

            Expr expr = (Expr){.type = EXPR_INT, .data.intExpr.value = value};
            (*currentIndex)++;
            return expr;
        }
        case (TOKEN_FLOATVAL):
        {
            errno = 0;
            char *string = strndup(current.originInSource, current.lengthInSource);
            double value = strtod(string, NULL);
            if (errno == ERANGE)
            {
                printf("OVERFLOW FLOAT");
                return PARSE_INVALID_EXPR;
            }
            free(string);

            Expr expr = (Expr){.type = EXPR_FLOAT, .data.floatExpr.value = value};
            (*currentIndex)++;
            return expr;
        }

        case (TOKEN_TRUE):
            (*currentIndex)++;
            return (Expr){.type = EXPR_TRUE};
        case (TOKEN_FALSE):
            (*currentIndex)++;
            return (Expr){.type = EXPR_FALSE};
        default:
            if (!matchToken(currentIndex, tokenList, TOKEN_LSQUARE)) {
                printf("Invalid array expr\n");
                printToken(tokenList.tokens[*currentIndex]);
                return PARSE_INVALID_EXPR;
            }
            if(peekToken(currentIndex, tokenList, TOKEN_RSQUARE)) {
                (*currentIndex)++;
                return (Expr){.type = EXPR_ARRAY, .data.array.capacity = 0, .data.array.count=0, .data.array.expressions=NULL};
            }
            Expr arrayExpr = {.type = EXPR_ARRAY, .data.array.capacity = 0, .data.array.count =0, .data.array.expressions=NULL};
            bool first = true;
            while(first || peekToken(currentIndex, tokenList, TOKEN_COMMA)) {
                if(!first) {
                    (*currentIndex)++;
                }
                first = false;
                Expr expr = parseExpr(currentIndex, tokenList);    
                if(expr.type == EXPR_INVALID) {
                    printf("Invalid expression parsed.\n");
                    return PARSE_INVALID_EXPR;
                }

                addExpression(&arrayExpr, expr);
            }
            if (!matchToken(currentIndex, tokenList, TOKEN_RSQUARE)) {
                printf("Invalid array expr\n");
                printToken(tokenList.tokens[*currentIndex]);
                return PARSE_INVALID_EXPR;
            }
            return arrayExpr;
        }
        return PARSE_INVALID_EXPR;
    }
    return PARSE_INVALID_EXPR;
}
Command *parseReadCommand(TokenList tokenList, int *currentIndex)
{
    Command *command = malloc(sizeof(Command));
    command->type = CMD_READ;
    (*currentIndex)++;
    if (!matchToken(currentIndex, tokenList, TOKEN_IMAGE))
        return NULL;
    if (!matchToken(currentIndex, tokenList, TOKEN_STRING))
        return NULL;
    command->data.read.filename = tokenList.tokens[*currentIndex - 1].originInSource;
    command->data.read.filenameLength = tokenList.tokens[*currentIndex - 1].lengthInSource;
    if (!matchToken(currentIndex, tokenList, TOKEN_TO))
        return NULL;
    if (!matchToken(currentIndex, tokenList, TOKEN_VARIABLE))
        return NULL;
    command->data.read.argument.length = tokenList.tokens[*currentIndex - 1].lengthInSource;
    command->data.read.argument.start = tokenList.tokens[*currentIndex - 1].originInSource;
    command->data.read.argument.type = LVALUE_VARIABLE;
    return command;
}
Command *parseWriteCommand(TokenList tokenList, int *currentIndex)
{
    Command *command = malloc(sizeof(Command));
    command->type = CMD_WRITE;
    (*currentIndex)++;
    if (!matchToken(currentIndex, tokenList, TOKEN_IMAGE))
        return NULL;
    Expr expr = parseExpr(currentIndex, tokenList);
    if (expr.type == EXPR_INVALID)
        return NULL;
    command->data.write.expression = expr;
    if (!matchToken(currentIndex, tokenList, TOKEN_TO))
        return NULL;
    if (!matchToken(currentIndex, tokenList, TOKEN_STRING))
        return NULL;
    command->data.write.filename = tokenList.tokens[*currentIndex - 1].originInSource;
    command->data.write.filenameLength = tokenList.tokens[*currentIndex - 1].lengthInSource;
    return command;
}
Command *parseLetCommand(TokenList tokenList, int *currentIndex)
{
    Command *command = malloc(sizeof(Command));
    command->type = CMD_LET;
    (*currentIndex)++;
    if (!matchToken(currentIndex, tokenList, TOKEN_VARIABLE))
        return NULL;
    command->data.let.lvalue.length = tokenList.tokens[*currentIndex - 1].lengthInSource;
    command->data.let.lvalue.start = tokenList.tokens[*currentIndex - 1].originInSource;
    command->data.let.lvalue.type = LVALUE_VARIABLE;

    if (!matchToken(currentIndex, tokenList, TOKEN_EQUALS))
        return NULL;
    Expr expr = parseExpr(currentIndex, tokenList);
    if (expr.type == EXPR_INVALID)
        return NULL;
    command->data.let.expression = expr;
    return command;
}
Command *parseAssertCommand(TokenList tokenList, int *currentIndex)
{
    Command *command = malloc(sizeof(Command));
    command->type = CMD_ASSERT;
    (*currentIndex)++;
    Expr expr = parseExpr(currentIndex, tokenList);
    if (expr.type == EXPR_INVALID)
        return NULL;

    command->data.assertCmd.expression = expr;
    if (!matchToken(currentIndex, tokenList, TOKEN_COMMA))
        return NULL;
    if (!matchToken(currentIndex, tokenList, TOKEN_STRING))
        return NULL;
    command->data.assertCmd.message = tokenList.tokens[*currentIndex - 1].originInSource;
    command->data.assertCmd.messageLength = tokenList.tokens[*currentIndex - 1].lengthInSource;
    return command;
}
Command *parseShowCommand(TokenList tokenList, int *currentIndex)
{
    Command *command = malloc(sizeof(Command));
    command->type = CMD_SHOW;
    (*currentIndex)++;
    Expr expr = parseExpr(currentIndex, tokenList);
    if (expr.type == EXPR_INVALID)
        return NULL;
    command->data.show.expression = expr;
    return command;
}
Command *parsePrintCommand(TokenList tokenList, int *currentIndex)
{
    Command *command = malloc(sizeof(Command));
    command->type = CMD_PRINT;
    (*currentIndex)++;
    if (!matchToken(currentIndex, tokenList, TOKEN_STRING))
        return NULL;
    command->data.print.string = tokenList.tokens[*currentIndex - 1].originInSource;
    command->data.print.length = tokenList.tokens[*currentIndex - 1].lengthInSource;
    return command;
}
Command *parseStructCommand(TokenList tokenList, int *currentIndex)
{
    Command *command = malloc(sizeof(Command));
    command->type = CMD_STRUCT;
    (*currentIndex)++;
    if (!matchToken(currentIndex, tokenList, TOKEN_VARIABLE))
        return NULL;
    command->data.structDecl.name = tokenList.tokens[*currentIndex - 1].originInSource;
    command->data.structDecl.nameLength = tokenList.tokens[*currentIndex - 1].lengthInSource;
    if (!matchToken(currentIndex, tokenList, TOKEN_LCURLY))
        return NULL;
    if (!matchToken(currentIndex, tokenList, TOKEN_NEWLINE))
        return NULL;
    if (!matchToken(currentIndex, tokenList, TOKEN_VARIABLE))
        return NULL;
    command->data.structDecl.fieldCapacity = 0;
    command->data.structDecl.fieldCount = 0;
    command->data.structDecl.fields = NULL;
    bool first = true;
    while(first || !peekToken(currentIndex, tokenList, TOKEN_RCURLY)) {
        if(!first) {
            (*currentIndex)++;
        }
        first = false;
        const char* name = tokenList.tokens[*currentIndex - 1].originInSource;
        size_t nameLength = tokenList.tokens[*currentIndex - 1].lengthInSource; 
        if (!matchToken(currentIndex, tokenList, TOKEN_COLON))
            return NULL;
        Type type = parseType(currentIndex, tokenList);
        if(type.kind == TYPE_INVALID) {
            printf("Invalid type\n");
            return NULL;
        }
        addField(command,name, nameLength, type);
        if (!matchToken(currentIndex, tokenList, TOKEN_NEWLINE))
            return NULL;
    }
    if (!matchToken(currentIndex, tokenList, TOKEN_RCURLY))
            return NULL;
    return command;
}

Command *parseTimeCommand(TokenList tokenList, int *currentIndex);
Command* parseCommand(TokenList tokenList, int *currentIndex) {
    switch (tokenList.tokens[*currentIndex].type)
        {
        case TOKEN_READ:
            return parseReadCommand(tokenList, currentIndex);
            break;
        case TOKEN_LET:
            return parseLetCommand(tokenList, currentIndex);
            break;
        case TOKEN_WRITE:
            return parseWriteCommand(tokenList, currentIndex);
            break;
        case TOKEN_SHOW:
            return parseShowCommand(tokenList, currentIndex);
            break;
        case TOKEN_PRINT:
            return parsePrintCommand(tokenList, currentIndex);
            break;
        case TOKEN_TIME:
            return parseTimeCommand(tokenList, currentIndex);
            break;
        case TOKEN_ASSERT:
            return parseAssertCommand(tokenList, currentIndex);
            break;
        case TOKEN_STRUCT:
            return parseStructCommand(tokenList, currentIndex);
            break;
        default:
            printf("Unhandled parse case %d.\n", tokenList.tokens[*currentIndex].type);
            printToken(tokenList.tokens[*currentIndex]);
            return NULL;
        }
}
Command *parseTimeCommand(TokenList tokenList, int *currentIndex)
{
    Command *command = malloc(sizeof(Command));
    command->type = CMD_TIME;
    (*currentIndex)++;
    Command *subcommand = parseCommand(tokenList, currentIndex);
    command->data.time.command = subcommand;
    if(subcommand == NULL) {
        printf("Invalid time command\n");
        return NULL;
    }
    return command;
}
AST parse(TokenList tokenList)
{
    int currentIndex = 0;
    AST ast = {.commandCount = 0, .commands = NULL};
    while (currentIndex < tokenList.count)
    {
        if (tokenList.tokens[currentIndex].type == TOKEN_END_OF_FILE)
        {
            break;
        }
        if (tokenList.tokens[currentIndex].type == TOKEN_NEWLINE)
        {
            currentIndex++;
            continue;
        }
        Command *command = NULL;
        command = parseCommand(tokenList, &currentIndex); 
        if (command == NULL) {
            fflush(stdout);
            return PARSE_INVALID_AST;
        }
        addCommand(&ast, command);

    }
    return ast;
}