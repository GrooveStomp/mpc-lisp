#include <stdio.h>
#include <stdlib.h>
#include <alloca.h>

#include <editline/readline.h>
#include <editline/history.h>

#include "gs.h"
#include "mpc.h"

long
EvalOperator(long A, char *Operator, long B)
{
        if(strcmp(Operator, "+") == 0) { return A + B; }
        if(strcmp(Operator, "-") == 0) { return A - B; }
        if(strcmp(Operator, "*") == 0) { return A * B; }
        if(strcmp(Operator, "/") == 0) { return A / B; }
        return(0);
}

long
Eval(mpc_ast_t *Tree)
{
        if(strstr(Tree->tag, "number"))
        {
                return(atoi(Tree->contents));
        }

        char *Operator = Tree->children[1]->contents;

        long Parameter = Eval(Tree->children[2]);

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

        while(1)
        {
                char *input = readline("lispy> ");
                add_history(input);
                if(mpc_parse("<stdin>", input, Lispy, &r))
                {
                        long Result = Eval(r.output);
                        printf("%li\n", Result);
                        mpc_ast_delete(r.output);
                }
                else
                {
                        mpc_err_print(r.error);
                        mpc_err_delete(r.error);
                }
                printf("%s\n", input);
                free(input);
        }

        mpc_cleanup(4, Number, Operator, Expr, Lispy);

        return(0);
}
