#include "ast.h"
#include <stdio.h>
#include <stdlib.h>
void printType(Type type)
{
    switch (type.kind)
    {
    case TYPE_INT:
        printf("(IntType)");
        break;

    case TYPE_BOOL:
        printf("(BoolType)");
        break;

    case TYPE_FLOAT:
        printf("(FloatType)");
        break;

    case TYPE_VOID:
        printf("(VoidType)");
        break;

    case TYPE_VARIABLE:
        printf("(StructType %.*s)",
               (int)type.data.variable.length,
               type.data.variable.start);
        break;

    case TYPE_ARRAY:
        printf("(ArrayType ");
        printType(*type.data.array.type);
        printf(" %zu)", type.data.array.rank);
        break;

    case TYPE_INVALID:
        printf("(InvalidType)");
        break;

    default:
        printf("(UnknownType)");
        break;
    }
}

void printLValue(LValue lvalue)
{
    if (lvalue.type == LVALUE_VARIABLE)
    {
        printf("(VarLValue %.*s)",
               (int)lvalue.length,
               lvalue.start);
        return;
    }

    if (lvalue.type == LVALUE_INDEX)
    {
        printf("(ArrayLValue %.*s",
               (int)lvalue.length,
               lvalue.start);

        for (size_t i = 0; i < lvalue.indexCount; i++)
        {
            printf(" %.*s",
                   (int)lvalue.indexLengths[i],
                   lvalue.indices[i]);
        }

        printf(")");
        return;
    }

    printf("(InvalidLValue)");
}

void printField(Command command, size_t index)
{
    printf("%.*s ",
           (int)command.data.structDecl.fields[index].nameLength,
           command.data.structDecl.fields[index].name);

    printType(command.data.structDecl.fields[index].type);
}

void printExpr(Expr expr)
{
    switch (expr.type)
    {
    case EXPR_VARIABLE:
        printf("(VarExpr %.*s)",
               (int)expr.data.variable.length,
               expr.data.variable.start);
        return;

    case EXPR_INT:
        printf("(IntExpr %ld)",
               expr.data.intExpr.value);
        return;

    case EXPR_FLOAT:
        printf("(FloatExpr %ld)",
               (long)expr.data.floatExpr.value);
        return;

    case EXPR_TRUE:
        printf("(TrueExpr)");
        return;

    case EXPR_FALSE:
        printf("(FalseExpr)");
        return;

    case EXPR_VOID:
        printf("(VoidExpr)");
        return;

    case EXPR_ARRAY:
        if (expr.data.array.count == 0)
        {
            printf("(ArrayLiteralExpr)");
            return;
        }

        printf("(ArrayLiteralExpr ");

        for (size_t i = 0; i < expr.data.array.count; i++)
        {
            printExpr(expr.data.array.expressions[i]);

            if (i + 1 < expr.data.array.count)
                printf(" ");
        }

        printf(")");
        return;

    case EXPR_STRUCT:
        printf("(StructLiteralExpr %.*s",
               (int)expr.data.structExpr.length,
               expr.data.structExpr.start);

        for (size_t i = 0; i < expr.data.structExpr.count; i++)
        {
            printf(" ");
            printExpr(expr.data.structExpr.expressions[i]);
        }

        printf(")");
        return;

    case EXPR_DOT:
        printf("(DotExpr ");
        printExpr(*expr.data.member.expression);
        printf(" %.*s)",
               (int)expr.data.member.memberLength,
               expr.data.member.member);
        return;

    case EXPR_INDEX:
        printf("(ArrayIndexExpr ");
        printExpr(*expr.data.index.expression);

        for (size_t i = 0; i < expr.data.index.count; i++)
        {
            printf(" ");
            printExpr(expr.data.index.indices[i]);
        }

        printf(")");
        return;

    case EXPR_CALL:
        printf("(CallExpr %.*s",
               (int)expr.data.call.length,
               expr.data.call.start);

        for (size_t i = 0; i < expr.data.call.count; i++)
        {
            printf(" ");
            printExpr(expr.data.call.arguments[i]);
        }

        printf(")");
        return;

    case EXPR_INVALID:
        printf("(InvalidExpr)");
        return;

    default:
        printf("(UnknownExpr)");
        return;
    }
}

void printStmt(Stmt stmt)
{
    switch (stmt.type)
    {
    case STMT_LET:
        printf("(LetStmt ");
        printLValue(stmt.data.let.lvalue);
        printf(" ");
        printExpr(stmt.data.let.expression);
        printf(")");
        return;

    case STMT_ASSERT:
        printf("(AssertStmt ");
        printExpr(stmt.data.assertStmt.expression);
        printf(" %.*s)",
               (int)stmt.data.assertStmt.messageLength,
               stmt.data.assertStmt.message);
        return;

    case STMT_RETURN:
        printf("(ReturnStmt ");
        printExpr(stmt.data.returnStmt.expression);
        printf(")");
        return;

    case STMT_INVALID:
        printf("(InvalidStmt)");
        return;

    default:
        printf("(UnknownStmt)");
        return;
    }
}

void printBinding(Binding binding)
{
    printLValue(binding.lvalue);
    printf(" ");
    printType(binding.type);
}

void printCommand(Command command)
{
    switch (command.type)
    {
    case CMD_READ:
        printf("(ReadCmd %.*s ",
               (int)command.data.read.filenameLength,
               command.data.read.filename);

        printLValue(command.data.read.argument);

        printf(")");
        break;

    case CMD_WRITE:
        printf("(WriteCmd ");
        printExpr(command.data.write.expression);
        printf(" %.*s)",
               (int)command.data.write.filenameLength,
               command.data.write.filename);
        break;

    case CMD_LET:
        printf("(LetCmd ");
        printLValue(command.data.let.lvalue);
        printf(" ");
        printExpr(command.data.let.expression);
        printf(")");
        break;

    case CMD_ASSERT:
        printf("(AssertCmd ");
        printExpr(command.data.assertCmd.expression);
        printf(" %.*s)",
               (int)command.data.assertCmd.messageLength,
               command.data.assertCmd.message);
        break;

    case CMD_PRINT:
        printf("(PrintCmd %.*s)",
               (int)command.data.print.length,
               command.data.print.string);
        break;

    case CMD_SHOW:
        printf("(ShowCmd ");
        printExpr(command.data.show.expression);
        printf(")");
        break;

    case CMD_TIME:
        printf("(TimeCmd ");
        printCommand(*command.data.time.command);
        printf(")");
        break;

    case CMD_STRUCT:
        printf("(StructCmd %.*s",
               (int)command.data.structDecl.nameLength,
               command.data.structDecl.name);

        for (size_t i = 0; i < command.data.structDecl.fieldCount; i++)
        {
            printf(" ");
            printField(command, i);
        }

        printf(")");
        break;

    case CMD_FN:
        printf("(FnCmd %.*s",
               (int)command.data.function.nameLength,
               command.data.function.name);

        printf(" (");
        if (command.data.function.bindingCount > 0)
        {
            printf("(");

            for (size_t i = 0;
                 i < command.data.function.bindingCount;
                 i++)
            {
                printBinding(command.data.function.bindings[i]);

                if (i + 1 < command.data.function.bindingCount)
                    printf(" ");
            }

            printf(")");
        }
        else
        {
            printf("()");
        }
        printf(")");

        printf(" ");
        printType(command.data.function.returnType);

        for (size_t i = 0;
             i < command.data.function.statementCount;
             i++)
        {
            printf(" ");
            printStmt(command.data.function.statements[i]);
        }

        printf(")");
        break;

    case CMD_INVALID:
        printf("(InvalidCmd)");
        break;

    default:
        printf("(UnknownCmd)");
        break;
    }
}

void printAST(AST ast)
{
    for (size_t i = 0; i < ast.commandCount; i++)
    {
        printCommand(*ast.commands[i]);
        printf("\n");
    }
}

void addCommand(AST *ast, Command *command)
{
    if (ast->commandCount >= ast->capacity)
    {

        size_t newCapacity = ast->capacity == 0
                                 ? 4096
                                 : ast->capacity * 2;

        Command **newList = realloc(
            ast->commands,
            sizeof(Command *) * newCapacity);

        if (newList == NULL)
        {
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
void addExpression(Expr *expression, Expr value)
{
    Expr **list = NULL;
    size_t *count = NULL;
    size_t *capacity = NULL;

    switch (expression->type)
    {
    case EXPR_ARRAY:
        list = &expression->data.array.expressions;
        count = &expression->data.array.count;
        capacity = &expression->data.array.capacity;
        break;

    case EXPR_STRUCT:
        list = &expression->data.structExpr.expressions;
        count = &expression->data.structExpr.count;
        capacity = &expression->data.structExpr.capacity;
        break;

    case EXPR_INDEX:
        list = &expression->data.index.indices;
        count = &expression->data.index.count;
        capacity = &expression->data.index.capacity;
        break;

    case EXPR_CALL:
        list = &expression->data.call.arguments;
        count = &expression->data.call.count;
        capacity = &expression->data.call.capacity;
        break;

    default:
        printf("Cannot add expression to expression type %d\n", expression->type);
        return;
    }

    if (*count >= *capacity)
    {
        size_t newCapacity = *capacity == 0 ? 4096 : *capacity * 2;

        Expr *newList = realloc(*list, sizeof(Expr) * newCapacity);

        if (newList == NULL)
        {
            printf("Realloc returned null. Aborting.");
            exit(EXIT_FAILURE);
        }

        *list = newList;
        *capacity = newCapacity;
    }

    (*list)[*count] = value;
    (*count)++;
}
void addStructExpression(Expr *expression, Expr value)
{
    if (expression->data.structExpr.count >=
        expression->data.structExpr.capacity)
    {
        size_t newCapacity =
            expression->data.structExpr.capacity == 0
                ? 4096
                : expression->data.structExpr.capacity * 2;

        Expr *newList = realloc(
            expression->data.structExpr.expressions,
            sizeof(*expression->data.structExpr.expressions) * newCapacity
        );

        if (newList == NULL)
        {
            printf("Realloc returned null. Aborting.");
            exit(EXIT_FAILURE);
        }

        expression->data.structExpr.expressions = newList;
        expression->data.structExpr.capacity = newCapacity;
    }

    expression->data.structExpr.expressions[
        expression->data.structExpr.count
    ] = value;

    expression->data.structExpr.count++;
}

void addIndexExpression(Expr *expression, Expr value)
{
    if (expression->data.index.count >=
        expression->data.index.capacity)
    {
        size_t newCapacity =
            expression->data.index.capacity == 0
                ? 4096
                : expression->data.index.capacity * 2;

        Expr *newList = realloc(
            expression->data.index.indices,
            sizeof(*expression->data.index.indices) * newCapacity
        );

        if (newList == NULL)
        {
            printf("Realloc returned null. Aborting.");
            exit(EXIT_FAILURE);
        }

        expression->data.index.indices = newList;
        expression->data.index.capacity = newCapacity;
    }

    expression->data.index.indices[
        expression->data.index.count
    ] = value;

    expression->data.index.count++;
}

void addCallArgument(Expr *expression, Expr value)
{
    if (expression->data.call.count >=
        expression->data.call.capacity)
    {
        size_t newCapacity =
            expression->data.call.capacity == 0
                ? 4096
                : expression->data.call.capacity * 2;

        Expr *newList = realloc(
            expression->data.call.arguments,
            sizeof(*expression->data.call.arguments) * newCapacity
        );

        if (newList == NULL)
        {
            printf("Realloc returned null. Aborting.");
            exit(EXIT_FAILURE);
        }

        expression->data.call.arguments = newList;
        expression->data.call.capacity = newCapacity;
    }

    expression->data.call.arguments[
        expression->data.call.count
    ] = value;

    expression->data.call.count++;
}

void addField(Command *structure, const char *name, size_t nameLength, Type type)
{
    if (structure->data.structDecl.fieldCount >=
        structure->data.structDecl.fieldCapacity)
    {
        size_t newCapacity =
            structure->data.structDecl.fieldCapacity == 0
                ? 4096
                : structure->data.structDecl.fieldCapacity * 2;

        void *newList = realloc(
            structure->data.structDecl.fields,
            sizeof(*structure->data.structDecl.fields) * newCapacity
        );

        if (newList == NULL)
        {
            printf("Realloc returned null. Aborting.");
            exit(EXIT_FAILURE);
            return;
        }

        structure->data.structDecl.fields = newList;
        structure->data.structDecl.fieldCapacity = newCapacity;
    }

    structure->data.structDecl.fields[
        structure->data.structDecl.fieldCount
    ].name = name;

    structure->data.structDecl.fields[
        structure->data.structDecl.fieldCount
    ].nameLength = nameLength;

    structure->data.structDecl.fields[
        structure->data.structDecl.fieldCount
    ].type = type;

    structure->data.structDecl.fieldCount++;
}
void addBinding(Command *function, Binding binding)
{
    if (function->data.function.bindingCount >=
        function->data.function.bindingCapacity)
    {
        size_t newCapacity =
            function->data.function.bindingCapacity == 0
                ? 4096
                : function->data.function.bindingCapacity * 2;

        Binding *newList = realloc(
            function->data.function.bindings,
            sizeof(*function->data.function.bindings) * newCapacity
        );

        if (newList == NULL)
        {
            printf("Realloc returned null. Aborting.");
            exit(EXIT_FAILURE);
            return;
        }

        function->data.function.bindings = newList;
        function->data.function.bindingCapacity = newCapacity;
    }

    function->data.function.bindings[
        function->data.function.bindingCount
    ] = binding;

    function->data.function.bindingCount++;
}

void addStatement(Command *function, Stmt statement)
{
    if (function->data.function.statementCount >=
        function->data.function.statementCapacity)
    {
        size_t newCapacity =
            function->data.function.statementCapacity == 0
                ? 4096
                : function->data.function.statementCapacity * 2;

        Stmt *newList = realloc(
            function->data.function.statements,
            sizeof(*function->data.function.statements) * newCapacity
        );

        if (newList == NULL)
        {
            printf("Realloc returned null. Aborting.");
            exit(EXIT_FAILURE);
            return;
        }

        function->data.function.statements = newList;
        function->data.function.statementCapacity = newCapacity;
    }

    function->data.function.statements[
        function->data.function.statementCount
    ] = statement;

    function->data.function.statementCount++;
}