/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

 frame_default_config.h for the Openbox window manager
 Copyright (c) 2006        Mikael Magnusson
 Copyright (c) 2003-2007   Dana Jansens

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 See the COPYING file for a copy of the GNU General Public License.
 */
#ifndef FRAME_CONCEPT2_CONFIG_H_
#define FRAME_CONCEPT2_CONFIG_H_

#include <X11/Xresource.h>
#include "render/render.h"

G_BEGIN_DECLS

struct _ObFrameThemeConfig
{
    const RrInstance *inst;

    gint border_width;
    gint left_width;
    RrColor * focus_border_color;
    RrColor * focus_corner_color;
    RrColor * unfocus_border_color;
    RrColor * unfocus_corner_color;

    gchar *name;
};

typedef struct _ObFrameThemeConfig ObFrameThemeConfig;

/*! The font values are all optional. If a NULL is used for any of them, then
 the default font will be used. */
gint load_theme_config(const RrInstance *inst, const gchar *name,
        const gchar * path, XrmDatabase db, RrFont *active_window_font,
        RrFont *inactive_window_font, RrFont *menu_title_font,
        RrFont *menu_item_font, RrFont *osd_font);

G_END_DECLS

#endif /*FRAME_CONCEPT2_CONFIG_H_*/
