#include "slit.h"
#include "screen.h"
#include "grab.h"
#include "timer.h"
#include "openbox.h"
#include "render/theme.h"
#include "render/render.h"

#define SLIT_EVENT_MASK (ButtonPressMask | ButtonReleaseMask | \
                         EnterWindowMask | LeaveWindowMask)
#define SLITAPP_EVENT_MASK (StructureNotifyMask)

struct Slit {
    Window frame;

    /* user-requested position stuff */
    SlitPosition pos;
    int gravity;
    int user_x, user_y;

    /* actual position (when not auto-hidden) */
    int x, y;
    int w, h;

    gboolean horz;
    gboolean hide;
    gboolean hidden;

    Appearance *a_frame;

    Timer *hide_timer;

    GList *slit_apps;
};

GHashTable *slit_map = NULL;
GHashTable *slit_app_map = NULL;

static Slit *slit;
static int nslits;

static guint slit_hide_timeout = 3000; /* XXX make a config option */

static void slit_configure(Slit *self);

void slit_startup()
{
    XSetWindowAttributes attrib;
    int i;

    slit_map = g_hash_table_new(g_int_hash, g_int_equal);
    slit_app_map = g_hash_table_new(g_int_hash, g_int_equal);

    nslits = 1;
    slit = g_new0(struct Slit, nslits);

    for (i = 0; i < nslits; ++i) {
        slit[i].horz = FALSE;
        slit[i].hide = FALSE;
        slit[i].hidden = TRUE;
        slit[i].pos = SlitPos_TopRight;

        attrib.event_mask = SLIT_EVENT_MASK;
        attrib.override_redirect = True;
        slit[i].frame = XCreateWindow(ob_display, ob_root, 0, 0, 1, 1, 0,
                                      render_depth, InputOutput, render_visual,
                                      CWOverrideRedirect | CWEventMask,
                                      &attrib);
        slit[i].a_frame = appearance_copy(theme_a_unfocused_title);
        XSetWindowBorder(ob_display, slit[i].frame, theme_b_color->pixel);
        XSetWindowBorderWidth(ob_display, slit[i].frame, theme_bwidth);

        g_hash_table_insert(slit_map, &slit[i].frame, &slit[i]);
    }
}

void slit_shutdown()
{
    int i;

    for (i = 0; i < nslits; ++i) {
        XDestroyWindow(ob_display, slit[i].frame);
        appearance_free(slit[i].a_frame);
        g_hash_table_remove(slit_map, &slit[i].frame);
    }
    g_hash_table_destroy(slit_app_map);
    g_hash_table_destroy(slit_map);
}

void slit_add(Window win, XWMHints *wmhints)
{
    Slit *s;
    SlitApp *app;
    XWindowAttributes attrib;

    /* XXX pick a slit */
    s = &slit[0];

    app = g_new0(SlitApp, 1);
    app->slit = s;
    app->win = win;
    app->icon_win = (wmhints->flags & IconWindowHint) ?
        wmhints->icon_window : win;
    
    if (XGetWindowAttributes(ob_display, app->icon_win, &attrib)) {
        app->w = attrib.width;
        app->h = attrib.height;
    } else {
        app->w = app->h = 64;
    }

    s->slit_apps = g_list_append(s->slit_apps, app);
    slit_configure(s);

    XReparentWindow(ob_display, app->icon_win, s->frame, app->x, app->y);
    /*
      This is the same case as in frame.c for client windows. When Openbox is
      starting, the window is already mapped so we see unmap events occur for
      it. There are 2 unmap events generated that we see, one with the 'event'
      member set the root window, and one set to the client, but both get
      handled and need to be ignored.
    */
    if (ob_state == State_Starting)
	app->ignore_unmaps += 2;

    if (app->win != app->icon_win) {
        /* have to map it so that it can be re-managed on a restart */
        XMoveWindow(ob_display, app->win, -1000, -1000);
        XMapWindow(ob_display, app->win);
    }
    XMapWindow(ob_display, app->icon_win);
    XSync(ob_display, False);

    /* specify that if we exit, the window should not be destroyed and should
       be reparented back to root automatically */
    XChangeSaveSet(ob_display, app->icon_win, SetModeInsert);
    XSelectInput(ob_display, app->icon_win, SLITAPP_EVENT_MASK);

    grab_button_full(2, 0, app->icon_win,
                     ButtonPressMask | ButtonReleaseMask | ButtonMotionMask,
                     GrabModeAsync, ob_cursors.move);

    g_hash_table_insert(slit_app_map, &app->icon_win, app);

    g_message("Managed Slit App: 0x%lx", app->icon_win);
}

void slit_remove_all()
{
    int i;

    for (i = 0; i < nslits; ++i)
        while (slit[i].slit_apps)
            slit_remove(slit[i].slit_apps->data, TRUE);
}

void slit_remove(SlitApp *app, gboolean reparent)
{
    ungrab_button(2, 0, app->icon_win);
    XSelectInput(ob_display, app->icon_win, NoEventMask);
    /* remove the window from our save set */
    XChangeSaveSet(ob_display, app->icon_win, SetModeDelete);
    XSync(ob_display, False);

    g_hash_table_remove(slit_app_map, &app->icon_win);

    if (reparent)
	XReparentWindow(ob_display, app->icon_win, ob_root, app->x, app->y);

    app->slit->slit_apps = g_list_remove(app->slit->slit_apps, app);
    slit_configure(app->slit);

    g_message("Unmanaged Slit App: 0x%lx", app->icon_win);

    g_free(app);
}

void slit_configure_all()
{
    int i; for (i = 0; i < nslits; ++i) slit_configure(&slit[i]);
}

static void slit_configure(Slit *self)
{
    GList *it;
    int spot;

    self->w = self->h = spot = 0;

    for (it = self->slit_apps; it; it = it->next) {
        struct SlitApp *app = it->data;
        if (self->horz) {
            app->x = spot;
            app->y = 0;
            self->w += app->w;
            self->h = MAX(self->h, app->h);
            spot += app->w;
        } else {
            app->x = 0;
            app->y = spot;
            self->w = MAX(self->w, app->w);
            self->h += app->h;
            spot += app->h;
        }

        XMoveWindow(ob_display, app->icon_win, app->x, app->y);
    }

    /* used for calculating offsets */
    self->w += theme_bwidth * 2;
    self->h += theme_bwidth * 2;

    /* calculate position */
    switch (self->pos) {
    case SlitPos_Floating:
        self->x = self->user_x;
        self->y = self->user_y;
        break;
    case SlitPos_TopLeft:
        self->x = 0;
        self->y = 0;
        self->gravity = NorthWestGravity;
        break;
    case SlitPos_Top:
        self->x = screen_physical_size.width / 2;
        self->y = 0;
        self->gravity = NorthGravity;
        break;
    case SlitPos_TopRight:
        self->x = screen_physical_size.width;
        self->y = 0;
        self->gravity = NorthEastGravity;
        break;
    case SlitPos_Left:
        self->x = 0;
        self->y = screen_physical_size.height / 2;
        self->gravity = WestGravity;
        break;
    case SlitPos_Right:
        self->x = screen_physical_size.width;
        self->y = screen_physical_size.height / 2;
        self->gravity = EastGravity;
        break;
    case SlitPos_BottomLeft:
        self->x = 0;
        self->y = screen_physical_size.height;
        self->gravity = SouthWestGravity;
        break;
    case SlitPos_Bottom:
        self->x = screen_physical_size.width / 2;
        self->y = screen_physical_size.height;
        self->gravity = SouthGravity;
        break;
    case SlitPos_BottomRight:
        self->x = screen_physical_size.width;
        self->y = screen_physical_size.height;
        self->gravity = SouthEastGravity;
        break;
    }

    switch(self->gravity) {
    case NorthGravity:
    case CenterGravity:
    case SouthGravity:
        self->x -= self->w / 2;
        break;
    case NorthEastGravity:
    case EastGravity:
    case SouthEastGravity:
        self->x -= self->w;
        break;
    }
    switch(self->gravity) {
    case WestGravity:
    case CenterGravity:
    case EastGravity:
        self->y -= self->h / 2;
        break;
    case SouthWestGravity:
    case SouthGravity:
    case SouthEastGravity:
        self->y -= self->h;
        break;
    }

    if (self->hide && self->hidden) {
        g_message("hidden");
        switch (self->pos) {
        case SlitPos_Floating:
            break;
        case SlitPos_TopLeft:
            if (self->horz)
                self->y -= self->h - theme_bwidth;
            else
                self->x -= self->w - theme_bwidth;
            break;
        case SlitPos_Top:
            self->y -= self->h - theme_bwidth;
            break;
        case SlitPos_TopRight:
            if (self->horz)
                self->y -= self->h - theme_bwidth;
            else
                self->x += self->w - theme_bwidth;
            break;
        case SlitPos_Left:
            self->x -= self->w - theme_bwidth;
            break;
        case SlitPos_Right:
            self->x += self->w - theme_bwidth;
            break;
        case SlitPos_BottomLeft:
            if (self->horz)
                self->y += self->h - theme_bwidth;
            else
                self->x -= self->w - theme_bwidth;
            break;
        case SlitPos_Bottom:
            self->y += self->h - theme_bwidth;
            break;
        case SlitPos_BottomRight:
            if (self->horz)
                self->y += self->h - theme_bwidth;
            else
                self->x += self->w - theme_bwidth;
            break;
        }    
    }

    /* not used for actually sizing shit */
    self->w -= theme_bwidth * 2;
    self->h -= theme_bwidth * 2;

    if (self->w > 0 && self->h > 0) {
        RECT_SET(self->a_frame->area, 0, 0, self->w, self->h);
        XMoveResizeWindow(ob_display, self->frame,
                          self->x, self->y, self->w, self->h);

        paint(self->frame, self->a_frame);
        XMapWindow(ob_display, self->frame);
    } else
        XUnmapWindow(ob_display, self->frame);

    /* but they are useful outside of this function! */
    self->w += theme_bwidth * 2;
    self->h += theme_bwidth * 2;
}

void slit_app_configure(SlitApp *app, int w, int h)
{
    app->w = w;
    app->h = h;
    slit_configure(app->slit);
}

void slit_app_drag(SlitApp *app, XMotionEvent *e)
{
    Slit *src, *dest = NULL;
    SlitApp *over = NULL;
    GList *it;
    int i;
    int x, y;
    gboolean after;

    src = app->slit;
    x = e->x_root;
    y = e->y_root;

    /* which slit are we on top of? */
    for (i = 0; i < nslits; ++i)
        if (x >= slit[i].x &&
            y >= slit[i].y &&
            x < slit[i].x + slit[i].w &&
            y < slit[i].y + slit[i].h) {
            dest = &slit[i];
            break;
        }
    if (!dest) return;

    x -= dest->x;
    y -= dest->y;

    /* which slit app are we on top of? */
    for (it = dest->slit_apps; it; it = it->next) {
        over = it->data;
        if (dest->horz) {
            if (x >= over->x && x < over->x + over->w)
                break;
        } else {
            if (y >= over->y && y < over->y + over->h)
                break;
        }
    }
    if (!it || app == over) return;

    x -= over->x;
    y -= over->y;

    if (dest->horz)
        after = (x > over->w / 2);
    else
        after = (y > over->h / 2);

    /* remove before doing the it->next! */
    src->slit_apps = g_list_remove(src->slit_apps, app);
    if (src != dest) slit_configure(src);

    if (after) it = it->next;

    dest->slit_apps = g_list_insert_before(dest->slit_apps, it, app);
    slit_configure(dest);
}

static void hide_timeout(Slit *self)
{
    /* dont repeat */
    timer_stop(self->hide_timer);
    self->hide_timer = NULL;

    /* hide */
    self->hidden = TRUE;
    slit_configure(self);
}

void slit_hide(Slit *self, gboolean hide)
{
    if (self->hidden == hide || !self->hide)
        return;
    if (!hide) {
        /* show */
        self->hidden = FALSE;
        slit_configure(self);

        /* if was hiding, stop it */
        if (self->hide_timer) {
            timer_stop(self->hide_timer);
            self->hide_timer = NULL;
        }
    } else {
        g_assert(!self->hide_timer);
        self->hide_timer = timer_start(slit_hide_timeout * 1000,
                                       (TimeoutHandler)hide_timeout, self);
    }
}
