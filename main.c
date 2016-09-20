#include <stdio.h>
#include <stdlib.h>
#include <alloca.h>

#include <editline/readline.h>
#include <editline/history.h>

#include "gs.h"
#include "mpc.h"

#define LASSERT(Args, Condition, Error) \
        if(!(Condition)) \
        { \
                LvalFree(Args); \
                return(LvalError(Error)); \
        }

typedef struct lval
{
        int Type;
        long Number;
        char *Error;
        char *Symbol;
        unsigned int CellCount;
        struct lval **Cell;
} lval;

enum lval_type_e
{
        LVAL_TYPE_NUMBER,
        LVAL_TYPE_ERROR,
        LVAL_TYPE_SYMBOL,
        LVAL_TYPE_SEXPRESSION,
        LVAL_TYPE_QEXPRESSION
};

enum lval_error_e
{
        LVAL_ERROR_DIV_ZERO,
        LVAL_ERROR_BAD_OPERATOR,
        LVAL_ERROR_BAD_NUMBER
};

lval *
LvalNumber(long Number)
{
        lval *Self = malloc(sizeof(lval));
        Self->Type = LVAL_TYPE_NUMBER;
        Self->Number = Number;
        return(Self);
}

lval *
LvalError(char *Error)
{
        unsigned int StringLength = GSStringLength(Error);

        lval *Self = malloc(sizeof(lval));
        Self->Type = LVAL_TYPE_ERROR;
        Self->Error = malloc(StringLength + 1);
        GSStringCopy(Error, Self->Error, StringLength);
        return(Self);
}

lval *
LvalSymbol(char *Symbol)
{
        unsigned int StringLength = GSStringLength(Symbol);

        lval *Self = malloc(sizeof(lval));
        Self->Type = LVAL_TYPE_SYMBOL;
        Self->Symbol = malloc(StringLength + 1);
        GSStringCopy(Symbol, Self->Symbol, StringLength);
        return(Self);
}

lval *
LvalSExpression()
{
        lval *Self = malloc(sizeof(lval));
        Self->Type = LVAL_TYPE_SEXPRESSION;
        Self->CellCount = 0;
        Self->Cell = GSNullPtr;
        return(Self);
}

lval *
LvalQExpression()
{
        lval *Self = malloc(sizeof(lval));
        Self->Type = LVAL_TYPE_QEXPRESSION;
        Self->CellCount = 0;
        Self->Cell = GSNullPtr;
        return(Self);
}

void
LvalFree(lval *Self)
{
        switch(Self->Type)
        {
                case LVAL_TYPE_NUMBER: break;
                case LVAL_TYPE_ERROR:
                {
                        free(Self->Error);
                } break;
                case LVAL_TYPE_SYMBOL:
                {
                        free(Self->Symbol);
                } break;
                case LVAL_TYPE_QEXPRESSION:
                case LVAL_TYPE_SEXPRESSION:
                {
                        for(int I = 0; I < Self->CellCount; I++)
                        {
                                LvalFree(Self->Cell[I]);
                        }
                        free(Self->Cell);
                } break;
        }
        free(Self);
}

lval *
LvalReadNumber(mpc_ast_t *Tree)
{
        lval *Result;

        errno = 0;
        long Number = strtol(Tree->contents, NULL, 10);
        if(errno == ERANGE)
        {
                Result = LvalError("Invalid Number");
        }
        else
        {
                Result = LvalNumber(Number);
        }
        return(Result);
}

lval *
LvalAdd(lval *Self, lval *ToAdd)
{
        Self->CellCount++;
        Self->Cell = realloc(Self->Cell, sizeof(lval *) * Self->CellCount);
        Self->Cell[Self->CellCount-1] = ToAdd;
        return(Self);
}

lval *
LvalRead(mpc_ast_t *Tree)
{
        lval *Result = GSNullPtr;

        if(strstr(Tree->tag, "number")) return(LvalReadNumber(Tree));
        if(strstr(Tree->tag, "symbol")) return(LvalSymbol(Tree->contents));

        if(GSStringIsEqual(Tree->tag, ">", 1)) Result = LvalSExpression();
        if(strstr(Tree->tag, "sexpr"))         Result = LvalSExpression();
        if(strstr(Tree->tag, "qexpr"))         Result = LvalQExpression();

        for(int I=0; I<Tree->children_num; I++)
        {
                if(GSStringIsEqual(Tree->children[I]->contents, "(", 1)) continue;
                if(GSStringIsEqual(Tree->children[I]->contents, ")", 1)) continue;
                if(GSStringIsEqual(Tree->children[I]->contents, "}", 1)) continue;
                if(GSStringIsEqual(Tree->children[I]->contents, "{", 1)) continue;
                if(GSStringIsEqual(Tree->children[I]->tag, "regex", 5)) continue;
                Result = LvalAdd(Result, LvalRead(Tree->children[I]));
        }

        return(Result);
}

void LvalPrint(lval *Value);

void
LvalPrintExpression(lval *Self, char Open, char Close)
{
        putchar(Open);

        for(int I=0; I < Self->CellCount; I++)
        {
                LvalPrint(Self->Cell[I]);

                if(I != (Self->CellCount - 1))
                {
                        putchar(' ');
                }
        }

        putchar(Close);
}

void
LvalPrint(lval *Self)
{
        switch(Self->Type)
        {
                case(LVAL_TYPE_NUMBER):      printf("%li", Self->Number);         break;
                case(LVAL_TYPE_ERROR):       printf("Error: %s", Self->Error);    break;
                case(LVAL_TYPE_SYMBOL):      printf("%s", Self->Symbol);          break;
                case(LVAL_TYPE_SEXPRESSION): LvalPrintExpression(Self, '(', ')'); break;
                case(LVAL_TYPE_QEXPRESSION): LvalPrintExpression(Self, '{', '}'); break;
        }
}

void
LvalPrintLine(lval *Self)
{
        LvalPrint(Self);
        putchar('\n');
}

lval *
LvalPop(lval *Self, unsigned int Index)
{
        lval *Result = Self->Cell[Index];

        memmove(&(Self->Cell[Index]), &(Self->Cell[Index + 1]),
                sizeof(lval *) * (Self->CellCount - Index - 1));

        Self->CellCount--;

        Self->Cell = realloc(Self->Cell, sizeof(lval *) * Self->CellCount);
        return(Result);
}

lval *
LvalTake(lval *Self, unsigned int Index)
{
        lval *Result = LvalPop(Self, Index);
        LvalFree(Self);
        return(Result);
}

lval *
LvalBuiltInOperator(lval *Self, char *Operator)
{
        lval *Result = GSNullPtr;

        for(int Cell = 0; Cell < Self->CellCount; Cell++)
        {
                if(Self->Cell[Cell]->Type != LVAL_TYPE_NUMBER)
                {
                        LvalFree(Self);
                        Result = LvalError("Cannot operate on non-number");
                        return(Result);
                }
        }

        Result = LvalPop(Self, 0);

        if(GSStringIsEqual(Operator, "-", 1) && Self->CellCount == 0)
        {
                Result->Number = -(Result->Number);
        }

        while(Self->CellCount > 0)
        {
                lval *Foo = LvalPop(Self, 0);

                if(GSStringIsEqual(Operator, "+", 1))
                        Result->Number += Foo->Number;
                if(GSStringIsEqual(Operator, "-", 1))
                        Result->Number -= Foo->Number;
                if(GSStringIsEqual(Operator, "*", 1))
                        Result->Number *= Foo->Number;
                if(GSStringIsEqual(Operator, "/", 1))
                {
                        if(Foo->Number == 0)
                        {
                                LvalFree(Self);
                                LvalFree(Foo);
                                Result = LvalError("Division by zero!");
                                break;
                        }
                        Result->Number /= Foo->Number;
                }

                LvalFree(Foo);
        }

        LvalFree(Self);
        return(Result);
}

lval *
LvalBuiltInHead(lval *Self)
{
        LASSERT(Self, Self->CellCount == 1,
                "Function 'head' passed too many arguments!");
        LASSERT(Self, Self->Cell[0]->Type == LVAL_TYPE_QEXPRESSION,
                "Function 'head' passed incorrect type!");
        LASSERT(Self, Self->Cell[0]->CellCount != 0,
                "Function 'head' passed {}!");

        lval *Result = LvalTake(Self, 0);
        while(Result->CellCount > 1)
        {
                LvalFree(LvalPop(Result, 1));
        }

        return(Result);
}

lval *
LvalBuiltInTail(lval *Self)
{
        LASSERT(Self, Self->CellCount == 1,
                "Function 'tail' passed too many arguments!");
        LASSERT(Self, Self->Cell[0]->Type == LVAL_TYPE_QEXPRESSION,
                "Function 'tail' passed incorrect type!");
        LASSERT(Self, Self->Cell[0]->CellCount != 0,
                "Function 'tail' passed {}!");

        lval *Result = LvalTake(Self, 0);
        LvalFree(LvalPop(Result, 0));
        return(Result);
}

lval *
LvalBuiltInList(lval *Self)
{
        Self->Type = LVAL_TYPE_QEXPRESSION;
        return(Self);
}

lval *LvalEval(lval *Self);

lval *
LvalBuiltInEval(lval *Self)
{
        LASSERT(Self, Self->CellCount == 1,
                "Function 'eval' passed too many arguments!");
        LASSERT(Self, Self->Cell[0]->Type == LVAL_TYPE_QEXPRESSION,
                "Function 'eval' passed incorrect type!");

        lval *Result = LvalTake(Self, 0);
        Result->Type = LVAL_TYPE_SEXPRESSION;
        return(LvalEval(Result));
}

lval *
LvalJoin(lval *Left, lval *Right)
{
        while(Right->CellCount)
        {
                Left = LvalAdd(Left, LvalPop(Right, 0));
        }

        LvalFree(Right);
        return(Left);
}

lval *
LvalBuiltInJoin(lval *Self)
{
        for(int Cell = 0; Cell < Self->CellCount; Cell++)
        {
                LASSERT(Self, Self->Cell[Cell]->Type == LVAL_TYPE_QEXPRESSION,
                        "Function 'join' passed incorrect type!");
        }

        lval *Result = LvalPop(Self, 0);

        while(Self->CellCount)
        {
                Result = LvalJoin(Result, LvalPop(Self, 0));
        }

        LvalFree(Self);
        return(Result);
}

lval *
LvalBuiltIn(lval *Self, char *Function)
{
        if(GSStringIsEqual(Function, "list", 4)) return(LvalBuiltInList(Self));
        if(GSStringIsEqual(Function, "head", 4)) return(LvalBuiltInHead(Self));
        if(GSStringIsEqual(Function, "tail", 4)) return(LvalBuiltInTail(Self));
        if(GSStringIsEqual(Function, "join", 4)) return(LvalBuiltInJoin(Self));
        if(GSStringIsEqual(Function, "eval", 4)) return(LvalBuiltInEval(Self));
        if(strstr("+-/*", Function)) return(LvalBuiltInOperator(Self, Function));

        LvalFree(Self);

        lval *Result = LvalError("Unknown Function!");
        return(Result);
}

lval *
LvalEvalSExpression(lval *Self)
{
        lval *Result = GSNullPtr;

        /* Evaluate all children. */
        for(int Cell = 0; Cell < Self->CellCount; Cell++)
        {
                Self->Cell[Cell] = LvalEval(Self->Cell[Cell]);
        }

        /* Check for errors. */
        for(int Cell = 0; Cell < Self->CellCount; Cell++)
        {
                if(Self->Cell[Cell]->Type == LVAL_TYPE_ERROR)
                {
                        Result = LvalTake(Self, Cell);
                        return(Result);
                }
        }

        /* Empty S-Expression. */
        if(Self->CellCount == 0) return(Self);

        /* Single element in S-Expression; ie., Single Expression. */
        if(Self->CellCount == 1)
        {
                Result = LvalTake(Self, 0);
                return(Result);
        }

        /* Ensure first element is a symbol. */
        lval *FirstElement = LvalPop(Self, 0);
        if(FirstElement->Type != LVAL_TYPE_SYMBOL)
        {
                LvalFree(FirstElement);
                LvalFree(Self);
                Result = LvalError("S-Expression does not start with a symbol!");
                return(Result);
        }

        Result = LvalBuiltIn(Self, FirstElement->Symbol);
        LvalFree(FirstElement);
        return(Result);
}

lval *
LvalEval(lval *Self)
{
        lval *Result = GSNullPtr;

        if(Self->Type == LVAL_TYPE_SEXPRESSION)
        {
                Result = LvalEvalSExpression(Self);
        }
        else
        {
                Result = Self;
        }

        return(Result);
}

lval *
EvalOperator(lval *A, char *Operator, lval *B)
{
        lval *Result;

        if(A->Type == LVAL_TYPE_ERROR) { return(A); }
        if(B->Type == LVAL_TYPE_ERROR) { return(B); }

        if(GSStringIsEqual(Operator, "+", 1)) { return(LvalNumber(A->Number + B->Number)); }
        if(GSStringIsEqual(Operator, "-", 1)) { return(LvalNumber(A->Number - B->Number)); }
        if(GSStringIsEqual(Operator, "*", 1)) { return(LvalNumber(A->Number * B->Number)); }
        if(GSStringIsEqual(Operator, "/", 1))
        {
                if(B->Number == 0)
                {
                        Result = LvalError(LVAL_ERROR_DIV_ZERO);
                        return(Result);
                }
                else
                {
                        Result = LvalNumber(A->Number / B->Number);
                        return(Result);
                }
        }

        Result = LvalError("Unknown Operator");
        return(Result);
}

lval *
Eval(mpc_ast_t *Tree)
{
        lval *Result;

        if(strstr(Tree->tag, "number"))
        {
                errno = 0;
                long Number = strtol(Tree->contents, NULL, 10);
                if(errno == ERANGE)
                {
                        Result = LvalError("Number too large");
                }
                else
                {
                        Result = LvalNumber(Number);
                }

                return(Result);
        }

        char *Operator = Tree->children[1]->contents;
        lval *Parameter = Eval(Tree->children[2]);

        int i = 3;
        while(strstr(Tree->children[i]->tag, "expr"))
        {
                Parameter = EvalOperator(Parameter, Operator, Eval(Tree->children[i]));
                i++;
        }

        return(Parameter);
}

void
Usage(char *ProgramName)
{
        printf("Usage: %s mpc_file\n\n", ProgramName);
        puts("Reads mpc_file and launches a repl to interactively test the generated parser.");
        exit(EXIT_SUCCESS);
}

int
main(int ArgCount, char *Arguments[])
{
        gs_args *Args = alloca(sizeof(gs_args));
        GSArgsInit(Args, ArgCount, Arguments);
        if(GSArgsHelpWanted(Args) || ArgCount < 2) Usage(GSArgsProgramName(Args));

        char *GrammarFile = GSArgsAtIndex(Args, 1);

        mpc_parser_t *Number = mpc_new("number");
        mpc_parser_t *Symbol = mpc_new("symbol");
        mpc_parser_t *Sexpr  = mpc_new("sexpr");
        mpc_parser_t *Qexpr  = mpc_new("qexpr");
        mpc_parser_t *Expr   = mpc_new("expr");
        mpc_parser_t *Lispy  = mpc_new("lispy");

        size_t FileSize = GSFileSize(GrammarFile);
        gs_buffer *FileBuffer = alloca(sizeof(gs_buffer));
        GSBufferInit(FileBuffer, malloc(FileSize), FileSize);
        GSFileCopyToBuffer(GrammarFile, FileBuffer);

        mpca_lang(MPCA_LANG_DEFAULT,
                  FileBuffer->Start,
                  Number, Symbol, Sexpr, Qexpr, Expr, Lispy);

        puts("Lispy Version 0.0.1");
        puts("Press Ctrl+c to exit\n");

        mpc_result_t *MpcResult = alloca(sizeof(mpc_result_t));

        while(true)
        {
                char *Input = readline("lispy> ");
                add_history(Input);
                if(mpc_parse("<stdin>", Input, Lispy, MpcResult))
                {
                        lval *Result = LvalRead(MpcResult->output);
                        Result = LvalEval(Result);
                        LvalPrintLine(Result);
                        LvalFree(Result);
                        mpc_ast_delete(MpcResult->output);
                }
                else
                {
                        mpc_err_print(MpcResult->error);
                        mpc_err_delete(MpcResult->error);
                }
                free(Input);
        }

        mpc_cleanup(4, Number, Symbol, Sexpr, Qexpr, Expr, Lispy);

        return(0);
}
