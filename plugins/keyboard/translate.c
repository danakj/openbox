#include "../../kernel/openbox.h"
#include <glib.h>
#include <string.h>

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
    g_message("Invalid modifier '%s' in binding.", str);
    return 0;
}

gboolean translate_key(char *str, guint *state, guint *keycode)
{
    char **parsed;
    char *l;
    int i;
    gboolean ret = FALSE;
    KeySym sym;

    parsed = g_strsplit(str, "-", -1);
    
    /* first, find the key (last token) */
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

    /* figure out the keycode */
    sym = XStringToKeysym(l);
    if (sym == NoSymbol) {
	g_message("Invalid key name '%s' in key binding.", l);
	goto translation_fail;
    }
    *keycode = XKeysymToKeycode(ob_display, sym);
    if (!*keycode) {
	g_message("Key '%s' does not exist on the display.", l); 
	goto translation_fail;
    }

    ret = TRUE;

translation_fail:
    g_strfreev(parsed);
    return ret;
}
