#include "openbox.h"
#include "event.h"
#include "client.h"
#include "dispatch.h"
#include "xerror.h"
#include "prop.h"
#include "screen.h"
#include "focus.h"
#include "extensions.h"
#include "gettext.h"
#include "config.h"
#include "grab.h"
#include "engine.h"
#include "themerc.h"
#include "plugin.h"
#include "timer.h"
#include "../render/render.h"
#include "../render/font.h"

#ifdef HAVE_FCNTL_H
#  include <fcntl.h>
#endif
#ifdef HAVE_SIGNAL_H
#  include <signal.h>
#endif
#ifdef HAVE_STDLIB_H
#  include <stdlib.h>
#endif
#ifdef HAVE_SYS_WAIT_H
#  include <sys/types.h>
#  include <sys/wait.h>
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

Display *ob_display  = NULL;
int      ob_screen;
Window   ob_root;
State    ob_state;
gboolean ob_shutdown = FALSE;
gboolean ob_restart  = FALSE;
char    *ob_restart_path = NULL;
gboolean ob_remote   = FALSE;
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

    ob_state = State_Starting;

    /* initialize the locale */
    if (!setlocale(LC_ALL, ""))
	g_warning("Couldn't set locale from environment.\n");
    bindtextdomain(PACKAGE, LOCALEDIR);
    bind_textdomain_codeset(PACKAGE, "UTF-8");
    textdomain(PACKAGE);

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
    sigaction(SIGCHLD, &action, (struct sigaction *) NULL);

    /* anything that died while we were restarting won't give us a SIGCHLD */
    while (waitpid(-1, NULL, WNOHANG) > 0);

    /* create the ~/.openbox dir */
    path = g_build_filename(g_get_home_dir(), ".openbox", NULL);
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

    /* set the DISPLAY environment variable for any lauched children, to the
       display we're using, so they open in the right place. */
    putenv(g_strdup_printf("DISPLAY=%s", DisplayString(ob_display)));

    ob_cursors.left_ptr = XCreateFontCursor(ob_display, XC_left_ptr);
    ob_cursors.ll_angle = XCreateFontCursor(ob_display, XC_ll_angle);
    ob_cursors.lr_angle = XCreateFontCursor(ob_display, XC_lr_angle);

    prop_startup(); /* get atoms values for the display */
    extensions_query_all(); /* find which extensions are present */
     
    if (screen_annex()) { /* it will be ours! */
	timer_startup();
        config_startup();
	render_startup();
	font_startup();
	themerc_startup();
	engine_startup();
	event_startup();
	screen_startup();
	focus_startup();
	client_startup();
        grab_startup();
        plugin_startup();

        /* XXX load all plugins!! */
        plugin_open("focus");
        plugin_open("keyboard");
        plugin_open("mouse");
        plugin_open("placement");
        plugin_open("resistance");

        config_parse();

	/* get all the existing windows */
	client_manage_all();

	ob_state = State_Running;
	while (!ob_shutdown)
	    event_loop();
	ob_state = State_Exiting;

	client_unmanage_all();

        plugin_shutdown(); /* calls all the plugins' shutdown functions */
        grab_shutdown();
	client_shutdown();
	focus_shutdown();
	screen_shutdown();
	event_shutdown();
	engine_shutdown();
	themerc_shutdown();
	render_shutdown();
        config_shutdown();
	timer_shutdown();
    }

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

    case SIGCHLD:
	wait(NULL);
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
	g_error("Caught signal %d. Aborting and dumping core.", s);
    }
}

void print_version()
{
    g_print("Openbox %s\n\n", VERSION);
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
    g_print("\nPlease report bugs at %s\n", BUGURL);
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
