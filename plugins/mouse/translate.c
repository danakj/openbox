#include "kernel/openbox.h"
#include "mouse.h"
#include <glib.h>
#include <string.h>
#include <stdlib.h>

static guint translate_modifier(char *str)
{
    if (!g_ascii_strcasecmp("Mod1", str) ||
        !g_ascii_strcasecmp("A", str)) return Mod1Mask;
    else if (!g_ascii_strcasecmp("Mod2", str)) return Mod2Mask;
    else if (!g_ascii_strcasecmp("Mod3", str)) return Mod3Mask;
    else if (!g_ascii_strcasecmp("Mod4", str) ||
             !g_ascii_strcasecmp("W", str)) return Mod4Mask;
    else if (!g_ascii_strcasecmp("Mod5", str)) return Mod5Mask;
    else if (!g_ascii_strcasecmp("Control", str) ||
             !g_ascii_strcasecmp("C", str)) return ControlMask;
    else if (!g_ascii_strcasecmp("Shift", str) ||
             !g_ascii_strcasecmp("S", str)) return ShiftMask;
    g_warning("Invalid modifier '%s' in binding.", str);
    return 0;
}

gboolean translate_button(char *str, guint *state, guint *button)
{
    char **parsed;
    char *l;
    int i;
    gboolean ret = FALSE;

    parsed = g_strsplit(str, "-", -1);
    
    /* first, find the button (last token) */
    l = NULL;
    for (i = 0; parsed[i] != NULL; ++i)
	l = parsed[i];
    if (l == NULL)
	goto translation_fail;

    /* figure out the mod mask */
    *state = 0;
    for (i = 0; parsed[i] != l; ++i) {
	guint m = translate_modifier(parsed[i]);
	if (!m) goto translation_fail;
	*state |= m;
    }

    /* figure out the button */
    if (!g_ascii_strcasecmp("Left", l)) *button = 1;
    else if (!g_ascii_strcasecmp("Middle", l)) *button = 2;
    else if (!g_ascii_strcasecmp("Right", l)) *button = 3;
    else if (!g_ascii_strcasecmp("Up", l)) *button = 4;
    else if (!g_ascii_strcasecmp("Down", l)) *button = 5;
    else if (!g_ascii_strncasecmp("Button", l, 6)) *button = atoi(l+6);
    if (!*button) {
	g_warning("Invalid button '%s' in pointer binding.", l);
	goto translation_fail;
    }

    ret = TRUE;

translation_fail:
    g_strfreev(parsed);
    return ret;
}
