%{
#define NO_Y_H
#include "parse.h"
#undef NO_Y_H

#ifdef HAVE_STDIO_H
#  include <stdio.h>
#endif

extern int yylex();

extern int yylineno;
extern FILE *yyin;

static char *path;
static union ParseToken t;

/* in parse.c */
void parse_token(ParseTokenType type, union ParseToken token);
void parse_set_section(char *section);
%}

%union ParseToken {
    float real;
    int integer;
    char *string;
    char *identifier;
    gboolean bool;
    char character;
}

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
  | lines tokens '\n' { t.character = $3; parse_token(TOKEN_NEWLINE, t); }
  ;

tokens:
    tokens token
  | token
  ;

token:
    REAL       { t.real = $1; parse_token(TOKEN_REAL, t); }
  | INTEGER    { t.integer = $1; parse_token(TOKEN_INTEGER, t); }
  | STRING     { t.string = $1; parse_token(TOKEN_STRING, t); }
  | IDENTIFIER { t.identifier = $1; parse_token(TOKEN_IDENTIFIER, t); }
  | BOOL       { t.bool = $1; parse_token(TOKEN_BOOL, t); }
  | '('        { t.character = $1; parse_token(TOKEN_LBRACKET, t); }
  | ')'        { t.character = $1; parse_token(TOKEN_RBRACKET, t); }
  | '{'        { t.character = $1; parse_token(TOKEN_LBRACE, t); }
  | '}'        { t.character = $1; parse_token(TOKEN_RBRACE, t); }
  | '='        { t.character = $1; parse_token(TOKEN_EQUALS, t); }
  | ','        { t.character = $1; parse_token(TOKEN_COMMA, t); }
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
