#include "openbox/action.h"
#include "openbox/action_value.h"
#include "openbox/client.h"
#include "openbox/screen.h"
#include "openbox/frame.h"
#include "openbox/config.h"

typedef struct {
    gint left;
    gint left_denom;
    gint right;
    gint right_denom;
    gint top;
    gint top_denom;
    gint bottom;
    gint bottom_denom;
} Options;

static gpointer setup_func(GHashTable *config);
static void     free_func(gpointer options);
static gboolean run_func(ObActionData *data, gpointer options);

void action_resizerelative_startup(void)
{
    action_register("ResizeRelative", setup_func, free_func, run_func);
}

static gpointer setup_func(GHashTable *config)
{
    ObActionValue *v;
    Options *o;

    o = g_slice_new0(Options);

    v = g_hash_table_lookup(config, "left");
    if (v && action_value_is_string(v))
        action_value_fraction(v, &o->left, &o->left_denom);
    v = g_hash_table_lookup(config, "right");
    if (v && action_value_is_string(v))
        action_value_fraction(v, &o->right, &o->right_denom);
    v = g_hash_table_lookup(config, "top");
    if (v && action_value_is_string(v))
        action_value_fraction(v, &o->top, &o->top_denom);
    v = g_hash_table_lookup(config, "bottom");
    if (v && action_value_is_string(v))
        action_value_fraction(v, &o->bottom, &o->bottom_denom);

    return o;
}

static void free_func(gpointer o)
{
    g_slice_free(Options, o);
}

/* Always return FALSE because its not interactive */
static gboolean run_func(ObActionData *data, gpointer options)
{
    Options *o = options;

    if (data->client) {
        ObClient *c = data->client;
        gint x, y, ow, xoff, nw, oh, yoff, nh, lw, lh;
        gint left = o->left, right = o->right, top = o->top, bottom = o->bottom;

        if (o->left_denom)
            left = left * c->area.width / o->left_denom;
        if (o->right_denom)
            right = right * c->area.width / o->right_denom;
        if (o->top_denom)
            top = top * c->area.height / o->top_denom;
        if (o->bottom_denom)
            bottom = bottom * c->area.height / o->bottom_denom;

        // When resizing, if the resize has a non-zero value then make sure it
        // is at least as big as the size increment so the window does actually
        // resize.
        x = c->area.x;
        y = c->area.y;
        ow = c->area.width;
        xoff = -MAX(left, (left ? c->size_inc.width : 0));
        nw = ow + MAX(right + left, (right + left ? c->size_inc.width : 0));
        oh = c->area.height;
        yoff = -MAX(top, (top ? c->size_inc.height : 0));
        nh = oh + MAX(bottom + top, (bottom + top ? c->size_inc.height : 0));

        client_try_configure(c, &x, &y, &nw, &nh, &lw, &lh, TRUE);
        xoff = xoff == 0 ? 0 :
            (xoff < 0 ? MAX(xoff, ow-nw) : MIN(xoff, ow-nw));
        yoff = yoff == 0 ? 0 :
            (yoff < 0 ? MAX(yoff, oh-nh) : MIN(yoff, oh-nh));

        action_client_move(data, TRUE);
        client_move_resize(c, x + xoff, y + yoff, nw, nh);
        action_client_move(data, FALSE);
    }

    return FALSE;
}
