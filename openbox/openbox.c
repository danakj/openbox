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
#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif
#ifdef HAVE_SYS_STAT_H
#  include <sys/stat.h>
#  include <sys/types.h>
#endif

#ifdef USE_SM
#include <X11/SM/SMlib.h>
#endif

#include <X11/cursorfont.h>

#ifdef USE_SM
gboolean    ob_sm_use = TRUE;
SmcConn     ob_sm_conn;
gchar      *ob_sm_id = NULL;
#endif

RrInstance *ob_rr_inst = NULL;
RrTheme    *ob_rr_theme = NULL;
Display    *ob_display = NULL;
int         ob_screen;
Window      ob_root;
ObState     ob_state;
gboolean    ob_shutdown = FALSE;
gboolean    ob_restart  = FALSE;
char       *ob_restart_path = NULL;
gboolean    ob_remote   = TRUE;
gboolean    ob_sync     = FALSE;
Cursor      ob_cursors[OB_NUM_CURSORS];
KeyCode     ob_keys[OB_NUM_KEYS];
char       *ob_rc_path  = NULL;

static void signal_handler(const ObEvent *e, void *data);
static void parse_args(int argc, char **argv);

static void sm_startup(int argc, char **argv);
static void sm_shutdown();

#ifdef USE_SM
static void sm_save_yourself(SmcConn conn, SmPointer data, int save_type,
                             Bool shutdown, int interact_style, Bool fast);
static void sm_die(SmcConn conn, SmPointer data);
static void sm_save_complete(SmcConn conn, SmPointer data);
static void sm_shutdown_cancelled(SmcConn conn, SmPointer data);
#endif
static void exit_with_error(gchar *msg);

int main(int argc, char **argv)
{
    struct sigaction action;
    sigset_t sigset;
    char *path;
    xmlDocPtr doc;
    xmlNodePtr node;

    ob_state = OB_STATE_STARTING;

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

    g_set_prgname(argv[0]);
     
    /* parse out command line args */
    parse_args(argc, argv);

    ob_display = XOpenDisplay(NULL);
    if (ob_display == NULL)
	exit_with_error("Failed to open the display.");
    if (fcntl(ConnectionNumber(ob_display), F_SETFD, 1) == -1)
        exit_with_error("Failed to set display as close-on-exec.");

    sm_startup(argc, argv);

#ifdef USE_LIBSN
    ob_sn_display = sn_display_new(ob_display, NULL, NULL);
#endif

    ob_screen = DefaultScreen(ob_display);
    ob_root = RootWindow(ob_display, ob_screen);

    ob_rr_inst = RrInstanceNew(ob_display, ob_screen);
    if (ob_rr_inst == NULL)
        exit_with_error("Failed to initialize the render library.");

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

    /* create available cursors */
    ob_cursors[OB_CURSOR_POINTER] =
        XCreateFontCursor(ob_display, XC_left_ptr);
    ob_cursors[OB_CURSOR_BUSY] =
        XCreateFontCursor(ob_display, XC_watch);
    ob_cursors[OB_CURSOR_MOVE] =
        XCreateFontCursor(ob_display, XC_fleur);
    ob_cursors[OB_CURSOR_NORTH] =
        XCreateFontCursor(ob_display, XC_top_side);
    ob_cursors[OB_CURSOR_NORTHEAST] =
        XCreateFontCursor(ob_display, XC_top_right_corner);
    ob_cursors[OB_CURSOR_EAST] =
        XCreateFontCursor(ob_display, XC_right_side);
    ob_cursors[OB_CURSOR_SOUTHEAST] =
        XCreateFontCursor(ob_display, XC_bottom_right_corner);
    ob_cursors[OB_CURSOR_SOUTH] =
        XCreateFontCursor(ob_display, XC_bottom_side);
    ob_cursors[OB_CURSOR_SOUTHWEST] =
        XCreateFontCursor(ob_display, XC_bottom_left_corner);
    ob_cursors[OB_CURSOR_WEST] =
        XCreateFontCursor(ob_display, XC_left_side);
    ob_cursors[OB_CURSOR_NORTHWEST] =
        XCreateFontCursor(ob_display, XC_top_left_corner);

    /* create available keycodes */
    ob_keys[OB_KEY_RETURN] =
        XKeysymToKeycode(ob_display, XStringToKeysym("Return"));
    ob_keys[OB_KEY_ESCAPE] =
        XKeysymToKeycode(ob_display, XStringToKeysym("Escape"));
    ob_keys[OB_KEY_LEFT] =
        XKeysymToKeycode(ob_display, XStringToKeysym("Left"));
    ob_keys[OB_KEY_RIGHT] =
        XKeysymToKeycode(ob_display, XStringToKeysym("Right"));
    ob_keys[OB_KEY_UP] =
        XKeysymToKeycode(ob_display, XStringToKeysym("Up"));
    ob_keys[OB_KEY_DOWN] =
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
            exit_with_error("Unable to load a theme.");

        frame_startup();
        moveresize_startup();
	screen_startup();
        group_startup();
	client_startup();
        dock_startup();

        /* call startup for all the plugins */
        plugin_startall();

	/* get all the existing windows */
	client_manage_all();

	ob_state = OB_STATE_RUNNING;
	while (!ob_shutdown)
	    event_loop();
	ob_state = OB_STATE_EXITING;

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

    dispatch_shutdown();

    RrThemeFree(ob_rr_theme);
    RrInstanceFree(ob_rr_inst);

    sm_shutdown();

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
    }
     
    return 0;
}

static void sm_startup(int argc, char **argv)
{
#ifdef USE_SM

#define SM_ERR_LEN 1024

    SmcCallbacks cb;
    char sm_err[SM_ERR_LEN];

    cb.save_yourself.callback = sm_save_yourself;
    cb.save_yourself.client_data = NULL;

    cb.die.callback = sm_die;
    cb.die.client_data = NULL;

    cb.save_complete.callback = sm_save_complete;
    cb.save_complete.client_data = NULL;

    cb.shutdown_cancelled.callback = sm_shutdown_cancelled;
    cb.shutdown_cancelled.client_data = NULL;

    ob_sm_conn = SmcOpenConnection(NULL, NULL, 1, 0,
                                   SmcSaveYourselfProcMask |
                                   SmcDieProcMask |
                                   SmcSaveCompleteProcMask |
                                   SmcShutdownCancelledProcMask,
                                   &cb, ob_sm_id, &ob_sm_id,
                                   SM_ERR_LEN, sm_err);
    if (ob_sm_conn == NULL)
        g_warning("Failed to connect to session manager: %s", sm_err);
    else {
        SmPropValue val_prog;
        SmPropValue val_uid;
        SmPropValue val_hint; 
        SmPropValue val_pri;
        SmPropValue val_pid;
        SmProp prop_cmd = { SmCloneCommand, SmLISTofARRAY8, 1, };
        SmProp prop_res = { SmRestartCommand, SmLISTofARRAY8, };
        SmProp prop_prog = { SmProgram, SmARRAY8, 1, };
        SmProp prop_uid = { SmUserID, SmARRAY8, 1, };
        SmProp prop_hint = { SmRestartStyleHint, SmCARD8, 1, };
        SmProp prop_pid = { SmProcessID, SmARRAY8, 1, };
        SmProp prop_pri = { "_GSM_Priority", SmCARD8, 1, };
        SmProp *props[7];
        gulong hint, pri;
        gchar pid[32];
        gint i, j;
        gboolean has_id;

        for (i = 1; i < argc - 1; ++i)
            if (strcmp(argv[i], "--sm-client-id") == 0)
                break;
        has_id = (i < argc - 1);

        prop_cmd.vals = g_new(SmPropValue, (has_id ? argc-2 : argc));
        prop_cmd.num_vals = (has_id ? argc-2 : argc);
        for (i = 0, j = 0; i < argc; ++i, ++j) {
            if (strcmp (argv[i], "--sm-client-id") == 0) {
                ++i, --j; /* skip the next as well, keep j where it is */
            } else {
                prop_cmd.vals[j].value = argv[i];
                prop_cmd.vals[j].length = strlen(argv[i]);
            }
        }

        prop_res.vals = g_new(SmPropValue, (has_id ? argc : argc+2));
        prop_res.num_vals = (has_id ? argc : argc+2);
        for (i = 0, j = 0; i < argc; ++i, ++j) { 
            if (strcmp (argv[i], "--sm-client-id") == 0) {
                ++i, --j; /* skip the next as well, keep j where it is */
            } else {
                prop_res.vals[j].value = argv[i];
                prop_res.vals[j].length = strlen(argv[i]);
            }
        }
        prop_res.vals[j].value = "--sm-client-id";
        prop_res.vals[j++].length = strlen("--sm-client-id");
        prop_res.vals[j].value = ob_sm_id;
        prop_res.vals[j++].length = strlen(ob_sm_id);

        val_prog.value = argv[0];
        val_prog.length = strlen(argv[0]);

        val_uid.value = g_strdup(g_get_user_name());
        val_uid.length = strlen(val_uid.value);

        hint = SmRestartImmediately;
        val_hint.value = &hint;
        val_hint.length = 1;

        sprintf(pid, "%ld", (long)getpid());
        val_pid.value = pid;
        val_pid.length = strlen(pid);

        /* priority with gnome-session-manager, low to run before other apps */
        pri = 20;
        val_pri.value = &pri;
        val_pri.length = 1;

        prop_prog.vals = &val_prog;
        prop_uid.vals = &val_uid;
        prop_hint.vals = &val_hint;
        prop_pid.vals = &val_pid;
        prop_pri.vals = &val_pri;

        props[0] = &prop_prog;
        props[1] = &prop_cmd;
        props[2] = &prop_res;
        props[3] = &prop_uid;
        props[4] = &prop_hint;
        props[5] = &prop_pid;
        props[6] = &prop_pri;

        SmcSetProperties(ob_sm_conn, 7, props);

        g_free(val_uid.value);
        g_free(prop_cmd.vals);
        g_free(prop_res.vals);

        g_message("Connected to session manager with id %s", ob_sm_id);
    }
    g_free (ob_sm_id);
#endif
}

static void sm_shutdown()
{
#ifdef USE_SM
    if (ob_sm_conn) {
        SmPropValue val_hint;
        SmProp prop_hint = { SmRestartStyleHint, SmCARD8, 1, };
        SmProp *props[1];
        gulong hint;

        /* when we exit, we want to reset this to a more friendly state */
        hint = SmRestartIfRunning;
        val_hint.value = &hint;
        val_hint.length = 1;

        prop_hint.vals = &val_hint;

        props[0] = &prop_hint;

        SmcSetProperties(ob_sm_conn, 1, props);

        SmcCloseConnection(ob_sm_conn, 0, NULL);
    }
#endif
}

static void signal_handler(const ObEvent *e, void *data)
{
    int s;

    s = e->data.s.signal;
    switch (s) {
    case SIGUSR1:
	fprintf(stderr, "Caught SIGUSR1 signal. Restarting.");
	ob_shutdown = ob_restart = TRUE;
	break;

    case SIGHUP:
    case SIGINT:
    case SIGTERM:
    case SIGPIPE:
	fprintf(stderr, "Caught signal %d. Exiting.", s);
	ob_shutdown = TRUE;
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
    g_print("  --rc PATH          Specify the path to the rc file to use\n");
#ifdef USE_SM
    g_print("  --sm-client-id ID  Specify session management ID\n");
    g_print("  --sm-disable       Disable connection to session manager\n");
#endif
    g_print("  --help             Display this help and exit\n");
    g_print("  --version          Display the version and exit\n");
    g_print("  --sync             Run in synchronous mode (this is slow and\n"
            "                     meant for debugging X routines)\n");
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
        } else if (!strcmp(argv[i], "--sync")) {
            ob_sync = TRUE;
        } else if (!strcmp(argv[i], "--rc")) {
            if (i == argc - 1) /* no args left */
                g_printerr(_("--rc requires an argument\n"));
            else
                ob_rc_path = argv[++i];
#ifdef USE_SM
        } else if (!strcmp(argv[i], "--sm-client-id")) {
            if (i == argc - 1) /* no args left */
                g_printerr(_("--sm-client-id requires an argument\n"));
            else
                ob_sm_id = argv[++i];
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

gboolean ob_pointer_pos(int *x, int *y)
{
    Window w;
    int i;
    guint u;

    return !!XQueryPointer(ob_display, ob_root, &w, &w, x, y, &i, &i, &u);
}

#ifdef USE_SM
static void sm_save_yourself(SmcConn conn, SmPointer data, int save_type,
                             Bool shutdown, int interact_style, Bool fast)
{
    g_message("got SAVE YOURSELF from session manager");
    SmcSaveYourselfDone(conn, TRUE);
}

static void sm_die(SmcConn conn, SmPointer data)
{
    ob_shutdown = TRUE;
    g_message("got DIE from session manager");
}

static void sm_save_complete(SmcConn conn, SmPointer data)
{
    g_message("got SAVE COMPLETE from session manager");
}

static void sm_shutdown_cancelled(SmcConn conn, SmPointer data)
{
    g_message("got SHUTDOWN CANCELLED from session manager");
}
#endif

static void exit_with_error(gchar *msg)
{
    g_critical(msg);
    sm_shutdown();
    exit(EXIT_FAILURE);
}

Cursor ob_cursor(ObCursor cursor)
{
    g_assert(cursor < OB_NUM_CURSORS);
    return ob_cursors[cursor];
}

KeyCode ob_keycode(ObKey key)
{
    g_assert(key < OB_NUM_KEYS);
    return ob_keys[key];
}
