%{
#include <glib.h>
#include "y.tab.h"

extern void kparseerror(char *s);
int kparselineno;
%}

field [A-Za-z0-9][-A-Za-z0-9]*
number [0-9]+
desktop [dD][eE][sS][kK][tT][oO][pP]
execute [eE][xX][eE][cC][uU][tT][eE]
restart [rR][eE][sS][tT][aA][rR][tT]
string \"[^"\n]*\"

%%

^[ \t]*#.*\n kparselineno++;
{number}     kparselval.integer = atoi(kparsetext);    return INTEGER;
{execute}    kparselval.string = g_strdup(kparsetext); return EXECUTE;
{restart}    kparselval.string = g_strdup(kparsetext); return RESTART;
{desktop}    kparselval.string = g_strdup(kparsetext); return DESKTOP;
{field}      kparselval.string = g_strdup(kparsetext); return FIELD;
{string}     {
                 /* drop the quotes */
                 kparselval.string = g_strdup(kparsetext+1);
                 if (kparselval.string[kparseleng-2] != '"') {
                     g_warning("improperly terminated string on line %d\n",
                               kparselineno);
                     kparseerror("");
                 } else
                     kparselval.string[kparseleng-2] = '\0';
                 return STRING;
             }

[ \t]
\n           kparselineno++; return *yytext;
.            kparseerror("");

%%

int kparsewrap() {
    return 1;
}
