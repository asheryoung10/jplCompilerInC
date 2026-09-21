#include "parse.h"
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

Expr parseExpr(int *currentIndex, TokenList tokenList);
Type parseType(int *currentIndex, TokenList tokenList);

#define PARSE_INVALID_AST (AST){.commandCount = 0, .commands = NULL};

bool matchToken(int *currentIndex, TokenList tokenList, TokenType type)
{
    if (*currentIndex >= tokenList.count)
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
    if (*currentIndex >= tokenList.count)
        return false;
    if (tokenList.tokens[*currentIndex].type == type)
    {
        return true;
    }
    return false;
}
bool peekNextToken(int *currentIndex, TokenList tokenList, TokenType type)
{
    if (*currentIndex + 1 >= tokenList.count)
        return false;
    if (tokenList.tokens[*currentIndex + 1].type == type)
    {
        return true;
    }
    return false;
}
LValue parseLValue(int *currentIndex, TokenList tokenList)
{
    LValue lvalue = {
        .type = LVALUE_VARIABLE,
        .start = NULL,
        .length = 0,
        .indices = NULL,
        .indexLengths = NULL,
        .indexCount = 0,
        .indexCapacity = 0};

    if (*currentIndex >= tokenList.count)
    {
        printf("failed to parse lvalue");
        return lvalue;
    }

    if (!peekToken(currentIndex, tokenList, TOKEN_VARIABLE))
        return lvalue;

    Token variable = tokenList.tokens[*currentIndex];

    lvalue.start = variable.originInSource;
    lvalue.length = variable.lengthInSource;

    (*currentIndex)++;

    if (!peekToken(currentIndex, tokenList, TOKEN_LSQUARE))
        return lvalue;

    lvalue.type = LVALUE_INDEX;

    (*currentIndex)++;

    if (!peekToken(currentIndex, tokenList, TOKEN_VARIABLE))
    {
        printf("failed to parse lvalue");
        return (LValue){0};
    }

    int firstIndex = *currentIndex;
    size_t count = 0;

    while (true)
    {
        if (!peekToken(currentIndex, tokenList, TOKEN_VARIABLE))
            return (LValue){0};

        count++;
        (*currentIndex)++;

        if (!peekToken(currentIndex, tokenList, TOKEN_COMMA))
            break;

        (*currentIndex)++;

        if (!peekToken(currentIndex, tokenList, TOKEN_VARIABLE))
        {
            printf("failed to parse lvalue");
            return (LValue){0};
        }
    }

    if (!matchToken(currentIndex, tokenList, TOKEN_RSQUARE))
    {
        printf("failed to parse lvalue");
        return (LValue){0};
    }

    lvalue.indices = malloc(count * sizeof(*lvalue.indices));
    lvalue.indexLengths = malloc(count * sizeof(*lvalue.indexLengths));

    if (lvalue.indices == NULL || lvalue.indexLengths == NULL)
    {
        free(lvalue.indices);
        free(lvalue.indexLengths);
        printf("failed to parse lvalue");
        return (LValue){0};
    }

    lvalue.indexCount = count;
    lvalue.indexCapacity = count;

    int secondIndex = firstIndex;

    for (size_t i = 0; i < count; i++)
    {
        Token index = tokenList.tokens[secondIndex];

        lvalue.indices[i] = index.originInSource;
        lvalue.indexLengths[i] = index.lengthInSource;

        secondIndex++;

        if (i + 1 < count)
            secondIndex++;
    }

    return lvalue;
}
Stmt parseStmt(int *currentIndex, TokenList tokenList)
{
    if (*currentIndex >= tokenList.count)
    {
        printf("Failed to parse statement: unexpected end of input\n");
        return (Stmt){.type = STMT_INVALID};
    }

    Token current = tokenList.tokens[*currentIndex];

    switch (current.type)
    {
    case TOKEN_LET:
    {
        (*currentIndex)++;

        LValue lvalue = parseLValue(currentIndex, tokenList);

        if (lvalue.start == NULL)
        {
            printf("Failed to parse let statement: expected lvalue after 'let'\n");
            return (Stmt){.type = STMT_INVALID};
        }

        if (!matchToken(currentIndex, tokenList, TOKEN_EQUALS))
        {
            printf("Failed to parse let statement: expected '=' after lvalue\n");
            return (Stmt){.type = STMT_INVALID};
        }

        Expr expression = parseExpr(currentIndex, tokenList);

        if (expression.type == EXPR_INVALID)
        {
            printf("Failed to parse let statement: invalid expression after '='\n");
            return (Stmt){.type = STMT_INVALID};
        }

        return (Stmt){
            .type = STMT_LET,
            .data.let = {
                .lvalue = lvalue,
                .expression = expression}};
    }

    case TOKEN_ASSERT:
    {
        (*currentIndex)++;

        Expr expression = parseExpr(currentIndex, tokenList);

        if (expression.type == EXPR_INVALID)
        {
            printf("Failed to parse assert statement: invalid expression after 'assert'\n");
            return (Stmt){.type = STMT_INVALID};
        }

        if (!matchToken(currentIndex, tokenList, TOKEN_COMMA))
        {
            printf("Failed to parse assert statement: expected ',' after expression\n");
            return (Stmt){.type = STMT_INVALID};
        }

        if (!matchToken(currentIndex, tokenList, TOKEN_STRING))
        {
            printf("Failed to parse assert statement: expected string after ','\n");

            if (*currentIndex < tokenList.count)
                printToken(tokenList.tokens[*currentIndex]);

            return (Stmt){.type = STMT_INVALID};
        }

        Token string = tokenList.tokens[*currentIndex - 1];

        return (Stmt){
            .type = STMT_ASSERT,
            .data.assertStmt = {
                .expression = expression,
                .message = string.originInSource,
                .messageLength = string.lengthInSource}};
    }

    case TOKEN_RETURN:
    {
        (*currentIndex)++;

        Expr expression = parseExpr(currentIndex, tokenList);

        if (expression.type == EXPR_INVALID)
        {
            printf("Failed to parse return statement: invalid expression after 'return'\n");
            return (Stmt){.type = STMT_INVALID};
        }

        return (Stmt){
            .type = STMT_RETURN,
            .data.returnStmt = {
                .expression = expression}};
    }

    default:
        printf("Failed to parse statement: expected 'let', 'assert', or 'return'\n");
        printToken(current);

        return (Stmt){.type = STMT_INVALID};
    }
}
Binding parseBinding(int *currentIndex, TokenList tokenList)
{
    Binding binding = {
        .lvalue = {0},
        .type = {.kind = TYPE_INVALID}};

    if (*currentIndex >= tokenList.count)
    {
        printf("Failed to parse binding: unexpected end of input\n");
        return binding;
    }

    LValue lvalue = parseLValue(currentIndex, tokenList);

    if (lvalue.start == NULL)
    {
        printf("Failed to parse binding: expected lvalue\n");
        return binding;
    }

    if (!matchToken(currentIndex, tokenList, TOKEN_COLON))
    {
        printf("Failed to parse binding: expected ':' after lvalue\n");
        return binding;
    }

    Type type = parseType(currentIndex, tokenList);

    if (type.kind == TYPE_INVALID)
    {
        printf("Failed to parse binding: expected type after ':'\n");
        return binding;
    }

    binding.lvalue = lvalue;
    binding.type = type;

    return binding;
}
Type parseNonArrayType(int *currentIndex, TokenList tokenList)
{
    Token current = tokenList.tokens[*currentIndex];
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
            .data.variable.length = current.lengthInSource};

        (*currentIndex)++;
        return type;
    }
    default:
        printf("Invalid type\n");
        printToken(current);
        return (Type){.kind = TYPE_INVALID};
    }
}

Type parseType(int *currentIndex, TokenList tokenList)
{
    if (*currentIndex >= tokenList.count)
        return (Type){.kind = TYPE_INVALID};

    Type type = parseNonArrayType(currentIndex, tokenList);

    if (type.kind == TYPE_INVALID)
        return type;

    while (peekToken(currentIndex, tokenList, TOKEN_LSQUARE))
    {
        if (!matchToken(currentIndex, tokenList, TOKEN_LSQUARE))
            return (Type){.kind = TYPE_INVALID};

        size_t rank = 1;

        while (peekToken(currentIndex, tokenList, TOKEN_COMMA)) {
            (*currentIndex)++;
            rank++;
        }

        if (!matchToken(currentIndex, tokenList, TOKEN_RSQUARE))
            return (Type){.kind = TYPE_INVALID};

        Type *elementType = malloc(sizeof(Type));

        if (elementType == NULL)
            return (Type){.kind = TYPE_INVALID};

        *elementType = type;

        type = (Type){
            .kind = TYPE_ARRAY,
            .data.array.rank = rank,
            .data.array.type = elementType};
    }

    return type;
}
#define PARSE_INVALID_EXPR \
    (Expr) { .type = EXPR_INVALID }

Expr parsePrimaryExpr(int *currentIndex, TokenList tokenList)
{
    if (*currentIndex >= tokenList.count)
    {
        printf("Failed to parse expression: unexpected end of input\n");
        return PARSE_INVALID_EXPR;
    }

    Token current = tokenList.tokens[*currentIndex];

    switch (current.type)
    {
    case TOKEN_VARIABLE:
    {
        (*currentIndex)++;

        if (peekToken(currentIndex, tokenList, TOKEN_LCURLY))
        {
            Expr expr = {
                .type = EXPR_STRUCT,
                .data.structExpr.start = current.originInSource,
                .data.structExpr.length = current.lengthInSource,
                .data.structExpr.expressions = NULL,
                .data.structExpr.count = 0,
                .data.structExpr.capacity = 0};

            (*currentIndex)++;

            if (peekToken(currentIndex, tokenList, TOKEN_RCURLY))
            {
                (*currentIndex)++;
                return expr;
            }

            while (true)
            {
                Expr value = parseExpr(currentIndex, tokenList);

                if (value.type == EXPR_INVALID)
                {
                    printf("Failed to parse struct expression '%.*s': invalid expression\n",
                           (int)current.lengthInSource,
                           current.originInSource);
                    return PARSE_INVALID_EXPR;
                }

                addExpression(&expr, value);

                if (!peekToken(currentIndex, tokenList, TOKEN_COMMA))
                    break;

                (*currentIndex)++;

                if (peekToken(currentIndex, tokenList, TOKEN_RCURLY))
                {
                    printf("Failed to parse struct expression '%.*s': trailing comma\n",
                           (int)current.lengthInSource,
                           current.originInSource);
                    return PARSE_INVALID_EXPR;
                }
            }

            if (!matchToken(currentIndex, tokenList, TOKEN_RCURLY))
            {
                printf("Failed to parse struct expression '%.*s': expected '}'\n",
                       (int)current.lengthInSource,
                       current.originInSource);
                return PARSE_INVALID_EXPR;
            }

            return expr;
        }
        return (Expr){
            .type = EXPR_VARIABLE,
            .data.variable.length = current.lengthInSource,
            .data.variable.start = current.originInSource};
    }

    case TOKEN_INTVAL:
    {
        char *string = strndup(
            current.originInSource,
            current.lengthInSource);

        if (string == NULL)
        {
            printf("Failed to parse integer expression: allocation failed\n");
            return PARSE_INVALID_EXPR;
        }

        errno = 0;
        char *end = NULL;
        long value = strtol(string, &end, 10);

        if (errno == ERANGE || end == string || *end != '\0')
        {
            printf("Failed to parse integer expression: invalid integer\n");
            free(string);
            return PARSE_INVALID_EXPR;
        }

        free(string);
        (*currentIndex)++;

        return (Expr){
            .type = EXPR_INT,
            .data.intExpr.value = value};
    }

    case TOKEN_FLOATVAL:
    {
        char *string = strndup(
            current.originInSource,
            current.lengthInSource);

        if (string == NULL)
        {
            printf("Failed to parse float expression: allocation failed\n");
            return PARSE_INVALID_EXPR;
        }

        errno = 0;
        char *end = NULL;
        double value = strtod(string, &end);

        if (errno == ERANGE || end == string || *end != '\0')
        {
            printf("Failed to parse float expression: invalid float\n");
            free(string);
            return PARSE_INVALID_EXPR;
        }

        free(string);
        (*currentIndex)++;

        return (Expr){
            .type = EXPR_FLOAT,
            .data.floatExpr.value = value};
    }

    case TOKEN_TRUE:
        (*currentIndex)++;
        return (Expr){.type = EXPR_TRUE};

    case TOKEN_FALSE:
        (*currentIndex)++;
        return (Expr){.type = EXPR_FALSE};

    case TOKEN_VOID:
        (*currentIndex)++;
        return (Expr){.type = EXPR_VOID};

    case TOKEN_LSQUARE:
    {
        (*currentIndex)++;

        Expr arrayExpr = {
            .type = EXPR_ARRAY,
            .data.array.capacity = 0,
            .data.array.count = 0,
            .data.array.expressions = NULL};

        if (peekToken(currentIndex, tokenList, TOKEN_RSQUARE))
        {
            (*currentIndex)++;
            return arrayExpr;
        }

        while (true)
        {
            Expr expr = parseExpr(currentIndex, tokenList);

            if (expr.type == EXPR_INVALID)
            {
                printf("Failed to parse array expression: invalid expression\n");
                return PARSE_INVALID_EXPR;
            }

            addExpression(&arrayExpr, expr);

            if (!peekToken(currentIndex, tokenList, TOKEN_COMMA))
                break;

            (*currentIndex)++;

            if (peekToken(currentIndex, tokenList, TOKEN_RSQUARE))
            {
                printf("Failed to parse array expression: trailing comma\n");
                return PARSE_INVALID_EXPR;
            }
        }

        if (!matchToken(currentIndex, tokenList, TOKEN_RSQUARE))
        {
            printf("Failed to parse array expression: expected ']'\n");
            return PARSE_INVALID_EXPR;
        }

        return arrayExpr;
    }

    default:
        printf("Failed to parse expression: unexpected token\n");
        printToken(current);
        return PARSE_INVALID_EXPR;
    }
}

Expr parsePostfixExpr(int *currentIndex, TokenList tokenList)
{
    Expr expr = parsePrimaryExpr(currentIndex, tokenList);

    if (expr.type == EXPR_INVALID)
        return expr;

    while (*currentIndex < tokenList.count)
    {
        if (peekToken(currentIndex, tokenList, TOKEN_DOT))
        {
            Expr object = expr;

            (*currentIndex)++;

            if (!matchToken(currentIndex, tokenList, TOKEN_VARIABLE))
            {
                printf("Failed to parse member expression: expected variable after '.'\n");
                return PARSE_INVALID_EXPR;
            }

            Token member = tokenList.tokens[*currentIndex - 1];

            Expr *objectPtr = malloc(sizeof(Expr));

            if (objectPtr == NULL)
            {
                printf("Failed to parse member expression: allocation failed\n");
                return PARSE_INVALID_EXPR;
            }

            *objectPtr = object;

            expr = (Expr){
                .type = EXPR_DOT,
                .data.member = {
                    .expression = objectPtr,
                    .member = member.originInSource,
                    .memberLength = member.lengthInSource}};
        }
        else if (peekToken(currentIndex, tokenList, TOKEN_LSQUARE))
        {
            (*currentIndex)++;

            Expr indexExpr = {
                .type = EXPR_INDEX,
                .data.index = {
                    .expression = malloc(sizeof(Expr)),
                    .indices = NULL,
                    .count = 0,
                    .capacity = 0}};

            if (indexExpr.data.index.expression == NULL)
            {
                printf("Failed to parse index expression: allocation failed\n");
                return PARSE_INVALID_EXPR;
            }

            *indexExpr.data.index.expression = expr;

            if (peekToken(currentIndex, tokenList, TOKEN_RSQUARE))
            {
                (*currentIndex)++;
                return indexExpr;
            }

            while (true)
            {
                Expr index = parseExpr(currentIndex, tokenList);

                if (index.type == EXPR_INVALID)
                {
                    printf("Failed to parse index expression: invalid index\n");
                    free(indexExpr.data.index.expression);
                    return PARSE_INVALID_EXPR;
                }

                addExpression(&indexExpr, index);

                if (!peekToken(currentIndex, tokenList, TOKEN_COMMA))
                    break;

                (*currentIndex)++;

                if (peekToken(currentIndex, tokenList, TOKEN_RSQUARE))
                {
                    printf("Failed to parse index expression: trailing comma\n");
                    free(indexExpr.data.index.expression);
                    return PARSE_INVALID_EXPR;
                }
            }

            if (!matchToken(currentIndex, tokenList, TOKEN_RSQUARE))
            {
                printf("Failed to parse index expression: expected ']'\n");
                free(indexExpr.data.index.expression);
                return PARSE_INVALID_EXPR;
            }

            expr = indexExpr;
        }
        else if (peekToken(currentIndex, tokenList, TOKEN_LPAREN))
        {
            if (expr.type != EXPR_VARIABLE)
            {
                printf("Failed to parse call expression: expected function variable\n");
                return PARSE_INVALID_EXPR;
            }

            const char *name = expr.data.variable.start;
            size_t nameLength = expr.data.variable.length;

            (*currentIndex)++;

            Expr callExpr = {
                .type = EXPR_CALL,
                .data.call = {
                    .start = name,
                    .length = nameLength,
                    .arguments = NULL,
                    .count = 0,
                    .capacity = 0}};

            if (peekToken(currentIndex, tokenList, TOKEN_RPAREN))
            {
                (*currentIndex)++;
                expr = callExpr;
                continue;
            }

            while (true)
            {
                Expr argument = parseExpr(currentIndex, tokenList);

                if (argument.type == EXPR_INVALID)
                {
                    printf("Failed to parse call expression '%.*s': invalid argument\n",
                           (int)nameLength,
                           name);
                    return PARSE_INVALID_EXPR;
                }

                addExpression(&callExpr, argument);

                if (!peekToken(currentIndex, tokenList, TOKEN_COMMA))
                    break;

                (*currentIndex)++;

                if (peekToken(currentIndex, tokenList, TOKEN_RPAREN))
                {
                    printf("Failed to parse call expression '%.*s': trailing comma\n",
                           (int)nameLength,
                           name);
                    return PARSE_INVALID_EXPR;
                }
            }

            if (!matchToken(currentIndex, tokenList, TOKEN_RPAREN))
            {
                printf("Failed to parse call expression '%.*s': expected ')'\n",
                       (int)nameLength,
                       name);
                return PARSE_INVALID_EXPR;
            }

            expr = callExpr;
        }
        else
        {
            break;
        }
    }

    return expr;
}

Expr parseExpr(int *currentIndex, TokenList tokenList)
{
    return parsePostfixExpr(currentIndex, tokenList);
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
Command *parseStructCommand(TokenList tokenList, int *currentIndex)
{
    Command *command = malloc(sizeof(Command));

    if (command == NULL)
    {
        printf("Failed to parse struct command: allocation failed\n");
        return NULL;
    }

    command->type = CMD_STRUCT;
    command->data.structDecl.name = NULL;
    command->data.structDecl.nameLength = 0;
    command->data.structDecl.fields = NULL;
    command->data.structDecl.fieldCount = 0;
    command->data.structDecl.fieldCapacity = 0;

    if (!matchToken(currentIndex, tokenList, TOKEN_STRUCT))
    {
        printf("Failed to parse struct command: expected 'struct'\n");
        free(command);
        return NULL;
    }

    if (!matchToken(currentIndex, tokenList, TOKEN_VARIABLE))
    {
        printf("Failed to parse struct command: expected struct name after 'struct'\n");
        free(command);
        return NULL;
    }

    Token name = tokenList.tokens[*currentIndex - 1];

    command->data.structDecl.name = name.originInSource;
    command->data.structDecl.nameLength = name.lengthInSource;

    if (!matchToken(currentIndex, tokenList, TOKEN_LCURLY))
    {
        printf("Failed to parse struct command: expected '{' after struct name\n");
        free(command);
        return NULL;
    }

    if (!matchToken(currentIndex, tokenList, TOKEN_NEWLINE))
    {
        printf("Failed to parse struct command: expected newline after '{'\n");
        free(command);
        return NULL;
    }

    while (!peekToken(currentIndex, tokenList, TOKEN_RCURLY))
    {
        if (*currentIndex >= tokenList.count)
        {
            printf("Failed to parse struct command: expected '}' before end of input\n");
            free(command);
            return NULL;
        }

        if (!matchToken(currentIndex, tokenList, TOKEN_VARIABLE))
        {
            printf("Failed to parse struct command: expected field name\n");
            free(command);
            return NULL;
        }

        Token fieldName = tokenList.tokens[*currentIndex - 1];

        if (!matchToken(currentIndex, tokenList, TOKEN_COLON))
        {
            printf("Failed to parse struct field '%.*s': expected ':'\n",
                   (int)fieldName.lengthInSource,
                   fieldName.originInSource);
            free(command);
            return NULL;
        }

        Type type = parseType(currentIndex, tokenList);

        if (type.kind == TYPE_INVALID)
        {
            printf("Failed to parse struct field '%.*s': invalid type\n",
                   (int)fieldName.lengthInSource,
                   fieldName.originInSource);
            free(command);
            return NULL;
        }
        if (!matchToken(currentIndex, tokenList, TOKEN_NEWLINE))
        {
            printf("Failed to parse struct field '%.*s': expected newline after type\n",
                   (int)fieldName.lengthInSource,
                   fieldName.originInSource);
            free(command);
            return NULL;
        }

        addField(
            command,
            fieldName.originInSource,
            fieldName.lengthInSource,
            type);
    }

    if (!matchToken(currentIndex, tokenList, TOKEN_RCURLY))
    {
        printf("Failed to parse struct command: expected '}'\n");
        free(command);
        return NULL;
    }

    return command;
}
Command *parseFunctionCommand(TokenList tokenList, int *currentIndex)
{
    Command *command = malloc(sizeof(Command));

    if (command == NULL)
    {
        printf("Failed to parse function command: allocation failed\n");
        return NULL;
    }

    command->type = CMD_FN;

    command->data.function.name = NULL;
    command->data.function.nameLength = 0;

    command->data.function.bindings = NULL;
    command->data.function.bindingCount = 0;
    command->data.function.bindingCapacity = 0;

    command->data.function.returnType = (Type){.kind = TYPE_INVALID};

    command->data.function.statements = NULL;
    command->data.function.statementCount = 0;
    command->data.function.statementCapacity = 0;

    if (!matchToken(currentIndex, tokenList, TOKEN_FN))
    {
        printf("Failed to parse function: expected 'fn'\n");
        free(command);
        return NULL;
    }

    if (!matchToken(currentIndex, tokenList, TOKEN_VARIABLE))
    {
        printf("Failed to parse function: expected function name after 'fn'\n");
        free(command);
        return NULL;
    }

    Token name = tokenList.tokens[*currentIndex - 1];

    command->data.function.name = name.originInSource;
    command->data.function.nameLength = name.lengthInSource;

    if (!matchToken(currentIndex, tokenList, TOKEN_LPAREN))
    {
        printf("Failed to parse function '%.*s': expected '('\n",
               (int)name.lengthInSource,
               name.originInSource);
        free(command);
        return NULL;
    }

    if (!peekToken(currentIndex, tokenList, TOKEN_RPAREN))
    {
        while (true)
        {
            Binding binding = parseBinding(currentIndex, tokenList);

            if (binding.type.kind == TYPE_INVALID)
            {
                printf("Failed to parse function '%.*s': invalid binding\n",
                       (int)name.lengthInSource,
                       name.originInSource);
                free(command);
                return NULL;
            }

            addBinding(command, binding);

            if (!peekToken(currentIndex, tokenList, TOKEN_COMMA))
                break;

            (*currentIndex)++;
        }
    }

    if (!matchToken(currentIndex, tokenList, TOKEN_RPAREN))
    {
        printf("Failed to parse function '%.*s': expected ')' after bindings\n",
               (int)name.lengthInSource,
               name.originInSource);
        free(command);
        return NULL;
    }

    if (!matchToken(currentIndex, tokenList, TOKEN_COLON))
    {
        printf("Failed to parse function '%.*s': expected ':' after ')'\n",
               (int)name.lengthInSource,
               name.originInSource);
        free(command);
        return NULL;
    }

    Type returnType = parseType(currentIndex, tokenList);

    if (returnType.kind == TYPE_INVALID)
    {
        printf("Failed to parse function '%.*s': invalid return type\n",
               (int)name.lengthInSource,
               name.originInSource);
        free(command);
        return NULL;
    }

    command->data.function.returnType = returnType;

    if (!matchToken(currentIndex, tokenList, TOKEN_LCURLY))
    {
        printf("Failed to parse function '%.*s': expected '{'\n",
               (int)name.lengthInSource,
               name.originInSource);
        free(command);
        return NULL;
    }

    if (!matchToken(currentIndex, tokenList, TOKEN_NEWLINE))
    {
        printf("Failed to parse function '%.*s': expected newline after '{'\n",
               (int)name.lengthInSource,
               name.originInSource);
        free(command);
        return NULL;
    }

    while (!peekToken(currentIndex, tokenList, TOKEN_RCURLY))
    {
        if (*currentIndex >= tokenList.count)
        {
            printf("Failed to parse function '%.*s': expected '}' before end of input\n",
                   (int)name.lengthInSource,
                   name.originInSource);
            free(command);
            return NULL;
        }

        Stmt statement = parseStmt(currentIndex, tokenList);

        if (statement.type == STMT_INVALID)
        {
            printf("Failed to parse function '%.*s': invalid statement\n",
                   (int)name.lengthInSource,
                   name.originInSource);
            free(command);
            return NULL;
        }

        addStatement(command, statement);

        if (!matchToken(currentIndex, tokenList, TOKEN_NEWLINE))
        {
            printf("Failed to parse function '%.*s': expected newline after statement\n",
                   (int)name.lengthInSource,
                   name.originInSource);
            free(command);
            return NULL;
        }
    }

    if (!matchToken(currentIndex, tokenList, TOKEN_RCURLY))
    {
        printf("Failed to parse function '%.*s': expected '}'\n",
               (int)name.lengthInSource,
               name.originInSource);
        free(command);
        return NULL;
    }

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

    command->data.let.lvalue = parseLValue(currentIndex, tokenList);
    if (command->data.let.lvalue.start == NULL)
        return NULL;

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


Command *parseTimeCommand(TokenList tokenList, int *currentIndex);
Command *parseCommand(TokenList tokenList, int *currentIndex)
{
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
    case TOKEN_FN:
        return parseFunctionCommand(tokenList, currentIndex);
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
    if (subcommand == NULL)
    {
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
        if (command == NULL)
        {
            return PARSE_INVALID_AST;
        }
        addCommand(&ast, command);

        if (currentIndex < tokenList.count &&
            tokenList.tokens[currentIndex].type != TOKEN_NEWLINE &&
            tokenList.tokens[currentIndex].type != TOKEN_END_OF_FILE)
        {
            printf("Expected newline after command, instead got: ");
            printToken(tokenList.tokens[currentIndex]);
            return PARSE_INVALID_AST;
        }
    }
    return ast;
}