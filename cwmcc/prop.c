#include "cwmcc_internal.h"
#include "atom.h"

#include <X11/Xutil.h>
#include <glib.h>
#include <string.h>

/* this just isn't used...
static gboolean get(Window win, Atom prop, Atom type, int size,
                    guchar **data, gulong num)
{
    gboolean ret = FALSE;
    int res;
    guchar *xdata = NULL;
    Atom ret_type;
    int ret_size;
    gulong ret_items, bytes_left;
    long num32 = 32 / size * num; /\* num in 32-bit elements *\/

    res = XGetWindowProperty(cwmcc_display, win, prop, 0l, num32,
			     FALSE, type, &ret_type, &ret_size,
			     &ret_items, &bytes_left, &xdata);
    if (res == Success && ret_items && xdata) {
	if (ret_size == size && ret_items >= num) {
	    *data = g_memdup(xdata, num * (size / 8));
	    ret = TRUE;
	}
	XFree(xdata);
    }
    return ret;
}
*/

static gboolean get_prealloc(Window win, Atom prop, Atom type, int size,
                             guchar *data, gulong num)
{
    gboolean ret = FALSE;
    int res;
    guchar *xdata = NULL;
    Atom ret_type;
    int ret_size;
    gulong ret_items, bytes_left;
    long num32 = 32 / size * num; /* num in 32-bit elements */

    res = XGetWindowProperty(cwmcc_display, win, prop, 0l, num32,
			     FALSE, type, &ret_type, &ret_size,
			     &ret_items, &bytes_left, &xdata);
    if (res == Success && ret_items && xdata) {
	if (ret_size == size && ret_items >= num) {
	    gulong i;
	    for (i = 0; i < num; ++i)
		switch (size) {
		case 8:
		    data[i] = xdata[i];
		    break;
		case 16:
		    ((guint16*)data)[i] = ((guint16*)xdata)[i];
		    break;
		case 32:
		    ((guint32*)data)[i] = ((guint32*)xdata)[i];
		    break;
		default:
		    g_assert_not_reached(); /* unhandled size */
		}
	    ret = TRUE;
	}
	XFree(xdata);
    }
    return ret;
}

static gboolean get_all(Window win, Atom prop, Atom type, int size,
                        guchar **data, gulong *num)
{
    gboolean ret = FALSE;
    int res;
    guchar *xdata = NULL;
    Atom ret_type;
    int ret_size;
    gulong ret_items, bytes_left;

    res = XGetWindowProperty(cwmcc_display, win, prop, 0l, G_MAXLONG,
			     FALSE, type, &ret_type, &ret_size,
			     &ret_items, &bytes_left, &xdata);
    if (res == Success) {
	if (ret_size == size && ret_items > 0) {
	    *data = g_memdup(xdata, ret_items * (size / 8));
	    *num = ret_items;
	    ret = TRUE;
	}
	XFree(xdata);
    }
    return ret;
}

static gboolean get_stringlist(Window win, Atom prop, char ***list, int *nstr)
{
    XTextProperty tprop;
    gboolean ret = FALSE;

    if (XGetTextProperty(cwmcc_display, win, &tprop, prop) && tprop.nitems) {
        if (XTextPropertyToStringList(&tprop, list, nstr))
            ret = TRUE;
        XFree(tprop.value);
    }
    return ret;
}

gboolean cwmcc_prop_get32(Window win, Atom prop, Atom type, gulong *ret)
{
    return get_prealloc(win, prop, type, 32, (guchar*)ret, 1);
}

gboolean cwmcc_prop_get_array32(Window win, Atom prop, Atom type, gulong **ret,
                          gulong *nret)
{
    return get_all(win, prop, type, 32, (guchar**)ret, nret);
}

gboolean cwmcc_prop_get_string_locale(Window win, Atom prop, char **data)
{
    char **list;
    int nstr;

    if (get_stringlist(win, prop, &list, &nstr) && nstr) {
        *data = g_locale_to_utf8(list[0], -1, NULL, NULL, NULL);
        XFreeStringList(list);
        if (data) return TRUE;
    }
    return FALSE;
}

gboolean cwmcc_prop_get_string_utf8(Window win, Atom prop, char **ret)
{
    char *raw;
    gulong num;
     
    if (get_all(win, prop, CWMCC_ATOM(type, utf8), 8, (guchar**)&raw, &num)) {
	*ret = g_strdup(raw); /* grab the first string from the list */
	g_free(raw);
	return TRUE;
    }
    return FALSE;
}

gboolean cwmcc_prop_get_strings_utf8(Window win, Atom prop, char ***ret)
{
    char *raw, *p;
    gulong num, i;

    if (get_all(win, prop, CWMCC_ATOM(type, utf8), 8, (guchar**)&raw, &num)) {
        *ret = g_new(char*, num + 1);
        (*ret)[num] = NULL; /* null terminated list */

        p = raw;
        for (i = 0; i < num; ++i) {
            (*ret)[i] = g_strdup(p);
            p = strchr(p, '\0');
        }
	g_free(raw);
	return TRUE;
    }
    return FALSE;
}

gboolean cwmcc_prop_get_strings_locale(Window win, Atom prop, char ***ret)
{
    char *raw, *p;
    gulong num, i;

    if (get_all(win, prop, CWMCC_ATOM(type, string), 8, (guchar**)&raw, &num)){
        *ret = g_new(char*, num + 1);
        (*ret)[num] = NULL; /* null terminated list */

        p = raw;
        for (i = 0; i < num; ++i) {
            (*ret)[i] = g_locale_to_utf8(p, -1, NULL, NULL, NULL);
            /* make sure translation did not fail */
            if (!(*ret)[i]) {
                g_strfreev(*ret); /* free what we did so far */
                break; /* the force is not strong with us */
            }
            p = strchr(p, '\0');
        }
	g_free(raw);
        if (i == num)
            return TRUE;
    }
    return FALSE;
}

void cwmcc_prop_set32(Window win, Atom prop, Atom type, gulong val)
{
    XChangeProperty(cwmcc_display, win, prop, type, 32, PropModeReplace,
                    (guchar*)&val, 1);
}

void cwmcc_prop_set_array32(Window win, Atom prop, Atom type,
                            gulong *val, gulong num)
{
    XChangeProperty(cwmcc_display, win, prop, type, 32, PropModeReplace,
                    (guchar*)val, num);
}

void cwmcc_prop_set_string_utf8(Window win, Atom prop, char *val)
{
    XChangeProperty(cwmcc_display, win, prop, CWMCC_ATOM(type, utf8), 8,
                    PropModeReplace, (guchar*)val, strlen(val));
}

void cwmcc_prop_set_strings_utf8(Window win, Atom prop, char **strs)
{
    GString *str;
    guint i;

    str = g_string_sized_new(0);
    for (i = 0; strs[i]; ++i) {
        str = g_string_append(str, strs[i]);
        str = g_string_append_c(str, '\0');
    }
    XChangeProperty(cwmcc_display, win, prop, CWMCC_ATOM(type, utf8), 8,
                    PropModeReplace, (guchar*)str->str, str->len);
}

void cwmcc_prop_erase(Window win, Atom prop)
{
    XDeleteProperty(cwmcc_display, win, prop);
}

