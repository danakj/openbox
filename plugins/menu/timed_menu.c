#include <glib.h>

#include "kernel/menu.h"
#include "kernel/timer.h"
#include "timed_menu.h"
#include "kernel/action.h"

#define TIMED_MENU(m) ((Menu *)m)
#define TIMED_MENU_DATA(m) ((Timed_Menu_Data *)((Menu *)m)->plugin_data)
static char *PLUGIN_NAME = "timed_menu";

typedef enum {
    TIMED_MENU_PIPE
} Timed_Menu_Type;

/* we can use various GIO channels to support reading menus (can we add them to
   the event loop? )
   stat() based update
   exec() based read
*/
typedef struct {
    Timed_Menu_Type type;
    Timer *timer; /* timer code handles free */
    char *command; /* for the PIPE */
} Timed_Menu_Data;


void plugin_setup_config() { }
void plugin_startup()
{ }
void plugin_shutdown() { }

void timed_menu_timeout_handler(void *data)
{
    Action *a;
    ((Menu *)data)->invalid = TRUE;

    if (!TIMED_MENU(data)->shown) {
        switch (TIMED_MENU_DATA(data)->type) {
            case (TIMED_MENU_PIPE):
            {
                /* if the menu is not shown, run a process and use its output
                   as menu */
                char *args[] = {"/bin/sh", "-c", "ls", NULL};
                GIOChannel *io;
                char *line;
                gint child, c_stdout, line_len, terminator_pos;
                GIOStatus status;
                /* this blocks for now, until the event stuff can handle it */
                if (!g_spawn_async_with_pipes(NULL,
                        args,
                        NULL,
                        G_SPAWN_DO_NOT_REAP_CHILD,
                        NULL, NULL,
                        &child, NULL, &c_stdout, NULL,
                        NULL)) {
                    g_warning("%s: Unable to run timed_menu program",
                        __FUNCTION__);
                    break;
                }
                
                io = g_io_channel_unix_new(c_stdout);
                if (io == NULL) {
                    g_error("%s: Unable to get IO channel", __FUNCTION__);
                    break;
                }

                menu_clear(TIMED_MENU(data));
                
                while ( G_IO_STATUS_NORMAL == (status =
                            g_io_channel_read_line
                            (io, &line, &line_len, &terminator_pos, NULL))
                    ) {
                    /* the \n looks ugly */
                    line[terminator_pos] = '\0';
                    menu_add_entry(TIMED_MENU(data),
                        menu_entry_new_separator(line));
                    g_message("%s", line);
                    g_free(line);
                }
                break;
            }
        }
    }
}

void *plugin_create()
{
    Timed_Menu_Data *d = g_new(Timed_Menu_Data, 1);
    Menu *m = menu_new("", PLUGIN_NAME, NULL);
    
    m->plugin = PLUGIN_NAME;

    d->type = TIMED_MENU_PIPE;
    d->timer = timer_start(1000000, &timed_menu_timeout_handler, m);
    
    m->plugin_data = (void *)d;
  
    return (void *)m;
}

void plugin_destroy (void *m)
{
    /* this will be freed by timer_* */
    timer_stop( ((Timed_Menu_Data *)TIMED_MENU(m)->plugin_data)->timer);
    
    g_free( TIMED_MENU(m)->plugin_data );
}
