/*
 * $Header$
 *
 * FFIO menu plugin
 * Provides a menu from a FIFO located in ~/.openbox/fifo_menu/id
 * Example:
 * rc3:
 *   <menu id="fonk" label="fonk" plugin="fifo_menu"></menu>
 * Menu format
 * <fifo_menu>
 *    <item label="GLOVE.png">
 *       <action name="execute">
 *          <execute>
 *             bsetbg "/home/woodblock/.openbox/backgrounds/GLOVE.png"
 *          </execute>
 *       </action>
 *    </item>
 *  </fifo_menu>
 *
 * If the attribute pid="true" is in the <menu>
 */

#include <glib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include "kernel/menu.h"
#include "kernel/event.h"

static char *PLUGIN_NAME = "fifo_menu";

typedef struct Fifo_Menu_Data {
    char *fifo;
    char *buf; /* buffer to hold partially read menu */
    unsigned long buflen; /* how many bytes are in the buffer */
    int fd; /* file descriptor to read menu from */
    gboolean use_pid;
    event_fd_handler *handler;
} Fifo_Menu_Data;

#define FIFO_MENU(m) ((ObMenu *)m)
#define FIFO_MENU_DATA(m) ((Fifo_Menu_Data *)((ObMenu *)m)->plugin_data)


void fifo_menu_clean_up(ObMenu *m) {
    if (FIFO_MENU_DATA(m)->buf != NULL) {
        g_free(FIFO_MENU_DATA(m)->buf);
        FIFO_MENU_DATA(m)->buf = NULL;
        FIFO_MENU_DATA(m)->buflen = 0;
    }

    if (FIFO_MENU_DATA(m)->fd != -1) {
        close(FIFO_MENU_DATA(m)->fd);
        FIFO_MENU_DATA(m)->fd = -1;
    }
}

void plugin_setup_config() { }
void plugin_startup()
{ }
void plugin_shutdown() { }

void fifo_menu_handler(int fd, void *d) {
    ObMenu *menu = d;
    char *tmpbuf = NULL;
    unsigned long num_read;
#ifdef DEBUG
    /* because gdb is dumb */
#if 0
    Fifo_Menu_Data *d = FIFO_MENU_DATA(menu);
#endif
#endif
    
    /* if the menu is shown this will go into busy loop :(
     fix me*/
    if (!menu->shown) { 
        unsigned long num_realloc;
        /* if we have less than a quarter BUFSIZ left, allocate more */
        num_realloc = (BUFSIZ - (FIFO_MENU_DATA(menu)->buflen % BUFSIZ) <
                       BUFSIZ >> 2) ?
            0 : BUFSIZ;
    
        tmpbuf = g_try_realloc(FIFO_MENU_DATA(menu)->buf,             
                               FIFO_MENU_DATA(menu)->buflen + num_realloc);

        if (tmpbuf == NULL) {
            g_warning("Unable to allocate memory for read()");
            return;
        }
    
        FIFO_MENU_DATA(menu)->buf = tmpbuf;
    
        num_read = read(fd,
                        FIFO_MENU_DATA(menu)->buf +
                        FIFO_MENU_DATA(menu)->buflen,
                        num_realloc);

        if (num_read == 0) { /* eof */
            xmlDocPtr doc;
            xmlNodePtr node;

            menu->invalid = TRUE;
            menu_clear(menu);

            FIFO_MENU_DATA(menu)->buf[FIFO_MENU_DATA(menu)->buflen] = '\0';

            doc = xmlParseMemory(FIFO_MENU_DATA(menu)->buf,
                                 FIFO_MENU_DATA(menu)->buflen);

            node = xmlDocGetRootElement(doc);
            
            if (node &&
                !xmlStrcasecmp(node->name, (const xmlChar*) "fifo_menu")) {
                parse_menu_full(doc, node, menu, FALSE);
            }
            
            fifo_menu_clean_up(menu);
            
            event_remove_fd(FIFO_MENU_DATA(menu)->handler->fd);
        
            if ((FIFO_MENU_DATA(menu)->fd =
                 open(FIFO_MENU_DATA(menu)->fifo,
                      O_NONBLOCK | O_RDONLY)) == -1) {
                g_warning("Can't reopen FIFO");
                fifo_menu_clean_up(menu);
                return;
            }

            FIFO_MENU_DATA(menu)->handler->fd = FIFO_MENU_DATA(menu)->fd;
        
            event_add_fd_handler(FIFO_MENU_DATA(menu)->handler);
        } else if (num_read > 0) {
            FIFO_MENU_DATA(menu)->buflen += num_read;
            FIFO_MENU_DATA(menu)->buf[FIFO_MENU_DATA(menu)->buflen] = '\0';
        }
    }
}

void plugin_destroy (ObMenu *m)
{
    fifo_menu_clean_up(m);
    if (FIFO_MENU_DATA(m)->handler != NULL) {
        g_free(FIFO_MENU_DATA(m)->handler);
        FIFO_MENU_DATA(m)->handler = NULL;
    }

    if (FIFO_MENU_DATA(m)->fifo != NULL) {
        g_free(FIFO_MENU_DATA(m)->fifo);
        FIFO_MENU_DATA(m)->fifo = NULL;
    }

    if (FIFO_MENU_DATA(m)->buf != NULL) {
        g_free(FIFO_MENU_DATA(m)->buf);
        FIFO_MENU_DATA(m)->buf = NULL;
    }
 
    g_free(m->plugin_data);

    menu_free(m->name);
}

void *plugin_create(PluginMenuCreateData *data)
{
    char *fifo;
    char *dir;
    event_fd_handler *h;
    Fifo_Menu_Data *d;
    ObMenu *m;
    char *label = NULL, *id = NULL;
    char *attr_pid = NULL;
        
    d = g_new(Fifo_Menu_Data, 1);

    parse_attr_string("id", data->node, &id);
    parse_attr_string("label", data->node, &label);
    
    if (parse_attr_string("pid", data->node, &attr_pid) &&
        g_strcasecmp(attr_pid, "true") == 0) {
        d->use_pid = TRUE;
    } else
        d->use_pid = FALSE;
    
    m = menu_new( (label != NULL ? label : ""),
                  (id != NULL ? id : PLUGIN_NAME),
                  data->parent);

    if (data->parent)
        menu_add_entry(data->parent, menu_entry_new_submenu(
                           (label != NULL ? label : ""),
                           m));

    g_free(label);
    g_free(id);
    d->fd = -1;
    d->buf = NULL;
    d->buflen = 0;
    d->handler = NULL;
    
    m->plugin = PLUGIN_NAME;

    d->fd = -1;
    
    m->plugin_data = (void *)d;

        
    dir = g_build_filename(g_get_home_dir(), ".openbox",
                          PLUGIN_NAME, NULL);

    if (mkdir(dir, S_IRWXU | S_IRWXG | S_IRWXO) == -1 && errno != EEXIST) {
/* technically, if ~/.openbox/fifo_menu exists and isn't a directory
   this will fail, but we don't care because mkfifo will fail and warn
   anyway */
        g_warning("Can't create %s: %s", dir, strerror(errno));
        g_free(dir);
        plugin_destroy(m);
        return NULL;
    }

    if (d->use_pid)
    {
        char *pid = g_strdup_printf("%s.%d", m->name, getpid());
        fifo = g_build_filename(g_get_home_dir(), ".openbox",
                                PLUGIN_NAME,
                                pid, NULL);
        g_free(pid);
    } else {
        fifo = g_build_filename(g_get_home_dir(), ".openbox",
                                PLUGIN_NAME,
                                m->name, NULL);
    }
    
    if (mkfifo(fifo, S_IRUSR | S_IWUSR |
               S_IRGRP | S_IWGRP | /* let umask do its thing */
               S_IROTH | S_IWOTH) == -1 && errno != EEXIST) {
        g_warning("Can't create FIFO %s: %s", fifo, strerror(errno));
        g_free(fifo);
        g_free(d);
        menu_free(m->name);
        return NULL;
    }

/* open in non-blocking mode so we don't wait for a process to open FIFO
   for writing */
    if ((d->fd = open(fifo, O_NONBLOCK | O_RDONLY)) == -1) { 
        g_warning("Can't open FIFO %s: %s", fifo, strerror(errno));
        g_free(fifo);
        g_free(d);
        menu_free(m->name);
        return NULL;
    }

    d->fifo = fifo;
    
    h = g_new(event_fd_handler, 1);

    if (h == NULL) {
        g_warning("Out of memory");
        close(d->fd);
        g_free(fifo);
        g_free(d);
        menu_free(m->name);
        return NULL;
    }
    
    h->fd = d->fd;
    h->data = m;
    h->handler = fifo_menu_handler;
    d->handler = h;
    
    event_add_fd_handler(h);
    
    g_free(dir);
    return (void *)m;
}
