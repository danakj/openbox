%{
#include <stdio.h>
#include <string.h>
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
cancel              yylval = (int) strdup(yytext); return ACTION;
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

