%{
#include <glib.h>
#ifdef HAVE_STDIO_H
#  include <stdio.h>
#endif

%}

%union ParseToken {
    float real;
    int integer;
    char *string;
    char *identifier;
    gboolean bool;
    char character;
}

%{
#include "parse.h"

extern int yylex();
extern int yyparse();
void yyerror(char *err);

extern int yylineno;
extern FILE *yyin;

static char *path;
static union ParseToken t;

/* in parse.c */
void parse_token(ParseTokenType type, union ParseToken token);
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

%%

sections:
  | sections '[' IDENTIFIER ']' { parse_set_section($3); } '\n' lines
  ;

lines:
  | lines tokens '\n' { t.character = $3; parse_token($3, t); }
  ;

tokens:
    tokens token
  | token
  ;

token:
    REAL       { t.real = $1; parse_token(REAL, t); }
  | INTEGER    { t.integer = $1; parse_token(INTEGER, t); }
  | STRING     { t.string = $1; parse_token(STRING, t); }
  | IDENTIFIER { t.identifier = $1; parse_token(IDENTIFIER, t); }
  | BOOL       { t.bool = $1; parse_token(BOOL, t); }
  | '('        { t.character = $1; parse_token($1, t); }
  | ')'        { t.character = $1; parse_token($1, t); }
  | '{'        { t.character = $1; parse_token($1, t); }
  | '}'        { t.character = $1; parse_token($1, t); }
  | '='        { t.character = $1; parse_token($1, t); }
  | ','        { t.character = $1; parse_token($1, t); }
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
