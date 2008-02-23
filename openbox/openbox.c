/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   openbox.c for the Openbox window manager
   Copyright (c) 2006        Mikael Magnusson
   Copyright (c) 2003-2007   Dana Jansens

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   See the COPYING file for a copy of the GNU General Public License.
*/

#include "debug.h"
#include "openbox.h"
#include "session.h"
#include "dock.h"
#include "event.h"
#include "menu.h"
#include "client.h"
#include "screen.h"
#include "actions.h"
#include "startupnotify.h"
#include "focus.h"
#include "focus_cycle.h"
#include "focus_cycle_indicator.h"
#include "focus_cycle_popup.h"
#include "moveresize.h"
#include "engine_interface.h"
#include "keyboard.h"
#include "mouse.h"
#include "menuframe.h"
#include "grab.h"
#include "group.h"
#include "config.h"
#include "ping.h"
#include "prompt.h"
#include "gettext.h"
#include "render/render.h"
#include "render/theme.h"
#include "obt/display.h"
#include "obt/prop.h"
#include "obt/keyboard.h"
#include "obt/parse.h"

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
#ifdef HAVE_SYS_STAT_H
#  include <sys/stat.h>
#  include <sys/types.h>
#endif
#ifdef HAVE_SYS_WAIT_H
#  include <sys/types.h>
#  include <sys/wait.h>
#endif
#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif
#include <errno.h>

#include <X11/cursorfont.h>
#if USE_XCURSOR
#include <X11/Xcursor/Xcursor.h>
#endif

#include <X11/Xlib.h>
#include <X11/keysym.h>

RrInstance  *ob_rr_inst;
RrTheme     *ob_rr_theme;
ObtMainLoop *ob_main_loop;
gint         ob_screen;
gboolean     ob_replace_wm = FALSE;
gboolean     ob_sm_use = TRUE;
gchar       *ob_sm_id = NULL;
gchar       *ob_sm_save_file = NULL;
gboolean     ob_sm_restore = TRUE;
gboolean     ob_debug_xinerama = FALSE;
ObFrameEngine frame_engine;

static ObState   state;
static gboolean  xsync = FALSE;
static gboolean  reconfigure = FALSE;
static gboolean  restart = FALSE;
static gchar    *restart_path = NULL;
static Cursor    cursors[OB_NUM_CURSORS];
static KeyCode   keys[OB_NUM_KEYS];
static gint      exitcode = 0;
static guint     remote_control = 0;
static gboolean  being_replaced = FALSE;
static gchar    *config_file = NULL;

static void signal_handler(gint signal, gpointer data);
static void remove_args(gint *argc, gchar **argv, gint index, gint num);
static void parse_env();
static void parse_args(gint *argc, gchar **argv);
static Cursor load_cursor(const gchar *name, guint fontval);

gint main(gint argc, gchar **argv)
{
    gchar *program_name;

    state = OB_STATE_STARTING;

    ob_debug_startup();

    /* initialize the locale */
    if (!setlocale(LC_ALL, ""))
        g_message("Couldn't set locale from environment.");
    bindtextdomain(PACKAGE_NAME, LOCALEDIR);
    bind_textdomain_codeset(PACKAGE_NAME, "UTF-8");
    textdomain(PACKAGE_NAME);

    /* parse the command line args, which can change the argv[0] */
    parse_args(&argc, argv);
    /* parse the environment variables */
    parse_env();

    program_name = g_path_get_basename(argv[0]);
    g_set_prgname(program_name);

    if (!remote_control)
        session_startup(argc, argv);

    if (!obt_display_open(NULL))
        ob_exit_with_error(_("Failed to open the display from the DISPLAY environment variable."));

    if (remote_control) {
        /* Send client message telling the OB process to:
         * remote_control = 1 -> reconfigure
         * remote_control = 2 -> restart */
        OBT_PROP_MSG(ob_screen, obt_root(ob_screen),
                     OB_CONTROL, remote_control, 0, 0, 0, 0);
        obt_display_close(obt_display);
        exit(EXIT_SUCCESS);
    }

    ob_main_loop = obt_main_loop_new();

    /* set up signal handler */
    obt_main_loop_signal_add(ob_main_loop, SIGUSR1, signal_handler, NULL,NULL);
    obt_main_loop_signal_add(ob_main_loop, SIGUSR2, signal_handler, NULL,NULL);
    obt_main_loop_signal_add(ob_main_loop, SIGTERM, signal_handler, NULL,NULL);
    obt_main_loop_signal_add(ob_main_loop, SIGINT, signal_handler,  NULL,NULL);
    obt_main_loop_signal_add(ob_main_loop, SIGHUP, signal_handler,  NULL,NULL);
    obt_main_loop_signal_add(ob_main_loop, SIGPIPE, signal_handler, NULL,NULL);
    obt_main_loop_signal_add(ob_main_loop, SIGCHLD, signal_handler, NULL,NULL);

    ob_screen = DefaultScreen(obt_display);

    ob_rr_inst = RrInstanceNew(obt_display, ob_screen);
    if (ob_rr_inst == NULL)
        ob_exit_with_error(_("Failed to initialize the obrender library."));

    XSynchronize(obt_display, xsync);

    /* check for locale support */
    if (!XSupportsLocale())
        g_message(_("X server does not support locale."));
    if (!XSetLocaleModifiers(""))
        g_message(_("Cannot set locale modifiers for the X server."));

    /* set the DISPLAY environment variable for any lauched children, to the
       display we're using, so they open in the right place. */
    setenv("DISPLAY", DisplayString(obt_display), TRUE);

    /* create available cursors */
    cursors[OB_CURSOR_NONE] = None;
    cursors[OB_CURSOR_POINTER] = load_cursor("left_ptr", XC_left_ptr);
    cursors[OB_CURSOR_BUSYPOINTER] = load_cursor("left_ptr_watch",XC_left_ptr);
    cursors[OB_CURSOR_BUSY] = load_cursor("watch", XC_watch);
    cursors[OB_CURSOR_MOVE] = load_cursor("fleur", XC_fleur);
    cursors[OB_CURSOR_NORTH] = load_cursor("top_side", XC_top_side);
    cursors[OB_CURSOR_NORTHEAST] = load_cursor("top_right_corner",
                                               XC_top_right_corner);
    cursors[OB_CURSOR_EAST] = load_cursor("right_side", XC_right_side);
    cursors[OB_CURSOR_SOUTHEAST] = load_cursor("bottom_right_corner",
                                               XC_bottom_right_corner);
    cursors[OB_CURSOR_SOUTH] = load_cursor("bottom_side", XC_bottom_side);
    cursors[OB_CURSOR_SOUTHWEST] = load_cursor("bottom_left_corner",
                                               XC_bottom_left_corner);
    cursors[OB_CURSOR_WEST] = load_cursor("left_side", XC_left_side);
    cursors[OB_CURSOR_NORTHWEST] = load_cursor("top_left_corner",
                                               XC_top_left_corner);

    if (screen_annex()) { /* it will be ours! */
        do {
            if (reconfigure) obt_keyboard_reload();

            /* get the keycodes for keys we use */
            keys[OB_KEY_RETURN] = obt_keyboard_keysym_to_keycode(XK_Return);
            keys[OB_KEY_ESCAPE] = obt_keyboard_keysym_to_keycode(XK_Escape);
            keys[OB_KEY_LEFT] = obt_keyboard_keysym_to_keycode(XK_Left);
            keys[OB_KEY_RIGHT] = obt_keyboard_keysym_to_keycode(XK_Right);
            keys[OB_KEY_UP] = obt_keyboard_keysym_to_keycode(XK_Up);
            keys[OB_KEY_DOWN] = obt_keyboard_keysym_to_keycode(XK_Down);
            keys[OB_KEY_TAB] = obt_keyboard_keysym_to_keycode(XK_Tab);
            keys[OB_KEY_SPACE] = obt_keyboard_keysym_to_keycode(XK_space);

            {
                ObtParseInst *i;

                /* startup the parsing so everything can register sections
                   of the rc */
                i = obt_parse_instance_new();

                /* register all the available actions */
                actions_startup(reconfigure);
                /* start up config which sets up with the parser */
                config_startup(i);

                /* parse/load user options */
                if ((config_file &&
                     obt_parse_load_file(i, config_file, "openbox_config")) ||
                    obt_parse_load_config_file(i, "openbox", "rc.xml",
                                               "openbox_config"))
                {
                    obt_parse_tree_from_root(i);
                    obt_parse_close(i);
                }
                else {
                    g_message(_("Unable to find a valid config file, using some simple defaults"));
                    config_file = NULL;
                }

                if (config_file) {
                    gchar *p = g_filename_to_utf8(config_file, -1,
                                                  NULL, NULL, NULL);
                    if (p)
                        OBT_PROP_SETS(obt_root(ob_screen), OB_CONFIG_FILE,
                                      utf8, p);
                    g_free(p);
                }
                else
                    OBT_PROP_ERASE(obt_root(ob_screen), OB_CONFIG_FILE);

                /* we're done with parsing now, kill it */
                obt_parse_instance_unref(i);
            }

            /* load the theme specified in the rc file */
              {
                ob_debug("Entering LoadThemeConfig");
                init_frame_engine (&frame_engine,
                    config_theme, TRUE, config_font_activewindow,
                    config_font_inactivewindow, config_font_menutitle,
                    config_font_menuitem, config_font_osd);
                ob_debug("Exiting LoadThemeConfig");
                /* load the theme specified in the rc file */
                RrTheme *theme;
                if ((theme = RrThemeNew(ob_rr_inst, config_theme, TRUE,
                                        config_font_activewindow,
                                        config_font_inactivewindow,
                                        config_font_menutitle,
                                        config_font_menuitem,
                                        config_font_osd)))
                {
                    RrThemeFree(ob_rr_theme);
                    ob_rr_theme = theme;
                }
                if (ob_rr_theme == NULL)
                    ob_exit_with_error(_("Unable to load a theme."));

                OBT_PROP_SETS(obt_root(ob_screen),
                              OB_THEME, utf8, ob_rr_theme->name);
            }

            if (reconfigure) {
                GList *it;

                /* update all existing windows for the new theme */
                for (it = client_list; it; it = g_list_next(it)) {
                    ObClient *c = it->data;
                    frame_engine.frame_adjust_theme(c->frame);
                  }
              }
            event_startup(reconfigure);
            /* focus_backup is used for stacking, so this needs to come before
               anything that calls stacking_add */
            sn_startup(reconfigure);
            window_startup(reconfigure);
            focus_startup(reconfigure);
            focus_cycle_startup(reconfigure);
            focus_cycle_indicator_startup(reconfigure);
            focus_cycle_popup_startup(reconfigure);
            screen_startup(reconfigure);
            grab_startup(reconfigure);
            group_startup(reconfigure);
            ping_startup(reconfigure);
            prompt_startup(reconfigure);
            client_startup(reconfigure);
            dock_startup(reconfigure);
            moveresize_startup(reconfigure);
            keyboard_startup(reconfigure);
            mouse_startup(reconfigure);
            menu_frame_startup(reconfigure);
            menu_startup(reconfigure);

            if (!reconfigure) {
                guint32 xid;
                ObWindow *w;

                /* get all the existing windows */
                window_manage_all();
                focus_nothing();

                /* focus what was focused if a wm was already running */
                if (OBT_PROP_GET32(obt_root(ob_screen),
                                   NET_ACTIVE_WINDOW, WINDOW, &xid) &&
                    (w = window_find(xid)) && WINDOW_IS_CLIENT(w))
                {
                    client_focus(WINDOW_AS_CLIENT(w));
                }
            } else {
                GList *it;

                /* redecorate all existing windows */
                for (it = client_list; it; it = g_list_next(it)) {
                    ObClient *c = it->data;

                    /* the new config can change the window's decorations */
                    client_setup_decor_and_functions(c, FALSE);
                    /* redraw the frames */
                    frame_engine.frame_update_layout (c->frame, c->area, FALSE, FALSE);
                    /* if this occurs while we are focus cycling, the indicator needs to
                     match the changes */
                    if (focus_cycle_target == c)
                        focus_cycle_draw_indicator(c);
                    /* the decor sizes may have changed, so the windows may
                       end up in new positions */
                    client_reconfigure(c, FALSE);
                }
            }

            reconfigure = FALSE;

            state = OB_STATE_RUNNING;
            obt_main_loop_run(ob_main_loop);
            state = OB_STATE_EXITING;

            if (!reconfigure)
                window_unmanage_all();

            menu_shutdown(reconfigure);
            menu_frame_shutdown(reconfigure);
            mouse_shutdown(reconfigure);
            keyboard_shutdown(reconfigure);
            moveresize_shutdown(reconfigure);
            dock_shutdown(reconfigure);
            client_shutdown(reconfigure);
            prompt_shutdown(reconfigure);
            ping_shutdown(reconfigure);
            group_shutdown(reconfigure);
            grab_shutdown(reconfigure);
            screen_shutdown(reconfigure);
            focus_cycle_popup_shutdown(reconfigure);
            focus_cycle_indicator_shutdown(reconfigure);
            focus_cycle_shutdown(reconfigure);
            focus_shutdown(reconfigure);
            window_shutdown(reconfigure);
            sn_shutdown(reconfigure);
            event_shutdown(reconfigure);
            config_shutdown();
            actions_shutdown(reconfigure);
        } while (reconfigure);
    }

    XSync(obt_display, FALSE);

    session_shutdown(being_replaced);

    obt_display_close(obt_display);

    if (restart) {
        if (restart_path != NULL) {
            gint argcp;
            gchar **argvp;
            GError *err = NULL;

            /* run other window manager */
            if (g_shell_parse_argv(restart_path, &argcp, &argvp, &err)) {
                execvp(argvp[0], argvp);
                g_strfreev(argvp);
            } else {
                g_message(
                    _("Restart failed to execute new executable '%s': %s"),
                    restart_path, err->message);
                g_error_free(err);
            }
        }

        /* we remove the session arguments from argv, so put them back,
           also don't restore the session on restart */
        if (ob_sm_save_file != NULL || ob_sm_id != NULL) {
            gchar **nargv;
            gint i, l;

            l = argc + 1 +
                (ob_sm_save_file != NULL ? 2 : 0) +
                (ob_sm_id != NULL ? 2 : 0);
            nargv = g_new0(gchar*, l+1);
            for (i = 0; i < argc; ++i)
                nargv[i] = argv[i];

            if (ob_sm_save_file != NULL) {
                nargv[i++] = g_strdup("--sm-save-file");
                nargv[i++] = ob_sm_save_file;
            }
            if (ob_sm_id != NULL) {
                nargv[i++] = g_strdup("--sm-client-id");
                nargv[i++] = ob_sm_id;
            }
            nargv[i++] = g_strdup("--sm-no-load");
            g_assert(i == l);
            argv = nargv;
        }

        /* re-run me */
        execvp(argv[0], argv); /* try how we were run */
        execlp(argv[0], program_name, (gchar*)NULL); /* last resort */
    }

    /* free stuff passed in from the command line or environment */
    g_free(ob_sm_save_file);
    g_free(ob_sm_id);
    g_free(program_name);

    ob_debug_shutdown();

    return exitcode;
}

static void signal_handler(gint signal, gpointer data)
{
    switch (signal) {
    case SIGUSR1:
        ob_debug("Caught signal %d. Restarting.", signal);
        ob_restart();
        break;
    case SIGUSR2:
        ob_debug("Caught signal %d. Reconfiguring.", signal);
        ob_reconfigure();
        break;
    case SIGCHLD:
        /* reap children */
        while (waitpid(-1, NULL, WNOHANG) > 0);
        break;
    default:
        ob_debug("Caught signal %d. Exiting.", signal);
        /* TERM and INT return a 0 code */
        ob_exit(!(signal == SIGTERM || signal == SIGINT));
    }
}

static void print_version()
{
    g_print("Openbox %s\n", PACKAGE_VERSION);
    g_print(_("Copyright (c)"));
    g_print(" 2008        Mikael Magnusson\n");
    g_print(_("Copyright (c)"));
    g_print(" 2003-2006   Dana Jansens\n\n");
    g_print("This program comes with ABSOLUTELY NO WARRANTY.\n");
    g_print("This is free software, and you are welcome to redistribute it\n");
    g_print("under certain conditions. See the file COPYING for details.\n\n");
}

static void print_help()
{
    g_print(_("Syntax: openbox [options]\n"));
    g_print(_("\nOptions:\n"));
    g_print(_("  --help              Display this help and exit\n"));
    g_print(_("  --version           Display the version and exit\n"));
    g_print(_("  --replace           Replace the currently running window manager\n"));
    g_print(_("  --config-file FILE  Specify the path to the config file to use\n"));
    g_print(_("  --sm-disable        Disable connection to the session manager\n"));
    g_print(_("\nPassing messages to a running Openbox instance:\n"));
    g_print(_("  --reconfigure       Reload Openbox's configuration\n"));
    g_print(_("  --restart           Restart Openbox\n"));
    g_print(_("  --exit              Exit Openbox\n"));
    g_print(_("\nDebugging options:\n"));
    g_print(_("  --sync              Run in synchronous mode\n"));
    g_print(_("  --debug             Display debugging output\n"));
    g_print(_("  --debug-focus       Display debugging output for focus handling\n"));
    g_print(_("  --debug-session     Display debugging output for session managment\n"));
    g_print(_("  --debug-xinerama    Split the display into fake xinerama screens\n"));
    g_print(_("\nPlease report bugs at %s\n"), PACKAGE_BUGREPORT);
}

static void remove_args(gint *argc, gchar **argv, gint index, gint num)
{
    gint i;

    for (i = index; i < *argc - num; ++i)
        argv[i] = argv[i+num];
    for (; i < *argc; ++i)
        argv[i] = NULL;
    *argc -= num;
}

static void parse_env()
{
    /* unset this so we don't pass it on unknowingly */
    unsetenv("DESKTOP_STARTUP_ID");
}

static void parse_args(gint *argc, gchar **argv)
{
    gint i;

    for (i = 1; i < *argc; ++i) {
        if (!strcmp(argv[i], "--version")) {
            print_version();
            exit(0);
        }
        else if (!strcmp(argv[i], "--help")) {
            print_help();
            exit(0);
        }
        else if (!strcmp(argv[i], "--g-fatal-warnings")) {
            g_log_set_always_fatal(G_LOG_LEVEL_CRITICAL);
        }
        else if (!strcmp(argv[i], "--replace")) {
            ob_replace_wm = TRUE;
            remove_args(argc, argv, i, 1);
            --i; /* this arg was removed so go back */
        }
        else if (!strcmp(argv[i], "--sync")) {
            xsync = TRUE;
        }
        else if (!strcmp(argv[i], "--debug")) {
            ob_debug_enable(OB_DEBUG_NORMAL, TRUE);
            ob_debug_enable(OB_DEBUG_APP_BUGS, TRUE);
        }
        else if (!strcmp(argv[i], "--debug-focus")) {
            ob_debug_enable(OB_DEBUG_NORMAL, TRUE);
            ob_debug_enable(OB_DEBUG_APP_BUGS, TRUE);
            ob_debug_enable(OB_DEBUG_FOCUS, TRUE);
        }
        else if (!strcmp(argv[i], "--debug-session")) {
            ob_debug_enable(OB_DEBUG_NORMAL, TRUE);
            ob_debug_enable(OB_DEBUG_APP_BUGS, TRUE);
            ob_debug_enable(OB_DEBUG_SM, TRUE);
        }
        else if (!strcmp(argv[i], "--debug-xinerama")) {
            ob_debug_xinerama = TRUE;
        }
        else if (!strcmp(argv[i], "--reconfigure")) {
            remote_control = 1;
        }
        else if (!strcmp(argv[i], "--restart")) {
            remote_control = 2;
        }
        else if (!strcmp(argv[i], "--exit")) {
            remote_control = 3;
        }
        else if (!strcmp(argv[i], "--config-file")) {
            if (i == *argc - 1) /* no args left */
                g_printerr(_("--config-file requires an argument\n"));
            else {
                /* this will be in the current locale encoding, which is
                   what we want */
                config_file = argv[i+1];
                ++i; /* skip the argument */
                ob_debug("--config-file %s\n", config_file);
            }
        }
        else if (!strcmp(argv[i], "--sm-save-file")) {
            if (i == *argc - 1) /* no args left */
                /* not translated cuz it's sekret */
                g_printerr("--sm-save-file requires an argument\n");
            else {
                ob_sm_save_file = g_strdup(argv[i+1]);
                remove_args(argc, argv, i, 2);
                --i; /* this arg was removed so go back */
                ob_debug_type(OB_DEBUG_SM, "--sm-save-file %s",
                              ob_sm_save_file);
            }
        }
        else if (!strcmp(argv[i], "--sm-client-id")) {
            if (i == *argc - 1) /* no args left */
                /* not translated cuz it's sekret */
                g_printerr("--sm-client-id requires an argument\n");
            else {
                ob_sm_id = g_strdup(argv[i+1]);
                remove_args(argc, argv, i, 2);
                --i; /* this arg was removed so go back */
                ob_debug_type(OB_DEBUG_SM, "--sm-client-id %s", ob_sm_id);
            }
        }
        else if (!strcmp(argv[i], "--sm-disable")) {
            ob_sm_use = FALSE;
        }
        else if (!strcmp(argv[i], "--sm-no-load")) {
            ob_sm_restore = FALSE;
            remove_args(argc, argv, i, 1);
            --i; /* this arg was removed so go back */
        }
        else {
            /* this is a memleak.. oh well.. heh */
            gchar *err = g_strdup_printf
                (_("Invalid command line argument '%s'\n"), argv[i]);
            ob_exit_with_error(err);
        }
    }
}

static Cursor load_cursor(const gchar *name, guint fontval)
{
    Cursor c = None;

#if USE_XCURSOR
    c = XcursorLibraryLoadCursor(obt_display, name);
#endif
    if (c == None)
        c = XCreateFontCursor(obt_display, fontval);
    return c;
}

void ob_exit_with_error(const gchar *msg)
{
    g_message(msg);
    session_shutdown(TRUE);
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
    ob_exit(0);
}

void ob_reconfigure()
{
    reconfigure = TRUE;
    ob_exit(0);
}

void ob_exit(gint code)
{
    exitcode = code;
    obt_main_loop_exit(ob_main_loop);
}

void ob_exit_replace()
{
    exitcode = 0;
    being_replaced = TRUE;
    obt_main_loop_exit(ob_main_loop);
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
