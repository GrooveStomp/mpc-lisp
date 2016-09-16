#include <stdio.h>
#include <stdlib.h>

#include <editline/readline.h>
#include <editline/history.h>

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

int
main(int argc, char *argv[])
{
        mpc_parser_t *Number   = mpc_new("number");
        mpc_parser_t *Operator = mpc_new("operator");
        mpc_parser_t *Expr     = mpc_new("expr");
        mpc_parser_t *Lispy    = mpc_new("lispy");

        mpca_lang(MPCA_LANG_DEFAULT,
                  "                                                   \
                   number   : /-?[0-9]+/ ;                            \
                   operator : '+' | '-' | '*' | '/' ;                 \
                   expr     : <number> | '(' <operator> <expr>+ ')' ; \
                   lispy    : /^/ <operator> <expr>+ /$/ ;            \
                  ",
                  Number, Operator, Expr, Lispy);


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
