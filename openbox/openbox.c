#include "debug.h"
#include "openbox.h"
#include "session.h"
#include "dock.h"
#include "event.h"
#include "menu.h"
#include "client.h"
#include "xerror.h"
#include "prop.h"
#include "screen.h"
#include "startupnotify.h"
#include "focus.h"
#include "moveresize.h"
#include "frame.h"
#include "keyboard.h"
#include "mouse.h"
#include "extensions.h"
#include "menuframe.h"
#include "grab.h"
#include "group.h"
#include "config.h"
#include "mainloop.h"
#include "gettext.h"
#include "parser/parse.h"
#include "render/render.h"
#include "render/theme.h"

#ifdef HAVE_FCNTL_H
#  include <fcntl.h>
#endif
#ifdef HAVE_SIGNAL_H
#define __USE_UNIX98
#  include <signal.h>
#endif
#ifdef HAVE_STDLIB_H
#  include <stdlib.h>
#endif
#ifdef HAVE_LOCALE_H
#  include <locale.h>
#endif
#ifdef HAVE_SYS_STAT_H
#  include <sys/stat.h>
#  include <sys/types.h>
#endif
#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif

#include <X11/cursorfont.h>

RrInstance *ob_rr_inst;
RrTheme    *ob_rr_theme;
ObMainLoop *ob_main_loop;
Display    *ob_display;
gint        ob_screen;
gboolean    ob_sm_use = TRUE;
gchar      *ob_sm_id;
gboolean    ob_replace_wm;

static ObState   state;
static gboolean  xsync;
static gboolean  reconfigure;
static gboolean  restart;
static char     *restart_path;
static Cursor    cursors[OB_NUM_CURSORS];
static KeyCode   keys[OB_NUM_KEYS];
static gchar    *sm_save_file;

static void signal_handler(int signal, gpointer data);
static void parse_args(int argc, char **argv);

int main(int argc, char **argv)
{
    char *path;

#ifdef DEBUG
    ob_debug_show_output(TRUE);
#endif

    state = OB_STATE_STARTING;

    /* initialize the locale */
    if (!setlocale(LC_ALL, ""))
	g_warning("Couldn't set locale from environment.\n");
    bindtextdomain(PACKAGE_NAME, LOCALEDIR);
    bind_textdomain_codeset(PACKAGE_NAME, "UTF-8");
    textdomain(PACKAGE_NAME);

    /* create the ~/.openbox dir */
    path = g_build_filename(g_get_home_dir(), ".openbox", NULL);
    mkdir(path, (S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP |
                 S_IROTH | S_IWOTH | S_IXOTH));
    g_free(path);
    /* create the ~/.openbox/themes dir */
    path = g_build_filename(g_get_home_dir(), ".openbox", "themes", NULL);
    mkdir(path, (S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP |
                 S_IROTH | S_IWOTH | S_IXOTH));
    g_free(path);
    /* create the ~/.openbox/sessions dir */
    path = g_build_filename(g_get_home_dir(), ".openbox", "sessions", NULL);
    mkdir(path, (S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP |
                 S_IROTH | S_IWOTH | S_IXOTH));
    g_free(path);

    g_set_prgname(argv[0]);
     
    /* parse out command line args */
    parse_args(argc, argv);

    ob_display = XOpenDisplay(NULL);
    if (ob_display == NULL)
	ob_exit_with_error("Failed to open the display.");
    if (fcntl(ConnectionNumber(ob_display), F_SETFD, 1) == -1)
        ob_exit_with_error("Failed to set display as close-on-exec.");

    ob_main_loop = ob_main_loop_new(ob_display);

    /* set up signal handler */
    ob_main_loop_signal_add(ob_main_loop, SIGUSR1, signal_handler, NULL, NULL);
    ob_main_loop_signal_add(ob_main_loop, SIGUSR2, signal_handler, NULL, NULL);
    ob_main_loop_signal_add(ob_main_loop, SIGTERM, signal_handler, NULL, NULL);
    ob_main_loop_signal_add(ob_main_loop, SIGINT, signal_handler, NULL, NULL);
    ob_main_loop_signal_add(ob_main_loop, SIGHUP, signal_handler, NULL, NULL);
    ob_main_loop_signal_add(ob_main_loop, SIGPIPE, signal_handler, NULL, NULL);

    if (sm_save_file)
        session_load(sm_save_file);
    session_startup(argc, argv);

    ob_screen = DefaultScreen(ob_display);

    ob_rr_inst = RrInstanceNew(ob_display, ob_screen);
    if (ob_rr_inst == NULL)
        ob_exit_with_error("Failed to initialize the render library.");

    /* XXX fork self onto other screens */
     
    XSynchronize(ob_display, xsync);

    /* check for locale support */
    if (!XSupportsLocale())
	g_warning("X server does not support locale.");
    if (!XSetLocaleModifiers(""))
	g_warning("Cannot set locale modifiers for the X server.");

    /* set our error handler */
    XSetErrorHandler(xerror_handler);

    /* set the DISPLAY environment variable for any lauched children, to the
       display we're using, so they open in the right place. */
    putenv(g_strdup_printf("DISPLAY=%s", DisplayString(ob_display)));

    /* create available cursors */
    cursors[OB_CURSOR_NONE] = None;
    cursors[OB_CURSOR_POINTER] =
        XCreateFontCursor(ob_display, XC_left_ptr);
    cursors[OB_CURSOR_BUSY] =
        XCreateFontCursor(ob_display, XC_watch);
    cursors[OB_CURSOR_MOVE] =
        XCreateFontCursor(ob_display, XC_fleur);
    cursors[OB_CURSOR_NORTH] =
        XCreateFontCursor(ob_display, XC_top_side);
    cursors[OB_CURSOR_NORTHEAST] =
        XCreateFontCursor(ob_display, XC_top_right_corner);
    cursors[OB_CURSOR_EAST] =
        XCreateFontCursor(ob_display, XC_right_side);
    cursors[OB_CURSOR_SOUTHEAST] =
        XCreateFontCursor(ob_display, XC_bottom_right_corner);
    cursors[OB_CURSOR_SOUTH] =
        XCreateFontCursor(ob_display, XC_bottom_side);
    cursors[OB_CURSOR_SOUTHWEST] =
        XCreateFontCursor(ob_display, XC_bottom_left_corner);
    cursors[OB_CURSOR_WEST] =
        XCreateFontCursor(ob_display, XC_left_side);
    cursors[OB_CURSOR_NORTHWEST] =
        XCreateFontCursor(ob_display, XC_top_left_corner);

    /* create available keycodes */
    keys[OB_KEY_RETURN] =
        XKeysymToKeycode(ob_display, XStringToKeysym("Return"));
    keys[OB_KEY_ESCAPE] =
        XKeysymToKeycode(ob_display, XStringToKeysym("Escape"));
    keys[OB_KEY_LEFT] =
        XKeysymToKeycode(ob_display, XStringToKeysym("Left"));
    keys[OB_KEY_RIGHT] =
        XKeysymToKeycode(ob_display, XStringToKeysym("Right"));
    keys[OB_KEY_UP] =
        XKeysymToKeycode(ob_display, XStringToKeysym("Up"));
    keys[OB_KEY_DOWN] =
        XKeysymToKeycode(ob_display, XStringToKeysym("Down"));

    prop_startup(); /* get atoms values for the display */
    extensions_query_all(); /* find which extensions are present */

    if (screen_annex()) { /* it will be ours! */
        do {
            event_startup(reconfigure);
            grab_startup(reconfigure);
            /* focus_backup is used for stacking, so this needs to come before
               anything that calls stacking_add */
            focus_startup(reconfigure);
            window_startup(reconfigure);
            sn_startup(reconfigure);

            {
                ObParseInst *i;
                xmlDocPtr doc;
                xmlNodePtr node;

                /* startup the parsing so everything can register sections
                   of the rc */
                i = parse_startup();

                config_startup(i);
                /* parse/load user options */
                if (parse_load_rc(&doc, &node))
                    parse_tree(i, doc, node->xmlChildrenNode);
                /* we're done with parsing now, kill it */
                xmlFreeDoc(doc);
                parse_shutdown(i);
            }

            /* load the theme specified in the rc file */
            ob_rr_theme = RrThemeNew(ob_rr_inst, config_theme);
            if (ob_rr_theme == NULL)
                ob_exit_with_error("Unable to load a theme.");

            moveresize_startup(reconfigure);
            screen_startup(reconfigure);
            group_startup(reconfigure);
            client_startup(reconfigure);
            dock_startup(reconfigure);
            keyboard_startup(reconfigure);
            mouse_startup(reconfigure);
            menu_startup(reconfigure);

            if (!reconfigure) {
                /* get all the existing windows */
                client_manage_all();
            } else {
                GList *it;

                /* redecorate all existing windows */
                for (it = client_list; it; it = g_list_next(it)) {
                    ObClient *c = it->data;
                    frame_adjust_theme(c->frame);
                }
            }

            reconfigure = FALSE;

            state = OB_STATE_RUNNING;
            ob_main_loop_run(ob_main_loop);
            state = OB_STATE_EXITING;

            if (!reconfigure) {
                dock_remove_all();
                client_unmanage_all();
            }

            menu_shutdown(reconfigure);
            mouse_shutdown(reconfigure);
            keyboard_shutdown(reconfigure);
            dock_shutdown(reconfigure);
            client_shutdown(reconfigure);
            group_shutdown(reconfigure);
            screen_shutdown(reconfigure);
            focus_shutdown(reconfigure);
            moveresize_shutdown(reconfigure);
            sn_shutdown(reconfigure);
            window_shutdown(reconfigure);
            grab_shutdown(reconfigure);
            event_shutdown(reconfigure);
            config_shutdown();
        } while (reconfigure);
    }

    RrThemeFree(ob_rr_theme);
    RrInstanceFree(ob_rr_inst);

    session_shutdown();
    g_free(ob_sm_id);

    XCloseDisplay(ob_display);

    if (restart) {
        if (restart_path != NULL) {
            int argcp;
            char **argvp;
            GError *err = NULL;

            /* run other shit */
            if (g_shell_parse_argv(restart_path, &argcp, &argvp, &err)) {
                execvp(argvp[0], argvp);
                g_strfreev(argvp);
            } else {
                g_warning("failed to execute '%s': %s", restart_path,
                          err->message);
            }
        }

        /* re-run me */
        execvp(argv[0], argv); /* try how we were run */
    }
     
    return 0;
}

static void signal_handler(int signal, gpointer data)
{
    if (signal == SIGUSR1) {
	fprintf(stderr, "Caught signal %d. Restarting.\n", signal);
        ob_restart();
    } else if (signal == SIGUSR2) {
	fprintf(stderr, "Caught signal %d. Reconfiguring.\n", signal);
        ob_reconfigure();
    } else {
	fprintf(stderr, "Caught signal %d. Exiting.\n", signal);
        ob_exit();
    }
}

static void print_version()
{
    g_print("Openbox %s\n\n", PACKAGE_VERSION);
    g_print("This program comes with ABSOLUTELY NO WARRANTY.\n");
    g_print("This is free software, and you are welcome to redistribute it\n");
    g_print("under certain conditions. See the file COPYING for details.\n\n");
}

static void print_help()
{
    print_version();
    g_print("Syntax: openbox [options]\n\n");
    g_print("Options:\n\n");
#ifdef USE_SM
    g_print("  --sm-disable        Disable connection to session manager\n");
    g_print("  --sm-client-id ID   Specify session management ID\n");
    g_print("  --sm-save-file FILE Specify file to load a saved session\n"
            "                      from\n");
#endif
    g_print("  --replace           Replace the currently running window "
            "manager\n");
    g_print("  --help              Display this help and exit\n");
    g_print("  --version           Display the version and exit\n");
    g_print("  --sync              Run in synchronous mode (this is slow and\n"
            "                      meant for debugging X routines)\n");
    g_print("  --debug             Display debugging output\n");
    g_print("\nPlease report bugs at %s\n", PACKAGE_BUGREPORT);
}

static void parse_args(int argc, char **argv)
{
    int i;

    for (i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "--version")) {
            print_version();
            exit(0);
        } else if (!strcmp(argv[i], "--help")) {
            print_help();
            exit(0);
        } else if (!strcmp(argv[i], "--g-fatal-warnings")) {
            g_log_set_always_fatal(G_LOG_LEVEL_CRITICAL);
        } else if (!strcmp(argv[i], "--replace")) {
            ob_replace_wm = TRUE;
        } else if (!strcmp(argv[i], "--sync")) {
            xsync = TRUE;
        } else if (!strcmp(argv[i], "--debug")) {
            ob_debug_show_output(TRUE);
#ifdef USE_SM
        } else if (!strcmp(argv[i], "--sm-client-id")) {
            if (i == argc - 1) /* no args left */
                g_printerr(_("--sm-client-id requires an argument\n"));
            else
                ob_sm_id = g_strdup(argv[++i]);
        } else if (!strcmp(argv[i], "--sm-save-file")) {
            if (i == argc - 1) /* no args left */
                g_printerr(_("--sm-save-file requires an argument\n"));
            else
                sm_save_file = argv[++i];
        } else if (!strcmp(argv[i], "--sm-disable")) {
            ob_sm_use = FALSE;
#endif
        } else {
            g_printerr("Invalid option: '%s'\n\n", argv[i]);
            print_help();
            exit(1);
        }
    }
}

void ob_exit_with_error(gchar *msg)
{
    g_critical(msg);
    session_shutdown();
    exit(EXIT_FAILURE);
}

void ob_restart_other(const gchar *path)
{
    restart_path = g_strdup(path);
    ob_restart();
}

void ob_restart()
{
    restart = TRUE;
    ob_exit();
}

void ob_exit()
{
    ob_main_loop_exit(ob_main_loop);
}

Cursor ob_cursor(ObCursor cursor)
{
    g_assert(cursor < OB_NUM_CURSORS);
    return cursors[cursor];
}

KeyCode ob_keycode(ObKey key)
{
    g_assert(key < OB_NUM_KEYS);
    return keys[key];
}

ObState ob_state()
{
    return state;
}

gchar *ob_expand_tilde(const gchar *f)
{
    gchar **spl;
    gchar *ret, *mid;

    if (!f)
        return NULL;
    spl = g_strsplit(f, "~", 0);
    mid = g_strconcat(g_get_home_dir(), G_DIR_SEPARATOR_S, NULL);
    ret = g_strjoinv(mid, spl);
    g_free(mid);
    g_strfreev(spl);
    return ret;
}

void ob_reconfigure()
{
    reconfigure = TRUE;
    ob_exit();
}
