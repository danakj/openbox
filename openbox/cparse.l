%{
#include <glib.h>
#include "config.h"

static char *filename;
static int lineno = 1;
static gboolean haserror = FALSE;
static gboolean comment = FALSE;
static ConfigEntry entry = { NULL, -1 };

static void stringvalue();
static void numbervalue();
static void boolvalue();
static void identifier();
static void newline();
static int cparsewrap();
%}

number [0-9]+
string \"[^"\n]*\"
identifier [a-zA-Z][a-zA-Z0-9_.]*
white [ \t]*
assign {white}={white}
bool ([tT][rR][uU][eE]|[fF][aA][lL][sS][eE]|[yY][eE][sS]|[nN][oO]|[oO][nN]|[oO][fF][fF])

%%

^{white}# comment = TRUE;
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
            entry.value.string = g_strdup(cparsetext+1); /* drop the left quote */
            if (entry.value.string[cparseleng-2] != '"')
                printf("warning: improperly terminated string on line %d\n",
                       lineno);
            else
                entry.value.string[cparseleng-2] = '\0';
        } else
            haserror = TRUE;
    }
}

static void numbervalue()
{
    if (!comment) {
        if (!haserror && entry.name != NULL && (signed)entry.type < 0) {
            entry.type = Config_Integer;
            entry.value.integer = atoi(cparsetext);
        } else
            haserror = TRUE;
    }
}

static void boolvalue()
{
    if (!comment) {
        if (!haserror && entry.name != NULL && (signed)entry.type < 0) {
            entry.type = Config_Bool;
            entry.value.bool = (!g_ascii_strcasecmp("true", cparsetext) ||
                                !g_ascii_strcasecmp("yes", cparsetext) ||
                                !g_ascii_strcasecmp("on", cparsetext));
        } else
            haserror = TRUE;
    }
}

static void identifier()
{
    if (!comment) {
        entry.name = g_strdup(cparsetext);
        entry.type = -1;
    }
}

static void newline()
{
    if (!comment) {
        if (!haserror && entry.name != NULL && (signed)entry.type >= 0) {
            if (!config_set(entry.name, entry.type, entry.value))
                g_warning("Parser error in '%s' on line %d\n", filename,
                          lineno);
        } else if (haserror || entry.name != NULL || (signed)entry.type >= 0) {
            g_warning("Parser error in '%s' on line %d", filename, lineno);
        }
        g_free(entry.name);
        entry.name = NULL;
        if (entry.type == Config_String)
            g_free(entry.value.string);
        entry.type = -1;

        haserror = FALSE;
    }
    comment = FALSE;
    ++lineno;
}

static int cparsewrap()
{
    g_free(entry.name);
    entry.name = NULL;
    if (entry.type == Config_String)
        g_free(entry.value.string);
    return 1;
}

void cparse_go(char *fname, FILE *file)
{
    filename = fname;
    cparsein = file;
    cparselex();
}
