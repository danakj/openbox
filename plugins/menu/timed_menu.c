#include <glib.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

#include "kernel/menu.h"
#include "kernel/timer.h"
#include "timed_menu.h"
#include "kernel/action.h"
#include "kernel/event.h"

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
    char *buf;
    unsigned long buflen;
    int fd;
} Timed_Menu_Data;


void plugin_setup_config() { }
void plugin_startup()
{ }
void plugin_shutdown() { }

void timed_menu_clean_up(Menu *m) {
    if (TIMED_MENU_DATA(m)->buf != NULL) {
        fprintf(stderr, "%s", TIMED_MENU_DATA(m)->buf);
        g_free(TIMED_MENU_DATA(m)->buf);
        TIMED_MENU_DATA(m)->buf = NULL;
    }

    TIMED_MENU_DATA(m)->buflen = 0;

    if (TIMED_MENU_DATA(m)->fd != -1) {
        event_remove_fd(TIMED_MENU_DATA(m)->fd);
        close(TIMED_MENU_DATA(m)->fd);
        TIMED_MENU_DATA(m)->fd = -1;
    }

    /* child is reaped by glib ? */
}

void timed_menu_read_pipe(int fd, Menu *menu)
{
    char *tmpbuf = NULL;
    unsigned long num_read;
#ifdef DEBUG
    Timed_Menu_Data *d = TIMED_MENU_DATA(menu);
#endif

    unsigned long num_realloc;
    /* if we have less than a quarter BUFSIZ left, allocate more */
    num_realloc = (BUFSIZ - (TIMED_MENU_DATA(menu)->buflen % BUFSIZ) <
                   BUFSIZ >> 2) ?
                  0 : BUFSIZ;
    
    tmpbuf = g_try_realloc(TIMED_MENU_DATA(menu)->buf,             
                           TIMED_MENU_DATA(menu)->buflen + num_realloc);

    if (tmpbuf == NULL) {
        g_warning("Unable to allocate memory for read()");
        return;
    }
    
    TIMED_MENU_DATA(menu)->buf = tmpbuf;
    
    num_read = read(fd,
                    TIMED_MENU_DATA(menu)->buf + TIMED_MENU_DATA(menu)->buflen,
                    num_realloc);
    if (num_read == 0) {
        unsigned long count = 0;
        char *found = NULL;
        menu->invalid = TRUE;
        menu_clear(menu);

        /* TEMP: list them */
        while (NULL !=
               (found = strchr(&TIMED_MENU_DATA(menu)->buf[count], '\n'))) {
            TIMED_MENU_DATA(menu)->buf
                [found - TIMED_MENU_DATA(menu)->buf] = '\0';
            menu_add_entry(menu,
                menu_entry_new_separator
                (&TIMED_MENU_DATA(menu)->buf[count]));
            count = found - TIMED_MENU_DATA(menu)->buf + 1;
        }
            

        TIMED_MENU_DATA(menu)->buf[TIMED_MENU_DATA(menu)->buflen] = '\0';
        timed_menu_clean_up(menu);
    } else if (num_read > 0) {
        TIMED_MENU_DATA(menu)->buflen += num_read;
        TIMED_MENU_DATA(menu)->buf[TIMED_MENU_DATA(menu)->buflen] = '\0';
    } else { /* num_read < 1 */
        g_warning("Error on read %s", strerror(errno));
        timed_menu_clean_up(menu);
    }
}

void timed_menu_timeout_handler(Menu *data)
{
    Action *a;

    if (!data->shown && TIMED_MENU_DATA(data)->fd == -1) {
        switch (TIMED_MENU_DATA(data)->type) {
            case (TIMED_MENU_PIPE):
            {
                /* if the menu is not shown, run a process and use its output
                   as menu */

                /* I hate you glib in all your hideous forms */
                char *args[] = {"/bin/sh", "-c", TIMED_MENU_DATA(data)->command,
                                NULL};
                int child_stdout;
                if (g_spawn_async_with_pipes(
                        NULL,
                        args,
                        NULL,
                        G_SPAWN_SEARCH_PATH | G_SPAWN_DO_NOT_REAP_CHILD,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        &child_stdout,
                        NULL,
                        NULL)) {
                    event_fd_handler *h = g_new(event_fd_handler, 1);
                    TIMED_MENU_DATA(data)->fd = h->fd = child_stdout;
                    h->handler = timed_menu_read_pipe;
                    h->data = data;
                    event_add_fd_handler(h);
                } else {
                    g_warning("unable to spawn child");
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
    d->timer = timer_start(60000000, &timed_menu_timeout_handler, m);
    d->command = "find /media -name *.m3u";
    d->buf = NULL;
    d->buflen = 0;
    d->fd = -1;
    
    m->plugin_data = (void *)d;

    timed_menu_timeout_handler(m);
    return (void *)m;
}

void plugin_destroy (void *m)
{
    /* this will be freed by timer_* */
    timer_stop( ((Timed_Menu_Data *)TIMED_MENU(m)->plugin_data)->timer);
    
    g_free( TIMED_MENU(m)->plugin_data );
}
