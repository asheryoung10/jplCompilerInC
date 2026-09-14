#pragma once
#include <stddef.h>

typedef enum
{
    EXPR_INT,
    EXPR_FLOAT,
    EXPR_TRUE,
    EXPR_FALSE,
    EXPR_VARIABLE,
    EXPR_ARRAY
} ExprType;

typedef struct Expr Expr;

struct Expr
{
    ExprType type;

    union
    {
        struct
        {
            long value;
        } intExpr;

        struct
        {
            double value;
        } floatExpr;

        struct
        {
            const char* start;
            size_t length;
        } variable;

        struct
        {
            Expr** expressions;
            size_t count;
            size_t capacity;
        } array;
    } data;
};

typedef enum
{
    LVALUE_VARIABLE
} LValueType;

typedef struct
{
    LValueType type;

    const char* start;
    size_t length;
} LValue;

typedef enum
{
    CMD_READ,
    CMD_WRITE,
    CMD_LET,
    CMD_ASSERT,
    CMD_PRINT,
    CMD_SHOW,
    CMD_TIME
} CommandType;

typedef struct Command Command;

struct Command
{
    CommandType type;

    union
    {
        struct
        {
            const char* filename;
            size_t filenameLength;
            LValue argument;
        } read;

        struct
        {
            Expr* expression;
            const char* filename;
            size_t filenameLength;
        } write;

        struct
        {
            LValue lvalue;
            Expr* expression;
        } let;

        struct
        {
            Expr* expression;
            const char* message;
            size_t messageLength;
        } assertCmd;

        struct
        {
            const char* string;
            size_t length;
        } print;

        struct
        {
            Expr* expression;
        } show;

        struct
        {
            Command* command;
        } time;
    } data;
};

typedef struct{
    Command** commands;
    size_t commandCount;
    size_t capacity;
} AST;

void freeAST(AST* ast);
void addCommand(AST* ast, Command* command);
void printAST(AST ast);