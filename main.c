#include <stdio.h>
#include <stdlib.h>
#include <alloca.h>

#include <editline/readline.h>
#include <editline/history.h>

#include "gs.h"
#include "mpc.h"

typedef struct
{
        int Type;
        int Number;
        int Error;
} lval;

enum lval_type_e
{
        LVAL_TYPE_NUMBER,
        LVAL_TYPE_ERROR
};

enum lval_error_e
{
        LVAL_ERROR_DIV_ZERO,
        LVAL_ERROR_BAD_OPERATOR,
        LVAL_ERROR_BAD_NUMBER
};

lval
LvalNumber(long Number)
{
        lval Result;
        Result.Type = LVAL_TYPE_NUMBER;
        Result.Number = Number;
        return(Result);
}

lval
LvalError(int Error)
{
        lval Result;
        Result.Type = LVAL_TYPE_ERROR;
        Result.Error = Error;
        return(Result);
}

void
LvalPrint(lval Value)
{
        switch(Value.Type)
        {
                case(LVAL_TYPE_NUMBER):
                {
                        printf("%i", Value.Number);
                } break;
                case(LVAL_TYPE_ERROR):
                {
                        switch(Value.Error)
                        {
                                case(LVAL_ERROR_DIV_ZERO):
                                {
                                        printf("Error: Division By Zero!");
                                } break;
                                case(LVAL_ERROR_BAD_OPERATOR):
                                {
                                        printf("Error: Invalid Operator!");
                                } break;
                                case(LVAL_ERROR_BAD_NUMBER):
                                {
                                        printf("Error: Invalid Number!");
                                } break;
                        }
                } break;
        }
}

void
LvalPrintln(lval Value)
{
        LvalPrint(Value); putchar('\n');
}

lval
EvalOperator(lval A, char *Operator, lval B)
{
        lval Result;

        if(A.Type == LVAL_TYPE_ERROR) { return(A); }
        if(B.Type == LVAL_TYPE_ERROR) { return(B); }

        if(strcmp(Operator, "+") == 0) { return(LvalNumber(A.Number + B.Number)); }
        if(strcmp(Operator, "-") == 0) { return(LvalNumber(A.Number - B.Number)); }
        if(strcmp(Operator, "*") == 0) { return(LvalNumber(A.Number * B.Number)); }
        if(strcmp(Operator, "/") == 0) {
                if(B.Number == 0)
                {
                        Result = LvalError(LVAL_ERROR_DIV_ZERO);
                        return(Result);
                }
                else
                {
                        Result = LvalNumber(A.Number / B.Number);
                        return(Result);
                }
        }

        Result = LvalError(LVAL_ERROR_BAD_OPERATOR);
        return(Result);
}

lval
Eval(mpc_ast_t *Tree)
{
        lval Result;

        if(strstr(Tree->tag, "number"))
        {
                errno = 0;
                long Number = strtol(Tree->contents, NULL, 10);
                if(errno == ERANGE)
                {
                        Result = LvalError(LVAL_ERROR_BAD_NUMBER);
                }
                else
                {
                        Result = LvalNumber(Number);
                }

                return(Result);
        }

        char *Operator = Tree->children[1]->contents;
        lval Parameter = Eval(Tree->children[2]);

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

        mpc_parser_t *Number   = mpc_new("number");
        mpc_parser_t *Operator = mpc_new("operator");
        mpc_parser_t *Expr     = mpc_new("expr");
        mpc_parser_t *Lispy    = mpc_new("lispy");

        size_t FileSize = GSFileSize(GrammarFile);
        gs_buffer *FileBuffer = alloca(sizeof(gs_buffer));
        GSBufferInit(FileBuffer, malloc(FileSize), FileSize);
        GSFileCopyToBuffer(GrammarFile, FileBuffer);

        mpca_lang(MPCA_LANG_DEFAULT, FileBuffer->Start, Number, Operator, Expr, Lispy);

        puts("Lispy Version 0.0.1");
        puts("Press Ctrl+c to exit\n");

        mpc_result_t r;

        while(true)
        {
                char *Input = readline("lispy> ");
                add_history(Input);
                if(mpc_parse("<stdin>", Input, Lispy, &r))
                {
                        lval Result = Eval(r.output);
                        LvalPrintln(Result);
                        mpc_ast_delete(r.output);
                }
                else
                {
                        mpc_err_print(r.error);
                        mpc_err_delete(r.error);
                }
                free(Input);
        }

        mpc_cleanup(4, Number, Operator, Expr, Lispy);

        return(0);
}
