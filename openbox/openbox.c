#include "debug.h"
#include "openbox.h"
#include "session.h"
#include "dock.h"
#include "event.h"
#include "menu.h"
#include "client.h"
#include "dispatch.h"
#include "xerror.h"
#include "prop.h"
#include "startup.h"
#include "screen.h"
#include "focus.h"
#include "moveresize.h"
#include "frame.h"
#include "extensions.h"
#include "grab.h"
#include "plugin.h"
#include "timer.h"
#include "group.h"
#include "config.h"
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
Display    *ob_display;
gint        ob_screen;
gboolean    ob_sm_use = TRUE;
gchar      *ob_sm_id;
gboolean    ob_replace_wm;

static ObState   state;
static gboolean  xsync;
static gboolean  shutdown;
static gboolean  restart;
static char     *restart_path;
static Cursor    cursors[OB_NUM_CURSORS];
static KeyCode   keys[OB_NUM_KEYS];
static gchar    *sm_save_file;

static void signal_handler(const ObEvent *e, void *data);
static void parse_args(int argc, char **argv);

int main(int argc, char **argv)
{
    struct sigaction action;
    sigset_t sigset;
    char *path;
    xmlDocPtr doc;
    xmlNodePtr node;

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

    /* start our event dispatcher and register for signals */
    dispatch_startup();
    dispatch_register(Event_Signal, signal_handler, NULL);

    /* set up signal handler */
    sigemptyset(&sigset);
    action.sa_handler = dispatch_signal;
    action.sa_mask = sigset;
    action.sa_flags = SA_NOCLDSTOP | SA_NODEFER;
    sigaction(SIGUSR1, &action, (struct sigaction *) NULL);
    sigaction(SIGPIPE, &action, (struct sigaction *) NULL);
/*    sigaction(SIGSEGV, &action, (struct sigaction *) NULL);*/
    sigaction(SIGFPE, &action, (struct sigaction *) NULL);
    sigaction(SIGTERM, &action, (struct sigaction *) NULL);
    sigaction(SIGINT, &action, (struct sigaction *) NULL);
    sigaction(SIGHUP, &action, (struct sigaction *) NULL);

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

    if (sm_save_file)
        session_load(sm_save_file);
    session_startup(argc, argv);

#ifdef USE_LIBSN
    ob_sn_display = sn_display_new(ob_display, NULL, NULL);
#endif

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

    /* save stuff that we can use to restore state */
    startup_save();

    if (screen_annex()) { /* it will be ours! */
        /* startup the parsing so everything can register sections of the rc */
        parse_startup();

        /* anything that is going to read data from the rc file needs to be 
           in this group */
	timer_startup();
	event_startup();
        grab_startup();
        /* focus_backup is used for stacking, so this needs to come before
           anything that calls stacking_add */
	focus_startup();
        window_startup();
        plugin_startup();
        /* load the plugins specified in the pluginrc */
        plugin_loadall();

        /* set up the kernel config shit */
        config_startup();
        menu_startup();
        /* parse/load user options */
        if (parse_load_rc(&doc, &node))
            parse_tree(doc, node->xmlChildrenNode, NULL);
        /* we're done with parsing now, kill it */
        parse_shutdown();

        /* load the theme specified in the rc file */
        ob_rr_theme = RrThemeNew(ob_rr_inst, config_theme);
        if (ob_rr_theme == NULL)
            ob_exit_with_error("Unable to load a theme.");

        moveresize_startup();
	screen_startup();
        group_startup();
	client_startup();
        dock_startup();

        /* call startup for all the plugins */
        plugin_startall();

	/* get all the existing windows */
	client_manage_all();

	state = OB_STATE_RUNNING;
	while (!shutdown)
	    event_loop();
	state = OB_STATE_EXITING;

        dock_remove_all();
	client_unmanage_all();

        plugin_shutdown(); /* calls all the plugins' shutdown functions */
        dock_shutdown();
	client_shutdown();
        group_shutdown();
	screen_shutdown();
	focus_shutdown();
        moveresize_shutdown();
        menu_shutdown();
        window_shutdown();
        grab_shutdown();
	event_shutdown();
	timer_shutdown();
        config_shutdown();
    }

    dispatch_shutdown();

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

static void signal_handler(const ObEvent *e, void *data)
{
    int s;

    s = e->data.s.signal;
    switch (s) {
    case SIGUSR1:
	fprintf(stderr, "Caught SIGUSR1 signal. Restarting.");
        ob_restart();
	break;

    case SIGHUP:
    case SIGINT:
    case SIGTERM:
    case SIGPIPE:
	fprintf(stderr, "Caught signal %d. Exiting.", s);
        ob_exit();
	break;

    case SIGFPE:
    case SIGSEGV:
        fprintf(stderr, "Caught signal %d. Aborting and dumping core.", s);
        abort();
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
    shutdown = TRUE;
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
