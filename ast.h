#pragma once
#include <stddef.h>

typedef enum
{
    TYPE_INT,
    TYPE_BOOL,
    TYPE_FLOAT,
    TYPE_ARRAY,
    TYPE_VARIABLE,
    TYPE_VOID,
    TYPE_INVALID
} TypeKind;

typedef struct Type Type;

struct Type
{
    TypeKind kind;

    union
    {
        struct
        {
            Type* type;
            size_t rank;
        } array;

        struct
        {
            const char* start;
            size_t length;
        } variable;
    } data;
};

typedef enum
{
    EXPR_INT,
    EXPR_FLOAT,
    EXPR_TRUE,
    EXPR_FALSE,
    EXPR_VARIABLE,
    EXPR_VOID,
    EXPR_ARRAY,
    EXPR_STRUCT,
    EXPR_MEMBER,
    EXPR_INDEX,
    EXPR_CALL,
    EXPR_INVALID
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
            Expr* expressions;
            size_t count;
            size_t capacity;
        } array;

     
        struct
        {
            const char* start;
            size_t length;

            Expr* expressions;
            size_t count;
            size_t capacity;
        } structExpr;

      
        struct
        {
            Expr* expression;

            const char* member;
            size_t memberLength;
        } member;

       
        struct
        {
            Expr* expression;

            Expr* indices;
            size_t count;
            size_t capacity;
        } index;

      
        struct
        {
            const char* start;
            size_t length;

            Expr* arguments;
            size_t count;
            size_t capacity;
        } call;

    } data;
};


typedef enum
{
    LVALUE_VARIABLE,
    LVALUE_INDEX
} LValueType;

typedef struct
{
    LValueType type;

    const char* start;
    size_t length;

    /*
     * Only used by LVALUE_INDEX:
     *
     * <variable> [ <variable> , ... ]
     */
    const char** indices;
    size_t* indexLengths;
    size_t indexCount;
    size_t indexCapacity;

} LValue;


typedef struct
{
    LValue lvalue;
    Type type;
} Binding;

typedef enum
{
    STMT_LET,
    STMT_ASSERT,
    STMT_RETURN,
    STMT_INVALID
} StmtType;

typedef struct Stmt Stmt;

struct Stmt
{
    StmtType type;

    union
    {
        struct
        {
            LValue lvalue;
            Expr expression;
        } let;

        struct
        {
            Expr expression;

            const char* message;
            size_t messageLength;
        } assertStmt;

        struct
        {
            Expr expression;
        } returnStmt;

    } data;
};


typedef enum
{
    CMD_READ,
    CMD_WRITE,
    CMD_LET,
    CMD_ASSERT,
    CMD_PRINT,
    CMD_SHOW,
    CMD_TIME,
    CMD_FN,
    CMD_STRUCT,
    CMD_INVALID
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
            Expr expression;

            const char* filename;
            size_t filenameLength;
        } write;

    
        struct
        {
            LValue lvalue;
            Expr expression;
        } let;

     
        struct
        {
            Expr expression;

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
            Expr expression;
        } show;

        
        struct
        {
            Command* command;
        } time;


        struct
        {
            const char* name;
            size_t nameLength;

            Binding* bindings;
            size_t bindingCount;
            size_t bindingCapacity;

            Type returnType;

            Stmt* statements;
            size_t statementCount;
            size_t statementCapacity;
        } function;

   
        struct
        {
            const char* name;
            size_t nameLength;

            struct
            {
                const char* name;
                size_t nameLength;
                Type type;
            } * fields;

            size_t fieldCount;
            size_t fieldCapacity;
        } structDecl;

    } data;
};




typedef struct
{
    Command** commands;
    size_t commandCount;
    size_t capacity;
} AST;



void freeAST(AST* ast);

void addCommand(AST* ast, Command* command);

void addExpression(Expr* array, Expr expression);
void addField(Command *structure, const char *name, size_t nameLength, Type type);

void printAST(AST ast);
void printExpr(Expr expr);