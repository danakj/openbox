#include "../../kernel/dispatch.h"
#include "../../kernel/client.h"
#include "../../kernel/frame.h"
#include "../../kernel/screen.h"
#include "../../kernel/openbox.h"
#include "../../kernel/config.h"
#include "history.h"
#include <glib.h>

gboolean history = TRUE;

void plugin_setup_config()
{
    ConfigValue val;

    config_def_set(config_def_new("placement.remember", Config_Bool,
                                  "Remember Window Positions",
                                  "Place windows where they last were "
                                  "positioned."));
    val.bool = TRUE;
    config_set("placement.remember", Config_Bool, val);
}

static void place_random(Client *c)
{
    int l, r, t, b;
    int x, y;
    Rect *area;

    if (ob_state == State_Starting) return;

    area = screen_area(c->desktop);

    l = area->x;
    t = area->y;
    r = area->x + area->width - c->frame->area.width;
    b = area->y + area->height - c->frame->area.height;

    if (r > l) x = g_random_int_range(l, r + 1);
    else       x = 0;
    if (b > t) y = g_random_int_range(t, b + 1);
    else       y = 0;

    frame_frame_gravity(c->frame, &x, &y); /* get where the client should be */
    client_configure(c, Corner_TopLeft, x, y, c->area.width, c->area.height,
                     TRUE, TRUE);
}

static void event(ObEvent *e, void *foo)
{
    ConfigValue remember;
    gboolean r;

    g_assert(e->type == Event_Client_New);

    /* requested a position */
    if (e->data.c.client->positioned) return;

    r = config_get("placement.remember", Config_Bool, &remember);
    g_assert(r);

    if (!remember.bool || !place_history(e->data.c.client))
        place_random(e->data.c.client);
}

void plugin_startup()
{
    dispatch_register(Event_Client_New, (EventHandler)event, NULL);

    history_startup();
}

void plugin_shutdown()
{
    dispatch_register(0, (EventHandler)event, NULL);

    history_shutdown();
}
