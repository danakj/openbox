%{
#include "keyboard.h"
#include "../../kernel/action.h"
#include <glib.h>
#ifdef HAVE_STDIO_H
#  include <stdio.h>
#endif

extern int kparselex();
extern int kparselineno;
extern FILE *kparsein;  /* lexer input */

void kparseerror(char *s);
static void addbinding(GList *keylist, char *action, char *path, int num);

static char *path;
%}

%union {
    char *string;
    int integer;
    GList *list;
}

%token <integer> INTEGER
%token <string> STRING
%token <string> FIELD
%token <string> DESKTOP

%type <list> fields

%%

config:
  | config '\n'
  | config fields FIELD '\n' { addbinding($2, $3, NULL, 0); }
  | config fields FIELD INTEGER '\n' { addbinding($2, $3, NULL, $4); }
  | config fields FIELD STRING '\n' { addbinding($2, $3, $4, 0); }
  ;

fields:
  FIELD { $$ = g_list_append(NULL, $1); }
  | fields FIELD { $$ = g_list_append($1, $2); }
  ;

%%

void kparseerror(char *s) {
    g_warning("Parser error in '%s' on line %d", path, kparselineno);
}

void keysrc_parse()
{
    path = g_build_filename(g_get_home_dir(), ".openbox", "keysrc", NULL);
    if ((kparsein = fopen(path, "r")) == NULL) {
        g_free(path);
        path = g_build_filename(RCDIR, "keysrc", NULL);
        if ((kparsein = fopen(path, "r")) == NULL) {
            g_warning("No keysrc file found!");
            g_free(path);
            return;
        }
    }

    kparselineno = 1;

    kparseparse();
}

static void addbinding(GList *keylist, char *action, char *apath, int num)
{
    Action *a;

    a = action_from_string(action);

    /* no move/resize with the keyboard */
    if (a && (a->func == action_move || a->func == action_resize)) {
        action_free(a);
        a = NULL;
    }
    if (a == NULL) {
        g_warning("Invalid action '%s' in '%s' on line %d", action, apath,
                  kparselineno - 1);
        return;
    }
    /* these have extra data! */
    if (a->func == action_execute || a->func == action_restart)
        a->data.execute.path = apath;
    else
        g_free(apath);
    if (a->func == action_desktop)
        a->data.desktop.desk = (unsigned) num - 1;
    if (a->func == action_move_relative_horz ||
        a->func == action_move_relative_vert ||
        a->func == action_resize_relative_horz ||
        a->func == action_resize_relative_vert)
        a->data.relative.delta = num;

    if (!kbind(keylist, a)) {
        action_free(a);
        g_warning("Unable to add binding in '%s' on line %d", path,
                  kparselineno - 1);
    }
}
