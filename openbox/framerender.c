#include "frame.h"
#include "openbox.h"
#include "screen.h"
#include "framerender.h"
#include "render2/theme.h"

void framerender_frame(Frame *self)
{
    /* XXX plate, client border */

    if (self->client->decorations & Decor_Titlebar) {
        struct RrSurface *t, *l, *m, *n, *i, *d, *s, *c;

        t = (self->focused ?
             ob_theme->title_f : ob_theme->title);
        l = (self->focused ?
             ob_theme->label_f : ob_theme->label);
        m = (self->focused ?
             (self->client->max_vert || self->client->max_horz ?
              ob_theme->max_p_f : (self->max_press ?
                                   ob_theme->max_p_f : ob_theme->max_f)) :
             (self->client->max_vert || self->client->max_horz ?
              ob_theme->max_p : (self->max_press ?
                                 ob_theme->max_p : ob_theme->max)));
        n = ob_theme->icon;
        i = (self->focused ?
             (self->iconify_press ?
              ob_theme->iconify_p_f : ob_theme->iconify_f) :
             (self->iconify_press ?
              ob_theme->iconify_p : ob_theme->iconify));
        d = (self->focused ?
             (self->client->desktop == DESKTOP_ALL ?
              ob_theme->desk_p_f : (self->desk_press ?
                                    ob_theme->desk_p_f : ob_theme->desk_f)) :
             (self->client->desktop == DESKTOP_ALL ?
              ob_theme->desk_p : (self->desk_press ?
                                  ob_theme->desk_p : ob_theme->desk)));
        s = (self->focused ?
             (self->client->shaded ?
              ob_theme->shade_p_f : (self->shade_press ?
                                     ob_theme->shade_p_f : ob_theme->shade_f)):
             (self->client->shaded ?
              ob_theme->shade_p : (self->shade_press ?
                                   ob_theme->shade_p : ob_theme->shade)));
        c = (self->focused ?
             (self->close_press ?
              ob_theme->close_p_f : ob_theme->close_f) :
             (self->close_press ?
              ob_theme->close_p : ob_theme->close));

        RrSurfaceCopy(self->s_title, t);
        RrSurfaceCopy(self->s_label, l);

        RrTextureSetText(self->s_label, 0, ob_theme->title_font,
                         ob_theme->title_justify,
                         (self->focused ?
                          &ob_theme->title_color_f : &ob_theme->title_color),
                         self->client->title);

        if (self->icon_x >= 0) {
            Icon *icon;

            RrSurfaceCopy(self->s_icon, n);

            icon = client_icon(self->client,
                             RrThemeButtonSize(ob_theme) + 2,
                             RrThemeButtonSize(ob_theme) + 2);
            if (icon)
                RrTextureSetRGBA(self->s_icon, 0,
                                 icon->data,
                                 0, 0, icon->width, icon->height);
            else
                RrTextureSetNone(self->s_icon, 0);
        }
        if (self->iconify_x >= 0)
            RrSurfaceCopy(self->s_iconify, i);
        if (self->desk_x >= 0)
            RrSurfaceCopy(self->s_desk, d);
        if (self->shade_x >= 0)
            RrSurfaceCopy(self->s_shade, s);
        if (self->close_x >= 0)
            RrSurfaceCopy(self->s_close, c);
    }

    if (self->client->decorations & Decor_Handle) {
        struct RrSurface *h, *g;

        h = (self->focused ?
             ob_theme->handle_f : ob_theme->handle);
        g = (self->focused ?
             ob_theme->grip_f : ob_theme->grip);

        RrSurfaceCopy(self->s_handle, h);
        RrSurfaceCopy(self->s_lgrip, g);
        RrSurfaceCopy(self->s_rgrip, g);
    }
    /* XXX this could be more efficient */
    RrPaint(self->s_frame);
}
