%{
#include <glib.h>
#include "config.h"

static int yylineno = 1;
static gboolean haserror = FALSE;
static ConfigEntry entry = { NULL, -1 };

static void stringvalue();
static void numbervalue();
static void identifier();
static void newline();
static int yywrap();
%}

number [0-9]+
string \"[^"\n]*\"
identifier [a-zA-Z][a-zA-Z0-9_]*
white [ \t]*
assign {white}={white}

%%

{string}/{white}\n stringvalue();
{number}/{white}\n numbervalue();
^{identifier}/{assign} identifier();
\n newline();
=
[ \t]
. haserror = TRUE;

%%

static void stringvalue()
{
    if (!haserror && entry.name != NULL && (signed)entry.type < 0) {
        entry.type = Config_String;
        entry.value.string = g_strdup(yytext+1); /* drop the left quote */
        if (entry.value.string[yyleng-2] != '"')
            printf("warning: improperly terminated string on line %d\n",
                   yylineno);
        else
            entry.value.string[yyleng-2] = '\0';
    } else
        haserror = TRUE;
}

static void numbervalue()
{
    if (!haserror && entry.name != NULL && (signed)entry.type < 0) {
        entry.type = Config_Integer;
        entry.value.integer = atoi(yytext);
    } else
        haserror = TRUE;
}

static void identifier()
{
    entry.name = g_strdup(yytext);
    entry.type = -1;
}

static void newline()
{
    if (!haserror && entry.name != NULL && (signed)entry.type >= 0) {
        if (!config_set(entry.name, entry.type, entry.value))
            g_warning("Invalid option in config file: '%s'\n", entry.name);
    } else {
        printf("Parser error in config file on line %d\n", yylineno);
    }
    g_free(entry.name);
    entry.name = NULL;
    if (entry.type == Config_String)
        g_free(entry.value.string);
    entry.type = -1;

    haserror = FALSE;
    ++yylineno;
}

static int yywrap()
{
    g_free(entry.name);
    entry.name = NULL;
    if (entry.type == Config_String)
        g_free(entry.value.string);
    return 1;
}

void cparse_go(FILE *file)
{
    yyin = file;
    yylex();
}
