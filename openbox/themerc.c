#include <glib.h>
#ifdef HAVE_STRING_H
#  include <string.h>
#endif

char *themerc_engine;
char *themerc_theme;
char *themerc_font;
char *themerc_titlebar_layout;

GError *error;

static void parse(char *path, int fd)
{
    GScanner *scanner;

    scanner = g_scanner_new(NULL);
    g_scanner_input_file(scanner, fd);

    while (g_scanner_get_next_token(scanner) != G_TOKEN_EOF) {
	char *name, *val;

	if (scanner->token != G_TOKEN_IDENTIFIER) {
	    g_scanner_unexp_token(scanner, scanner->token, NULL, NULL, NULL,
				  NULL, TRUE);
	    return;
	}
	name = g_strdup(scanner->value.v_identifier);

	g_scanner_get_next_token(scanner);
	if (scanner->token != G_TOKEN_EQUAL_SIGN) {
	    g_scanner_unexp_token(scanner, scanner->token, NULL, NULL, NULL,
				  NULL, TRUE);
	    g_free(name);
	    return;
	}

	g_scanner_get_next_token(scanner);
	if (scanner->token == G_TOKEN_STRING) {
	    val = g_strdup(scanner->value.v_identifier);

	    if (!g_ascii_strcasecmp(name, "engine")) {
		if (themerc_engine != NULL) {
		    g_warning("%s:%d: '%s' already defined", path,
			      scanner->line, name);
		    g_free(name);
		    g_free(val);
		} else
		    themerc_engine = val;
	    } else if (!g_ascii_strcasecmp(name, "theme")) {
		if (themerc_theme != NULL) {
		    g_warning("%s:%d: '%s' already defined", path,
			      scanner->line, name);
		    g_free(name);
		    g_free(val);
		} else
		    themerc_theme = val;
	    } else if (!g_ascii_strcasecmp(name, "font")) {
		if (themerc_font != NULL) {
		    g_warning("%s:%d: '%s' already defined", path,
			      scanner->line, name);
		    g_free(name);
		    g_free(val);
		} else
		    themerc_font = val;
	    } else if (!g_ascii_strcasecmp(name, "titlebarlayout")) {
		if (themerc_titlebar_layout != NULL) {
		    g_warning("%s:%d: '%s' already defined", path,
			      scanner->line, name);
		    g_free(name);
		    g_free(val);
		} else {
		    char *lowval = g_ascii_strup(val, -1);
		    int i, len = strlen(lowval);
		    g_free(val);
		    for (i = 0; i < len; ++i) {
			gboolean valid = FALSE;
			switch(lowval[i]) {
			case 'I':
			case 'L':
			case 'M':
			case 'C':
			case 'N':
			case 'D':
			    valid = TRUE;
			}
			if (!valid) {
			    g_warning("%s:%d: invalid titlebarlayout element "
				      "'%c'", path, scanner->line, lowval[i]);
			    break;
			}
		    }
		    if (i == len)
			themerc_titlebar_layout = lowval;
		    else {
			g_free(name);
			g_free(val);
		    }
		}
	    } else {
		g_warning("%s:%d: invalid option '%s'", path,
			  scanner->line, name);
		g_free(name);
		g_free(val);
	    }
	} else {
	    g_scanner_unexp_token(scanner, scanner->token, NULL, NULL, NULL,
				  NULL, TRUE);
	    g_free(name);
	    return;
	}
    }
}

void themerc_startup()
{
    GIOChannel *chan = NULL;
    char *path = NULL;

    /* defaults */
    themerc_engine = NULL;
    themerc_theme = NULL;
    themerc_font = NULL;
    themerc_titlebar_layout = NULL;

    path = g_build_filename(g_get_home_dir(), ".openbox", "themerc", NULL);
    error = NULL;
    chan = g_io_channel_new_file(path, "r", &error);

    if (chan == NULL) {
	g_free(path);
	path = g_build_filename(THEMERCDIR, "themerc", NULL);
	error = NULL;
	chan = g_io_channel_new_file(path, "r", &error);
    }

    if (chan != NULL) {
	parse(path, g_io_channel_unix_get_fd(chan));
	g_free(path);
	g_io_channel_close(chan);
    }

    /* non-NULL defaults */
    if (themerc_titlebar_layout == NULL)
	themerc_titlebar_layout = g_strdup("NDLIMC");
    if (themerc_font == NULL)
	themerc_font = g_strdup("sans-7");
}

void themerc_shutdown()
{
    if (themerc_engine != NULL) g_free(themerc_engine);
    if (themerc_theme != NULL) g_free(themerc_theme);
    if (themerc_font != NULL) g_free(themerc_font);
    if (themerc_titlebar_layout != NULL) g_free(themerc_titlebar_layout);
}
