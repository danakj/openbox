%{
#include <glib.h>
#include "config.h"

static char *yyfilename;
static int yylineno = 1;
static gboolean haserror = FALSE;
static gboolean comment = FALSE;
static ConfigEntry entry = { NULL, -1 };

static void stringvalue();
static void numbervalue();
static void boolvalue();
static void identifier();
static void newline();
static int yywrap();
%}

number [0-9]+
string \"[^"\n]*\"
identifier [a-zA-Z][a-zA-Z0-9_.]*
white [ \t]*
assign {white}={white}
bool ([tT][rR][uU][eE]|[fF][aA][lL][sS][eE]|[yY][eE][sS]|[nN][oO]|[oO][nN]|[oO][fF][fF])

%%

^# comment = TRUE;
{bool}/{white}\n boolvalue();
{string}/{white}\n stringvalue();
{number}/{white}\n numbervalue();
^{identifier}/{assign} identifier();
\n newline();
=
[ \t]
. if (!comment) haserror = TRUE;

%%

static void stringvalue()
{
    if (!comment) {
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
}

static void numbervalue()
{
    if (!comment) {
        if (!haserror && entry.name != NULL && (signed)entry.type < 0) {
            entry.type = Config_Integer;
            entry.value.integer = atoi(yytext);
        } else
            haserror = TRUE;
    }
}

static void boolvalue()
{
    if (!comment) {
        if (!haserror && entry.name != NULL && (signed)entry.type < 0) {
            entry.type = Config_Bool;
            entry.value.bool = (!g_ascii_strcasecmp("true", yytext) ||
                                !g_ascii_strcasecmp("yes", yytext) ||
                                !g_ascii_strcasecmp("on", yytext));
        } else
            haserror = TRUE;
    }
}

static void identifier()
{
    if (!comment) {
        entry.name = g_strdup(yytext);
        entry.type = -1;
    }
}

static void newline()
{
    if (!comment) {
        if (!haserror && entry.name != NULL && (signed)entry.type >= 0) {
            if (!config_set(entry.name, entry.type, entry.value))
                g_warning("Parser error in '%s' on line %d\n", yyfilename,
                          yylineno);
        } else {
            g_warning("Parser error in '%s' on line %d", yyfilename, yylineno);
        }
        g_free(entry.name);
        entry.name = NULL;
        if (entry.type == Config_String)
            g_free(entry.value.string);
        entry.type = -1;

        haserror = FALSE;
    }
    comment = FALSE;
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

void cparse_go(char *filename, FILE *file)
{
    yyfilename = filename;
    yyin = file;
    yylex();
}
