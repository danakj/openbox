#include "config.h"

#ifdef HAVE_STDIO_H
#  include <stdio.h>
#endif

static GSList *config = NULL;

/* provided by cparse.l */
void cparse_go(FILE *);


void config_startup()
{
}

void config_shutdown()
{
}

void config_parse()
{
    FILE *file;
    char *path;

    path = g_build_filename(g_get_home_dir(), ".openbox", "rc3", NULL);
    if ((file = fopen(path, "r")) != NULL) {
        cparse_go(file);
        fclose(file);
    }
}

gboolean config_set(char *name, ConfigValueType type, ConfigValue value)
{
    g_message("Setting %s\n", name);
    return TRUE;
}
