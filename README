# Openbox

Openbox fork patched for [Steambox](https://github.com/garnetius/steambox).

![Preview](https://raw.github.com/garnetius/steambox/master/screenshots/switch-preview.png "Preview")
[Full screenshot](https://raw.github.com/garnetius/steambox/master/screenshots/switch.png)

## Added Features

  * Add support for graphical icons in title bar: [example](https://raw.github.com/garnetius/steambox/master/screenshots/desktop.png).
    Use `.png` instead of `.xbm` with the same base name for each icon state to load graphical icons.
  * Fix setting virtual desktops layout from `rc.xml`: `setlayout` program no longer needed.
    Desktop layout would be properly updated upon `openbox --restart` after editing the configuration file.
  * Menus can now have configurable vertical and horizontal padding for items, as well as spacing between individual items.
  * Add ability to change border width and color of window [selector](https://raw.github.com/garnetius/steambox/master/screenshots/switch.png).
  * Add support for ethereal window borders: borders that are drawn normally, but are ignored in screen and window resistance calculations.
    Useful with [compositors](https://github.com/garnetius/compton) which implement border opacity.
  * New `keepBorderMaximized` property (override `keepBorder` behavior for maximized windows).
  * New default window [icon](https://github.com/garnetius/openbox/blob/master/data/openbox.png).
  * Proper Title Case for Menu Labels (default English strings only).
  * Proper handling of `MOTIF_WM_HINTS`.
    Useful with [tint2](https://github.com/garnetius/tint2) to save vertical screen space.
  * Many other little fixes and adjustments.


###### New Configuration Properties

  * `borderTitle`.
  * `keepBorderMaximized`: override `keepBorder` behavior for maximized windows.
  * `borderEthereal`: toggle ethereal window borders.
  * `borderForceCSD`: force ethereal window borders for windows with CSD (client-side decorations).
  * `windowListScrollMargin`: margin for client window list.
  * `desktops/orientation`: virtual desktops layout orientation.
  * `desktops/rows`: number of virtual desktops rows.
  * `desktops/columns`: number of virtual desktops columns.
  * `desktops/startingCorner`: virtual desktops starting edge.

Steambox [rc.xml](https://github.com/garnetius/steambox/blob/master/.config/openbox/rc.xml).

###### New Theme Properties

  * `menu.padding.width`, `menu.padding.height`, `menu.spacing`.
  * `window.button.spacing`.
  * `osd.padding.width`, `osd.padding.height`.
  * `osd.focus.border.width`.
  * `osd.focus.hiliteouter`, `osd.focus.hiliteinner`, `osd.focus.hilitemargin`.
  * `osd.focus.margin`, `osd.focus.textmargin`, `osd.focus.marginicons`.

[Arc](https://github.com/garnetius/steambox/blob/master/.themes/Arc/openbox-3/themerc) Steambox theme.

## Original Readme

Copyright (C) 2015 Kristian Garnét
Copyright (C) 2004 Mikael Magnusson
Copyright (C) 2002 Dana Jansens

```
This software is OSI Certified Open Source Software.
OSI Certified is a certification mark of the Open Source Initiative.
```

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

See the COPYING file for a copy of the GNU General Public License.
