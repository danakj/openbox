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
#include "parse.h"

extern int yylex();
extern int yyparse();
void yyerror(char *err);

extern int yylineno;
extern FILE *yyin;

static char *path;
static ParseToken t;

/* in parse.c */
void parse_token(ParseToken *token);
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
  | sections '[' IDENTIFIER ']' { parse_set_section($3); } '\n' lines
  ;

lines:
  | lines tokens '\n' { t.type = $3; t.data.character = $3; parse_token(&t); }
  ;

tokens:
    tokens token
  | token
  ;

token:
    REAL       { t.type = TOKEN_REAL; t.data.real = $1; parse_token(&t); }
  | INTEGER    { t.type = TOKEN_INTEGER; t.data.integer = $1;
                 parse_token(&t); }
  | STRING     { t.type = TOKEN_STRING; t.data.string = $1; parse_token(&t); }
  | IDENTIFIER { t.type = TOKEN_IDENTIFIER; t.data.identifier = $1;
                 parse_token(&t);}
  | BOOL       { t.type = TOKEN_BOOL; t.data.bool = $1; parse_token(&t); }
  | list       { t.type = TOKEN_LIST; t.data.list = $1; parse_token(&t); }
  | '{'        { t.type = $1; t.data.character = $1; parse_token(&t); }
  | '}'        { t.type = $1; t.data.character = $1; parse_token(&t); }
  | '='        { t.type = $1; t.data.character = $1; parse_token(&t); }
  | ','        { t.type = $1; t.data.character = $1; parse_token(&t); }
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
  | '='        { t.type = $1; t.data.character = $1; }
  | ','        { t.type = $1; t.data.character = $1; }
  ;


%%

void yyerror(char *err) {
    g_message("%s:%d: %s", path, yylineno, err);
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

    yylineno = 1;

    yyparse();

    g_free(path);
}
