%{
#include <stdio.h>
#include "yacc_parser.hh"
%}

%%

\{                  return OBRACE;
\}                  return EBRACE;
;                   return SEMICOLON;
-                   return DASH;
noaction |
execute |
iconify |
raise |
lower |
close |
toggleshade |
toggleomnipresent |
moveWindowUp |
moveWindowDown |
moveWindowLeft |
moveWindowRight |
resizeWindowWidth |
resizeWindowHeight |
toggleMaximizeFull |
toggleMaximizeVertical |
toggleMaximizeHorizontal |
sendToWorkspace |
nextWindow |
prevWindow |
nextWindowOnAllWorkspaces |
prevWindowOnAllWorkspaces |
nextWindowOnAllScreens |
prevWindowOnAllScreens |
nextWindowOfClass |
prevWindowOfClass |
nextWindowOfClassOnAllWorkspaces |
prevWindowOfClassOnAllWorkspaces |
changeWorkspace |
nextWorkspace |
prevWorkspace |
nextScreen |
prevScreen |
showRootMenu |
showWorkspaceMenu |
stringChain |
keyChain |
numberChain |
cancel              yylval = strdup(yytext); return ACTION;
Mod1 |
Mod2 |
Mod3 |
Mod4 |
Control |
Shift               yylval = strdup(yytext); return BINDING;
[0-9]+              yylval = strdup(yytext); return NUMBER;
\".+\"              yylval = strdup(yytext); return QUOTES;
[a-zA-Z_0-9]+       yylval = strdup(yytext); return WORD;
#.+\n               /* ignore */
\n                  /* ignore */
[ \t]+              /* */
%%

