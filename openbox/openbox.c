#include "openbox.h"
#include "event.h"
#include "client.h"
#include "xerror.h"
#include "prop.h"
#include "screen.h"
#include "focus.h"
#include "extensions.h"
#include "gettext.h"
#include "keyboard.h"
#include "pointer.h"
#include "engine.h"
#include "python.h"
#include "hooks.h"
#include "clientwrap.h"
#include "openboxwrap.h"
#include "themerc.h"
#include "timer.h"
#include "../render/render.h"

#ifdef HAVE_FCNTL_H
#  include <fcntl.h>
#endif
#ifdef HAVE_SYS_SELECT_H
#  include <sys/select.h>
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

#include <X11/cursorfont.h>

Display *ob_display  = NULL;
int      ob_screen;
Window   ob_root;
State    ob_state;
gboolean ob_shutdown = FALSE;
gboolean ob_restart  = FALSE;
char    *ob_restart_path = NULL;
gboolean ob_remote   = FALSE;
gboolean ob_sync     = TRUE;
Cursors  ob_cursors;

void signal_handler(int signal);

int main(int argc, char **argv)
{
    struct sigaction action;
    sigset_t sigset;

    ob_state = State_Starting;

    /* initialize the locale */
    if (!setlocale(LC_ALL, ""))
	g_warning("Couldn't set locale from environment.\n");
    bindtextdomain(PACKAGE, LOCALEDIR);
    bind_textdomain_codeset(PACKAGE, "UTF-8");
    textdomain(PACKAGE);

    /* set up signal handler */
    sigemptyset(&sigset);
    action.sa_handler = signal_handler;
    action.sa_mask = sigset;
    action.sa_flags = SA_NOCLDSTOP | SA_NODEFER;
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
     
    /* XXX parse out command line args */
    (void)argc;(void)argv;

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
	render_startup();
	themerc_startup();
	engine_startup(themerc_engine);
	python_startup();
	openboxwrap_startup();
	clientwrap_startup();
	hooks_startup();
	event_startup();
	screen_startup();
	focus_startup();
	client_startup();
	keyboard_startup();
	pointer_startup();
	  
	/* load the user's settings */
	if (!python_import("rc"))
	    g_warning("ERROR LOADING RC FILE");

	HOOKFIRE(startup, "()");

	/* get all the existing windows */
	client_manage_all();

	ob_state = State_Running;
	while (!ob_shutdown) {
	    event_loop();
	}
	ob_state = State_Exiting;

	client_unmanage_all();

	HOOKFIRE(shutdown, "()");

	pointer_shutdown();
	keyboard_shutdown();
	client_shutdown();
	screen_shutdown();
	event_shutdown();
	hooks_shutdown();
	clientwrap_shutdown();
	openboxwrap_shutdown();
	python_shutdown();
	engine_shutdown();
	themerc_shutdown();
	render_shutdown();
	timer_shutdown();
    }
	  
    XCloseDisplay(ob_display);

    /* XXX if (ob_restart) */
     
    return 0;
}

void signal_handler(int signal)
{
    switch (signal) {
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
	g_message("Caught signal %d. Exiting.", signal);
	ob_shutdown = TRUE;
	break;

    case SIGFPE:
    case SIGSEGV:
	g_error("Caught signal %d. Aborting and dumping core.", signal);
    }
}
