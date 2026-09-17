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
            printf("(StructType ");
            printf("%.*s", (int)type.data.variable.length,
                   type.data.variable.start);
            printf(")");
            break;

        case TYPE_ARRAY:
            printf("(ArrayType");

            printf("UNHANDLED");

            printf(")");
            break;
        case TYPE_INVALID:
            printf("INvalid type made it to printing ast\n");
            break;
        default:
            printf("default case\n");
            break;
    }
}

void printField(Command command, int index) {
    printf("%.*s ", (int)command.data.structDecl.fields[index].nameLength, command.data.structDecl.fields[index].name);
    printType(command.data.structDecl.fields[index].type);

}

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
    case CMD_STRUCT:
        printf("(StructCmd %.*s ", (int)command.data.structDecl.nameLength, command.data.structDecl.name);
        for (int i = 0; i < command.data.structDecl.fieldCount; i++)
        {
            printField(command, i);
            if(i != command.data.structDecl.fieldCount-1) {
            printf(" ");
            }
        }
        printf(")");

        break;

    default:
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