%{
#include "obcl.h"

int yylex(void);
void yyerror(char *msg, ...);

extern int yylineno;
extern char *yytext;
GList *config; /* this is what we parse into */

%}

%union {
    double num;
    gchar *string;
    CLNode *node;
    GList *glist;
};

%token <num> TOK_NUM
%token <string> TOK_ID TOK_STRING
%token TOK_SEP

%type <glist> config
%type <glist> stmts
%type <node> stmt
%type <glist> list
%type <glist> block
%type <node> value

%expect 2 /* for now */

%%

config: stmts
    {
        config = $$ = $1;
    }
    ;

stmts:
    { $$ = NULL; }
    | stmt
    { $$ = g_list_append(NULL, $1); }
    | stmts stmt
    { $$ = g_list_append($1, $2); }
    ;

stmt: TOK_ID list ';'
    {
        CLNode *s = g_new(CLNode,1);
        s->type = CL_LIST;
        s->u.lb.list = $2;
        s->u.lb.block = NULL;
        s->u.lb.id = $1;
        $$ = s;
    }
    | TOK_ID list block
    {
        CLNode *s = g_new(CLNode,1);
        s->type = CL_LISTBLOCK;
        s->u.lb.list = $2;
        s->u.lb.block = $3;
        s->u.lb.id = $1;
        $$ = s;
    }
    | TOK_ID block
    { 
        CLNode *s = g_new(CLNode,1);
        s->type = CL_BLOCK;
        s->u.lb.block = $2;
        s->u.lb.list = NULL;
        s->u.lb.id = $1;
        $$ = s;
    }
    ;

list: value
    {
        $$ = g_list_append(NULL, $1);
    }
    | list ',' value
    {
        $$ = g_list_append($1, $3);
    }
    ;

block: '{' stmts '}'
    {
        $$ = $2;
    }
    ;

value: TOK_ID
    {
        CLNode *node = g_new(CLNode,1);
        node->type = CL_ID;
        node->u.str = $1;
        $$ = node;
    }
    | TOK_STRING
    { 
        CLNode *node = g_new(CLNode,1);
        node->type = CL_STR;
        node->u.str = $1;
        $$ = node;
    }
    | TOK_NUM
    {
        CLNode *node = g_new(CLNode,1);
        node->type = CL_NUM;
        node->u.num = $1;
        $$ = node;
    }
    ;

%%

int yywrap()
{
    return 1;
}

/* void yyerror(const char *err) */
/* { */
/*     fprintf(stderr, "Parse error on line %d, near '%s': %s\n", */
/*             yylineno, yytext, err); */
/* } */

void yyerror(char *msg, ...)
{
    va_list args;
    va_start(args,msg);

    fprintf(stderr, "Error on line %d, near '%s': ", yylineno, yytext);
    vfprintf(stderr, msg, args);
    fprintf(stderr,"\n");

    va_end(args);
}

GList *cl_parse_fh(FILE *fh)
{
    extern FILE *yyin;
    yyin = fh;
    yyparse();
    return config;
}
