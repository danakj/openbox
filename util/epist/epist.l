%{
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
Mod1 |
Mod2 |
Mod3 |
Mod4 |
Control |
Shift               yylval = (int) strdup(yytext); return BINDING;
[0-9]+              yylval = (int) strdup(yytext); return NUMBER;
\".+\"              yylval = (int) strdup(yytext); return QUOTES;
[a-zA-Z_0-9]+       yylval = (int) strdup(yytext); return WORD;
#.+\n               /* ignore */
\n                  /* ignore */
[ \t]+              /* */
%%

