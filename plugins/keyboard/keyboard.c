#include "../../kernel/focus.h"
#include "../../kernel/dispatch.h"
#include "../../kernel/openbox.h"
#include "../../kernel/grab.h"
#include "../../kernel/action.h"
#include "tree.h"
#include "keyboard.h"
#include <glib.h>

KeyBindingTree *firstnode;

static KeyBindingTree *curpos;
static guint reset_key, reset_state;
static gboolean grabbed;

static void grab_keys(gboolean grab)
{
    if (!grab) {
        ungrab_all_keys();
    } else {
	KeyBindingTree *p = firstnode;
	while (p) {
            grab_key(p->key, p->state, GrabModeSync);
	    p = p->next_sibling;
	}
    }
}

static void reset_chains()
{
    /* XXX kill timer */
    curpos = NULL;
    if (grabbed) {
	grabbed = FALSE;
        grab_keyboard(FALSE);
    }
}

static gboolean kbind(GList *keylist, Action *action)
{
    KeyBindingTree *tree, *t;
    gboolean conflict;

    g_assert(keylist != NULL);
    g_assert(action != NULL);

    if (!(tree = tree_build(keylist))) {
        g_warning("invalid binding");
        return FALSE;
    }
    if ((t = tree_find(tree, &conflict)) != NULL) {
	/* already bound to something */
	g_warning("keychain is already bound");
        tree_destroy(tree);
        return FALSE;
    }
    if (conflict) {
        g_warning("conflict with binding");
        tree_destroy(tree);
        return FALSE;
    }

    /* grab the server here to make sure no key presses go missed */
    grab_server(TRUE);
    grab_keys(FALSE);

    /* set the action */
    t = tree;
    while (t->first_child) t = t->first_child;
    t->action = action;
    /* assimilate this built tree into the main tree. assimilation
       destroys/uses the tree */
    tree_assimilate(tree);

    grab_keys(TRUE); 
    grab_server(FALSE);

    return TRUE;
}

static void press(ObEvent *e, void *foo)
{
    if (e->data.x.e->xkey.keycode == reset_key &&
        e->data.x.e->xkey.state == reset_state) {
        reset_chains();
        XAllowEvents(ob_display, AsyncKeyboard, CurrentTime);
    } else {
        KeyBindingTree *p;
        if (curpos == NULL)
            p = firstnode;
        else
            p = curpos->first_child;
        while (p) {
            if (p->key == e->data.x.e->xkey.keycode &&
                p->state == e->data.x.e->xkey.state) {
                if (p->first_child != NULL) { /* part of a chain */
                    /* XXX TIMER */
                    if (!grabbed) {
                        grab_keyboard(TRUE);
                        grabbed = TRUE;
                        XAllowEvents(ob_display, AsyncKeyboard, CurrentTime);
                    }
                    curpos = p;
                } else {
                    if (p->action->func != NULL) {
                        p->action->data.any.c = focus_client;

                        g_assert(!(p->action->func == action_move ||
                                   p->action->func == action_resize));

                        p->action->func(&p->action->data);
                    }

                    XAllowEvents(ob_display, AsyncKeyboard, CurrentTime);
                    reset_chains();
                }
                break;
            }
            p = p->next_sibling;
        }
    }
}

static void binddef()
{
    GList *list = g_list_append(NULL, NULL);
    Action *a;

    /* When creating an Action struct, all of the data elements in the
       appropriate struct need to be set, except the Client*, which will be set
       at call-time when then action function is used.
    */

    list->data = "A-Right";
    a = action_new(action_next_desktop);
    a->data.nextprevdesktop.wrap = TRUE;
    kbind(list, a);

    list->data = "A-Left";
    a = action_new(action_previous_desktop);
    a->data.nextprevdesktop.wrap = TRUE;
    kbind(list, a);

    list->data = "A-1";
    a = action_new(action_desktop);
    a->data.desktop.desk = 0;
    kbind(list, a);

    list->data = "A-2"; 
    a = action_new(action_desktop);
    a->data.desktop.desk = 1;
    kbind(list, a);

    list->data = "A-3";
    a = action_new(action_desktop);
    a->data.desktop.desk = 2;
    kbind(list, a);

    list->data = "A-4";
    a = action_new(action_desktop);
    a->data.desktop.desk = 3;
    kbind(list, a);

    list->data = "A-space";
    a = action_new(action_execute);
    a->data.execute.path = g_strdup("xterm");
    kbind(list, a);

    list->data = "C-A-Escape";
    a = action_new(action_execute);
    a->data.execute.path = g_strdup("xlock -nolock -mode puzzle");
    kbind(list, a);
}

void plugin_startup()
{
    dispatch_register(Event_X_KeyPress, (EventHandler)press, NULL);

    /* XXX parse config file! */
    binddef();
}

void plugin_shutdown()
{
    dispatch_register(0, (EventHandler)press, NULL);

    grab_keys(FALSE);
    tree_destroy(firstnode);
    firstnode = NULL;
    grab_keys(TRUE);
}

