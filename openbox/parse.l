%{
#include <glib.h>
#include "y.tab.h"
#ifdef HAVE_STDLIB_H
#  include <stdlib.h>
#endif

extern void yyerror(char *err);

int yylineno = 1;
%}

real [-0-9][0-9]*\.[0-9]+
integer [-0-9][0-9]*
string \"[^"\n]*\"
identifier [a-zA-Z][.a-zA-Z0-9]*
bool ([tT][rR][uU][eE]|[fF][aA][lL][sS][eE]|[yY][eE][sS]|[nN][oO]|[oO][nN]|[oO][fF][fF])

%%

^[ \t]*#.*\n /* comment */ { ++yylineno; }
^[ \t]*#.*   /* comment */
^[ \t]*\n    /* empty lines */ { ++yylineno; }
[ \t]        /* whitespace */
{real}       { yylval.real = atof(yytext); return REAL; }
{integer}    { yylval.integer = atoi(yytext); return INTEGER; }
{string}     { yylval.string = g_strdup(yytext+1); /* drop the left quote */
               if (yylval.string[yyleng-2] != '"')
                   yyerror("improperly terminated string on line %d");
               else
                   yylval.string[yyleng-2] = '\0';
               return STRING;
             }
{bool}       { yylval.bool = (!g_ascii_strcasecmp("true", yytext) ||
                              !g_ascii_strcasecmp("yes", yytext) ||
                              !g_ascii_strcasecmp("on", yytext));
               return BOOL;
             }
{identifier} { yylval.identifier = g_strdup(yytext); return IDENTIFIER; }
[{}()\[\]=,] { yylval.character = *yytext; return *yytext; }
\n           { yylval.character = *yytext; ++yylineno; return *yytext; }
.            { return INVALID; }

%%

int yywrap() {
    return 1;
}
