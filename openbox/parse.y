%{
#include <glib.h>
#ifdef HAVE_STDIO_H
#  include <stdio.h>
#endif

%}

%union ParseTokenData {
    float real;
    int integer;
    char *string;
    char *identifier;
    gboolean bool;
    char character;
    GList *list;
}

%{
#define NO_TAB_H
#include "parse.h"
#undef NO_TAB_H

extern int yylex();
extern int yyparse();
void yyerror(char *err);

extern int lineno;
extern FILE *yyin;

static char *path;
static ParseToken t;

/* in parse.c */
void parse_token(ParseToken *token);
void parse_assign(char *name, ParseToken *token);
void parse_set_section(char *section);
%}

%token <real> REAL
%token <integer> INTEGER
%token <string> STRING
%token <identifier> IDENTIFIER
%token <bool> BOOL
%token <character> '('
%token <character> ')'
%token <character> '{'
%token <character> '}'
%token <character> '='
%token <character> ','
%token <character> '\n'
%token INVALID

%type <list> list
%type <list> listtokens

%%

sections:
  | sections '[' IDENTIFIER ']' { parse_set_section($3); } '\n'
    { ++lineno; } lines
  ;

lines:
  | lines tokens { t.type='\n'; t.data.character='\n'; parse_token(&t); } '\n'
    { ++lineno; }
  | lines IDENTIFIER '=' listtoken { parse_assign($2, &t); } '\n'
    { ++lineno; }
  ;

tokens:
    tokens token { parse_token(&t); }
  | token        { parse_token(&t); }
  ;

token:
    REAL       { t.type = TOKEN_REAL; t.data.real = $1; }
  | INTEGER    { t.type = TOKEN_INTEGER; t.data.integer = $1; }
  | STRING     { t.type = TOKEN_STRING; t.data.string = $1; }
  | IDENTIFIER { t.type = TOKEN_IDENTIFIER; t.data.identifier = $1; }
  | BOOL       { t.type = TOKEN_BOOL; t.data.bool = $1; }
  | list       { t.type = TOKEN_LIST; t.data.list = $1; }
  | '{'        { t.type = $1; t.data.character = $1; }
  | '}'        { t.type = $1; t.data.character = $1; }
  | ','        { t.type = $1; t.data.character = $1; }
  ;

list:
    '(' listtokens ')' { $$ = $2; }
  ;

listtokens:
    listtokens listtoken { ParseToken *nt = g_new(ParseToken, 1);
                           nt->type = t.type;
                           nt->data = t.data;
                           $$ = g_list_append($1, nt);
                         }
  | listtoken            { ParseToken *nt = g_new(ParseToken, 1);
                           nt->type = t.type;
                           nt->data = t.data;
                           $$ = g_list_append(NULL, nt);
                         }
  ;

listtoken:
    REAL       { t.type = TOKEN_REAL; t.data.real = $1; }
  | INTEGER    { t.type = TOKEN_INTEGER; t.data.integer = $1; }
  | STRING     { t.type = TOKEN_STRING; t.data.string = $1; }
  | IDENTIFIER { t.type = TOKEN_IDENTIFIER; t.data.identifier = $1; }
  | BOOL       { t.type = TOKEN_BOOL; t.data.bool = $1; }
  | list       { t.type = TOKEN_LIST; t.data.list = $1; }
  | '{'        { t.type = $1; t.data.character = $1; }
  | '}'        { t.type = $1; t.data.character = $1; }
  | ','        { t.type = $1; t.data.character = $1; }
  ;


%%

int lineno;

void yyerror(char *err) {
    g_message("%s:%d: %s", path, lineno, err);
}

void parse_rc()
{
    /* try the user's rc */
    path = g_build_filename(g_get_home_dir(), ".openbox", "rc3", NULL);
    if ((yyin = fopen(path, "r")) == NULL) {
        g_free(path);
        /* try the system wide rc */
        path = g_build_filename(RCDIR, "rc3", NULL);
        if ((yyin = fopen(path, "r")) == NULL) {
            g_warning("No rc2 file found!");
            g_free(path);
            return;
        }
    }

    lineno = 1;

    yyparse();

    g_free(path);
}
