#include "kernel/menu.h"
#include "kernel/timer.h"
#include "timed_menu.h"
#include "kernel/action.h"

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
} Timed_Menu_Data;


void plugin_setup_config() { }
void plugin_startup()
{ }
void plugin_shutdown() { }

void timed_menu_timeout_handler(void *data)
{
    Action *a;
    printf("woop timer %s\n", ((Menu *)data)->name);
    ((Menu *)data)->invalid = TRUE;

    if (((Menu *)data)->shown) {
	a = action_from_string("execute");
	a->data.execute.path = g_strdup("xeyes");
	menu_add_entry((Menu *)data, menu_entry_new("xeyes", a));

	menu_show_full( (Menu *)data, ((Menu *)data)->location.x,
			((Menu *)data)->location.y, NULL);
    } else {
	GList *it;

	for (it = ((Menu *)data)->entries; it; it = it->next) {
	    MenuEntry *entry = it->data;
	    menu_entry_free(entry);
	}
	((Menu *)data)->entries = NULL;
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
    timer_stop( ((Timed_Menu_Data *)((Menu *)m)->plugin_data)->timer);
    
    g_free( ((Menu *)m)->plugin_data );
}
