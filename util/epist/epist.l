%{
#ifdef    HAVE_CONFIG_H
#  include "../../config.h"
#endif // HAVE_CONFIG_H

#include <stdio.h>
#include <string.h>
#include "yacc_parser.hh"

extern YYSTYPE yylval;
    
%}

%%

\{                  return OBRACE;
\}                  return EBRACE;
;                   return SEMICOLON;
-                   return DASH;
Options |
options             return OPTIONS;
Mod1 |
mod1 |
Mod2 |
mod2 |
Mod3 |
mod3 |
Mod4 |
mod4 |
Control |
control |
shift |
Shift               yylval = (int) strdup(yytext); return BINDING;
on |
On |
true |
True                yylval = (int) strdup(yytext); return TRUE;
Off |
off |
false |
False               yylval = (int) strdup(yytext); return FALSE;
[0-9]+              yylval = (int) strdup(yytext); return NUMBER;
\".+\"              yylval = (int) strdup(yytext); return QUOTES;
[a-zA-Z_0-9]+       yylval = (int) strdup(yytext); return WORD;
#.+\n               /* ignore */
\n                  /* ignore */
[ \t]+              /* */
%%

