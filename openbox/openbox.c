#include "openbox.h"
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
#include "render2/render.h"
#include "render2/theme.h"

#ifdef HAVE_FCNTL_H
#  include <fcntl.h>
#endif
#ifdef HAVE_SIGNAL_H
#  include <signal.h>
#endif
#ifdef HAVE_STDLIB_H
#  include <stdlib.h>
#endif
#ifdef HAVE_LOCALE_H
#  include <locale.h>
#endif
#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif
#ifdef HAVE_SYS_STAT_H
#  include <sys/stat.h>
#  include <sys/types.h>
#endif

#include <X11/cursorfont.h>

struct RrInstance *ob_render_inst;
struct RrTheme    *ob_theme;

Display *ob_display  = NULL;
int      ob_screen;
Window   ob_root;
State    ob_state;
gboolean ob_shutdown = FALSE;
gboolean ob_restart  = FALSE;
char    *ob_restart_path = NULL;
gboolean ob_remote   = TRUE;
gboolean ob_sync     = FALSE;
Cursors  ob_cursors;
char    *ob_rc_path  = NULL;

void signal_handler(const ObEvent *e, void *data);
void parse_args(int argc, char **argv);

int main(int argc, char **argv)
{
    struct sigaction action;
    sigset_t sigset;
    char *path;
    xmlDocPtr doc;
    xmlNodePtr node;

    ob_state = State_Starting;

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
    action.sa_flags = SA_NOCLDSTOP;
    sigaction(SIGUSR1, &action, (struct sigaction *) NULL);
    sigaction(SIGPIPE, &action, (struct sigaction *) NULL);
    sigaction(SIGSEGV, &action, (struct sigaction *) NULL);
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
     
    /* parse out command line args */
    parse_args(argc, argv);

    ob_display = XOpenDisplay(NULL);
    if (ob_display == NULL) {
	/* print a message and exit */
	g_critical("Failed to open the display.");
	exit(1);
    }
    if (fcntl(ConnectionNumber(ob_display), F_SETFD, 1) == -1) {
	/* print a message and exit */
	g_critical("Failed to set display as close-on-exec.");
	exit(1);
    }

#ifdef USE_LIBSN
    ob_sn_display = sn_display_new(ob_display, NULL, NULL);
#endif

    ob_screen = DefaultScreen(ob_display);
    ob_root = RootWindow(ob_display, ob_screen);

    /* XXX fork self onto other screens */
     
    XSynchronize(ob_display, ob_sync);

    /* check for locale support */
    if (!XSupportsLocale())
	g_warning("X server does not support locale.");
    if (!XSetLocaleModifiers(""))
	g_warning("Cannot set locale modifiers for the X server.");

    /* set our error handler */
    XSetErrorHandler(xerror_handler);

    ob_render_inst = RrInstanceNew(ob_display, ob_screen);
    if (!ob_render_inst) {
        g_critical("Unable to initialize GL rendering subsystem.");
        exit(1);
    }

    /* set the DISPLAY environment variable for any lauched children, to the
       display we're using, so they open in the right place. */
    putenv(g_strdup_printf("DISPLAY=%s", DisplayString(ob_display)));

    ob_cursors.ptr = XCreateFontCursor(ob_display, XC_left_ptr);
    ob_cursors.busy = XCreateFontCursor(ob_display, XC_watch);
    ob_cursors.move = XCreateFontCursor(ob_display, XC_fleur);
    ob_cursors.tl = XCreateFontCursor(ob_display, XC_top_left_corner);
    ob_cursors.tr = XCreateFontCursor(ob_display, XC_top_right_corner);
    ob_cursors.bl = XCreateFontCursor(ob_display, XC_bottom_left_corner);
    ob_cursors.br = XCreateFontCursor(ob_display, XC_bottom_right_corner);
    ob_cursors.t = XCreateFontCursor(ob_display, XC_top_side);
    ob_cursors.r = XCreateFontCursor(ob_display, XC_right_side);
    ob_cursors.b = XCreateFontCursor(ob_display, XC_bottom_side);
    ob_cursors.l = XCreateFontCursor(ob_display, XC_left_side);

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
        plugin_startup();
        /* load the plugins specified in the pluginrc */
        plugin_loadall();

        /* set up the kernel config shit */
        config_startup();
        /* parse/load user options */
        if (parse_load_rc(&doc, &node))
            parse_tree(doc, node->xmlChildrenNode, NULL);
        /* we're done with parsing now, kill it */
        parse_shutdown();

        /* load the theme specified in the rc file */
        ob_theme = RrThemeLoad(ob_render_inst, config_theme);
        if (!ob_theme) {
            g_critical("Unable to load a theme.");
            exit(1);
        }

        window_startup();
        menu_startup();
        frame_startup();
        moveresize_startup();
	focus_startup();
	screen_startup();
        group_startup();
	client_startup();
        dock_startup();

        /* call startup for all the plugins */
        plugin_startall();

	/* get all the existing windows */
	client_manage_all();

	ob_state = State_Running;
	while (!ob_shutdown)
	    event_loop();
	ob_state = State_Exiting;

        dock_remove_all();
	client_unmanage_all();

        plugin_shutdown(); /* calls all the plugins' shutdown functions */
        dock_shutdown();
	client_shutdown();
        group_shutdown();
	screen_shutdown();
	focus_shutdown();
        moveresize_shutdown();
        frame_shutdown();
        menu_shutdown();
        window_shutdown();
        grab_shutdown();
	event_shutdown();
	timer_shutdown();
        config_shutdown();
    }

    RrInstanceFree(ob_render_inst);

    dispatch_shutdown();

    XCloseDisplay(ob_display);

    if (ob_restart) {
        if (ob_restart_path != NULL) {
            int argcp;
            char **argvp;
            GError *err = NULL;

            /* run other shit */
            if (g_shell_parse_argv(ob_restart_path, &argcp, &argvp, &err)) {
                execvp(argvp[0], argvp);
                g_strfreev(argvp);
            } else {
                g_warning("failed to execute '%s': %s", ob_restart_path,
                          err->message);
            }
        }

        /* re-run me */
        execvp(argv[0], argv); /* try how we were run */
        execlp(BINARY, BINARY, NULL); /* try this as a last resort */
    }
     
    return 0;
}

void signal_handler(const ObEvent *e, void *data)
{
    int s;

    s = e->data.s.signal;
    switch (s) {
    case SIGUSR1:
	g_message("Caught SIGUSR1 signal. Restarting.");
	ob_shutdown = ob_restart = TRUE;
	break;

    case SIGHUP:
    case SIGINT:
    case SIGTERM:
    case SIGPIPE:
	g_message("Caught signal %d. Exiting.", s);
	ob_shutdown = TRUE;
	break;

    case SIGFPE:
    case SIGSEGV:
	g_message("Caught signal %d. Aborting and dumping core.", s);
        abort();
    }
}

void print_version()
{
    g_print("Openbox %s\n\n", PACKAGE_VERSION);
    g_print("This program comes with ABSOLUTELY NO WARRANTY.\n");
    g_print("This is free software, and you are welcome to redistribute it\n");
    g_print("under certain conditions. See the file COPYING for details.\n\n");
}

void print_help()
{
    print_version();
    g_print("Syntax: %s [options]\n\n", BINARY);
    g_print("Options:\n\n");
    g_print("  -rc PATH     Specify the path to the rc file to use\n");
    g_print("  -help        Display this help and exit\n");
    g_print("  -version     Display the version and exit\n");
    g_print("  -sync        Run in synchronous mode (this is slow and meant\n"
            "               for debugging X routines)\n");
    g_print("\nPlease report bugs at %s\n", PACKAGE_BUGREPORT);
}

void parse_args(int argc, char **argv)
{
    int i;

    for (i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "-version")) {
            print_version();
            exit(0);
        } else if (!strcmp(argv[i], "-help")) {
            print_help();
            exit(0);
        } else if (!strcmp(argv[i], "-sync")) {
            ob_sync = TRUE;
        } else if (!strcmp(argv[i], "-rc")) {
            if (i == argc - 1) /* no args left */
                g_printerr("-rc requires an argument\n");
            else
                ob_rc_path = argv[++i];
        } else {
            g_printerr("Invalid option: '%s'\n\n", argv[i]);
            print_help();
            exit(1);
        }
    }
}

gboolean ob_pointer_pos(int *x, int *y)
{
    Window w;
    int i;
    guint u;

    return !!XQueryPointer(ob_display, ob_root, &w, &w, x, y, &i, &i, &u);
}
