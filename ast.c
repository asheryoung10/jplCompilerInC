#include "ast.h"
#include <stdio.h>
#include <stdlib.h>
void printExpr(Expr expr)
{
    switch (expr.type)
    {
    case (EXPR_VARIABLE):
        printf("(VarExpr %.*s)", (int)expr.data.variable.length, expr.data.variable.start);
        return;

    case (EXPR_INT):
        printf("(IntExpr %ld)", expr.data.intExpr.value);
        return;
    case (EXPR_FLOAT):
        printf("(FloatExpr %ld)", (long)expr.data.floatExpr.value);
        return;
    case (EXPR_FALSE):
        printf("(FalseExpr)");
        return;
    case (EXPR_TRUE):
        printf("(TrueExpr)");
        return;
    case (EXPR_ARRAY):
        if (expr.data.array.count == 0)
        {
            printf("(ArrayLiteralExpr)");
            return;
        }
        printf("(ArrayLiteralExpr ");
        for (int i = 0; i < expr.data.array.count; i++)
        {
            printExpr(expr.data.array.expressions[i]);
            if(i != expr.data.array.count-1) {
            printf(" ");
            }
        }
        printf(")");
        return;

    default:
        printf("Unhandled print expr case");
        return;
    }
}
void printCommand(Command command)
{
    switch (command.type)
    {
    case CMD_READ:
        printf("(ReadCmd %.*s (VarLValue %.*s))", (int)command.data.read.filenameLength, command.data.read.filename, (int)command.data.read.argument.length, command.data.read.argument.start);
        break;
    case CMD_WRITE:
        printf("(WriteCmd ");
        printExpr(command.data.write.expression);
        printf(" %.*s)", (int)command.data.write.filenameLength, command.data.write.filename);
        break;
    case CMD_PRINT:
        printf("(PrintCmd %.*s)", (int)command.data.print.length, command.data.print.string);
        break;
    case CMD_LET:
        printf("(LetCmd (VarLValue %.*s) ", (int)command.data.let.lvalue.length, command.data.let.lvalue.start);
        printExpr(command.data.let.expression);
        printf(")");
        break;

    case CMD_TIME:
        printf("(TimeCmd ");
        printCommand(*command.data.time.command);
        printf(")");
        break;
    case CMD_SHOW:
        printf("(ShowCmd ");
        printExpr(command.data.assertCmd.expression);
        printf(")");
        break;

    case CMD_ASSERT:
        printf("(AssertCmd ");
        printExpr(command.data.assertCmd.expression);
        printf(" %.*s)", (int)command.data.assertCmd.messageLength, command.data.assertCmd.message);
        break;
    case CMD_INVALID:
        printf("invalid command type hit");
        break;
    default:
        printf("%d", command.type);
        printf("Unknown command type.");
        break;
    }
}
void printAST(AST ast)
{
    for (int i = 0; i < ast.commandCount; i++)
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
void addExpression(Expr *array, Expr expression)
{
    if (array->data.array.count >= array->data.array.capacity)
    {
        size_t newCapacity = array->data.array.capacity == 0 ? 4096 : array->data.array.capacity * 2;
        Expr *newList = realloc(array->data.array.expressions, sizeof(Expr) * newCapacity);
        if (newList == NULL)
        {
            printf("Realloc returned null. Aborting.");
            exit(EXIT_FAILURE);
            return;
        }
        array->data.array.expressions = newList;
        array->data.array.capacity = newCapacity;
    }
    array->data.array.expressions[array->data.array.count] = expression;
    array->data.array.count++;
}