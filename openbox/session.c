/* This session code is largely inspired by metacity code. */

#ifndef USE_SM

#include "session.h"
#include "client.h"

void session_load(char *path) {}
void session_startup(int argc, char **argv) {}
void session_shutdown() {}
ObSessionState* session_state_find(ObClient *c) { return NULL; }
void session_state_free(ObSessionState *state) {}

#else

#include "debug.h"
#include "openbox.h"
#include "session.h"
#include "client.h"
#include "prop.h"
#include "parser/parse.h"

#include <time.h>
#include <errno.h>
#include <stdio.h>

#ifdef HAVE_UNISTD_H
#  include <sys/types.h>
#  include <unistd.h>
#endif

#include <X11/SM/SMlib.h>

static SmcConn     sm_conn;
static gchar      *save_file;
static gint        sm_argc;
static gchar     **sm_argv;
static GSList     *sm_saved_state;

static gboolean session_save();

static void sm_save_yourself(SmcConn conn, SmPointer data, int save_type,
                             Bool shutdown, int interact_style, Bool fast);
static void sm_die(SmcConn conn, SmPointer data);
static void sm_save_complete(SmcConn conn, SmPointer data);
static void sm_shutdown_cancelled(SmcConn conn, SmPointer data);

static void save_commands()
{
    SmProp *props[2];
    SmProp prop_cmd = { SmCloneCommand, SmLISTofARRAY8, 1, };
    SmProp prop_res = { SmRestartCommand, SmLISTofARRAY8, };
    gint i, j, n;
    gboolean has_id = FALSE, has_file = FALSE;

    for (i = 1; !has_id && !has_file && i < sm_argc - 1; ++i) {
        if (!has_id && strcmp(sm_argv[i], "--sm-client-id") == 0)
            has_id = TRUE;
        if (!has_file && strcmp(sm_argv[i], "--sm-save-file") == 0)
            has_file = TRUE;
    }

    n = (has_file ? sm_argc-2 : sm_argc);
    n = (has_id ? n-2 : n);
    prop_cmd.vals = g_new(SmPropValue, n);
    prop_cmd.num_vals = n;
    for (i = 0, j = 0; i < sm_argc; ++i, ++j) {
        if (strcmp (sm_argv[i], "--sm-client-id") == 0 ||
            strcmp (sm_argv[i], "--sm-save-file") == 0) {
            ++i, --j; /* skip the next as well, keep j where it is */
        } else {
            prop_cmd.vals[j].value = sm_argv[i];
            prop_cmd.vals[j].length = strlen(sm_argv[i]);
        }
    }

    n = (has_file ? sm_argc : sm_argc+2);
    n = (has_id ? n-2 : n);
    prop_res.vals = g_new(SmPropValue, n);
    prop_res.num_vals = n;
    for (i = 0, j = 0; i < sm_argc; ++i, ++j) { 
        if (strcmp (sm_argv[i], "--sm-client-id") == 0 ||
            strcmp (sm_argv[i], "--sm-save-file") == 0) {
            ++i, --j; /* skip the next as well, keep j where it is */
        } else {
            prop_res.vals[j].value = sm_argv[i];
            prop_res.vals[j].length = strlen(sm_argv[i]);
        }
    }

    if (save_file) {
        prop_res.vals[j].value = "--sm-save-file";
        prop_res.vals[j++].length = strlen("--sm-save-file");
        prop_res.vals[j].value = save_file;
        prop_res.vals[j++].length = strlen(save_file);
    } else {
        prop_res.vals[j].value = "--sm-client-id";
        prop_res.vals[j++].length = strlen("--sm-client-id");
        prop_res.vals[j].value = ob_sm_id;
        prop_res.vals[j++].length = strlen(ob_sm_id);
    }

    props[0] = &prop_res;
    props[1] = &prop_cmd;
    SmcSetProperties(sm_conn, 2, props);

    g_free(prop_res.vals);
    g_free(prop_cmd.vals);
}

void session_startup(int argc, char **argv)
{
#define SM_ERR_LEN 1024

    SmcCallbacks cb;
    char sm_err[SM_ERR_LEN];

    sm_argc = argc;
    sm_argv = argv;

    cb.save_yourself.callback = sm_save_yourself;
    cb.save_yourself.client_data = NULL;

    cb.die.callback = sm_die;
    cb.die.client_data = NULL;

    cb.save_complete.callback = sm_save_complete;
    cb.save_complete.client_data = NULL;

    cb.shutdown_cancelled.callback = sm_shutdown_cancelled;
    cb.shutdown_cancelled.client_data = NULL;

    sm_conn = SmcOpenConnection(NULL, NULL, 1, 0,
                                SmcSaveYourselfProcMask |
                                SmcDieProcMask |
                                SmcSaveCompleteProcMask |
                                SmcShutdownCancelledProcMask,
                                &cb, ob_sm_id, &ob_sm_id,
                                SM_ERR_LEN, sm_err);
    if (sm_conn == NULL)
        g_warning("Failed to connect to session manager: %s", sm_err);
    else {
        SmPropValue val_prog;
        SmPropValue val_uid;
        SmPropValue val_hint; 
        SmPropValue val_pri;
        SmPropValue val_pid;
        SmProp prop_prog = { SmProgram, SmARRAY8, 1, };
        SmProp prop_uid = { SmUserID, SmARRAY8, 1, };
        SmProp prop_hint = { SmRestartStyleHint, SmCARD8, 1, };
        SmProp prop_pid = { SmProcessID, SmARRAY8, 1, };
        SmProp prop_pri = { "_GSM_Priority", SmCARD8, 1, };
        SmProp *props[6];
        gchar hint, pri;
        gchar pid[32];

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
        props[1] = &prop_uid;
        props[2] = &prop_hint;
        props[3] = &prop_pid;
        props[4] = &prop_pri;

        SmcSetProperties(sm_conn, 5, props);

        g_free(val_uid.value);

        save_commands();

        ob_debug("Connected to session manager with id %s\n", ob_sm_id);
    }
}

void session_shutdown()
{
    g_free(save_file);

    if (sm_conn) {
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

        SmcSetProperties(sm_conn, 1, props);

        SmcCloseConnection(sm_conn, 0, NULL);
    }
}

static void sm_save_yourself_phase2(SmcConn conn, SmPointer data)
{
    gboolean success;

    ob_debug("got SAVE YOURSELF PHASE 2 from session manager\n");

    success = session_save();
    save_commands();

    SmcSaveYourselfDone(conn, success);
}

static void sm_save_yourself(SmcConn conn, SmPointer data, int save_type,
                             Bool shutdown, int interact_style, Bool fast)
{
    ob_debug("got SAVE YOURSELF from session manager\n");

    if (!SmcRequestSaveYourselfPhase2(conn, sm_save_yourself_phase2, data)) {
        ob_debug("SAVE YOURSELF PHASE 2 failed\n");
        SmcSaveYourselfDone(conn, FALSE);
    }
}

static void sm_die(SmcConn conn, SmPointer data)
{
    ob_exit();
    ob_debug("got DIE from session manager\n");
}

static void sm_save_complete(SmcConn conn, SmPointer data)
{
    ob_debug("got SAVE COMPLETE from session manager\n");
}

static void sm_shutdown_cancelled(SmcConn conn, SmPointer data)
{
    ob_debug("got SHUTDOWN CANCELLED from session manager\n");
}

static gboolean session_save()
{
    gchar *filename;
    FILE *f;
    GList *it;
    gboolean success = TRUE;

    /* this algo is from metacity */
    filename = g_strdup_printf("%d-%d-%u.obs",
                               (int) time(NULL),
                               (int) getpid(),
                               g_random_int());
    save_file = g_build_filename(g_get_home_dir(), ".openbox", "sessions",
                                 filename, NULL);
    g_free(filename);

    f = fopen(save_file, "w");
    if (!f) {
        success = FALSE;
        g_warning("unable to save the session to %s: %s",
                  save_file, strerror(errno));
    } else {
        fprintf(f, "<?xml version=\"1.0\"?>\n\n");
        fprintf(f, "<openbox_session id=\"%s\">\n\n", ob_sm_id);

        for (it = client_list; it; it = g_list_next(it)) {
            guint num;
            gint32 *dimensions;
            gint prex, prey, prew, preh;
            ObClient *c = it->data;
            gchar *client_id, *t;

            if (!client_normal(c))
                continue;

            if (!(client_id = client_get_sm_client_id(c)))
                continue;

            prex = c->area.x;
            prey = c->area.y;
            prew = c->area.width;
            preh = c->area.height;
            if (PROP_GETA32(c->window, openbox_premax, cardinal,
                            (guint32**)&dimensions, &num)) {
                if (num == 4) {
                    prex = dimensions[0];
                    prey = dimensions[1];
                    prew = dimensions[2];
                    preh = dimensions[3];
                }
                g_free(dimensions);
            }

            fprintf(f, "<window id=\"%s\">\n", client_id);

            t = g_markup_escape_text(c->name, -1);
            fprintf(f, "\t<name>%s</name>\n", t);
            g_free(t);

            t = g_markup_escape_text(c->class, -1);
            fprintf(f, "\t<class>%s</class>\n", t);
            g_free(t);

            t = g_markup_escape_text(c->role, -1);
            fprintf(f, "\t<role>%s</role>\n", t);
            g_free(t);

            fprintf(f, "\t<desktop>%d</desktop>\n", c->desktop);
            fprintf(f, "\t<x>%d</x>\n", prex);
            fprintf(f, "\t<y>%d</y>\n", prey);
            fprintf(f, "\t<width>%d</width>\n", prew);
            fprintf(f, "\t<height>%d</height>\n", preh);
            if (c->shaded)
                fprintf(f, "\t<shaded />\n");
            if (c->iconic)
                fprintf(f, "\t<iconic />\n");
            if (c->skip_pager)
                fprintf(f, "\t<skip_pager />\n");
            if (c->skip_taskbar)
                fprintf(f, "\t<skip_taskbar />\n");
            if (c->fullscreen)
                fprintf(f, "\t<fullscreen />\n");
            if (c->above)
                fprintf(f, "\t<above />\n");
            if (c->below)
                fprintf(f, "\t<below />\n");
            if (c->max_horz)
                fprintf(f, "\t<max_horz />\n");
            if (c->max_vert)
                fprintf(f, "\t<max_vert />\n");
            fprintf(f, "</window>\n\n");

            g_free(client_id);
        }

        fprintf(f, "</openbox_session>\n");

        if (fflush(f)) {
            success = FALSE;
            g_warning("error while saving the session to %s: %s",
                      save_file, strerror(errno));
        }
        fclose(f);
    }

    return success;
}

void session_state_free(ObSessionState *state)
{
    if (state) {
        g_free(state->id);
        g_free(state->name);
        g_free(state->class);
        g_free(state->role);

        g_free(state);
    }
}

static gboolean session_state_cmp(ObSessionState *s, ObClient *c)
{
    gchar *client_id;

    if (!(client_id = client_get_sm_client_id(c)))
        return FALSE;
    g_print("\nsaved %s\nnow %s\n", s->id, client_id);
    if (strcmp(s->id, client_id)) {
        g_free(client_id);
        return FALSE;
    }
    g_free(client_id);
    g_print("saved %s\nnow %s\n", s->name, c->name);
    if (strcmp(s->name, c->name))
        return FALSE;
    g_print("saved %s\nnow %s\n", s->class, c->class);
    if (strcmp(s->class, c->class))
        return FALSE;
    g_print("saved %s\nnow %s\n\n", s->role, c->role);
    if (strcmp(s->role, c->role))
        return FALSE;
    return TRUE;
}

ObSessionState* session_state_find(ObClient *c)
{
    GSList *it;

    for (it = sm_saved_state; it; it = g_slist_next(it))
        if (session_state_cmp(it->data, c)) {
            ObSessionState *s = it->data;
            sm_saved_state = g_slist_remove(sm_saved_state, s);
            return s;
        }
    return NULL;
}

void session_load(char *path)
{
    xmlDocPtr doc;
    xmlNodePtr node, n;
    gchar *sm_id;

    if (!parse_load(path, "openbox_session", &doc, &node))
        return;

    if (!parse_attr_string("id", node, &sm_id))
        return;
    ob_sm_id = g_strdup(sm_id);

    node = parse_find_node("window", node->xmlChildrenNode);
    while (node) {
        ObSessionState *state;

        state = g_new0(ObSessionState, 1);

        if (!parse_attr_string("id", node, &state->id))
            goto session_load_bail;
        if (!(n = parse_find_node("name", node->xmlChildrenNode)))
            goto session_load_bail;
        state->name = parse_string(doc, n);
        if (!(n = parse_find_node("class", node->xmlChildrenNode)))
            goto session_load_bail;
        state->class = parse_string(doc, n);
        if (!(n = parse_find_node("role", node->xmlChildrenNode)))
            goto session_load_bail;
        state->role = parse_string(doc, n);
        if (!(n = parse_find_node("desktop", node->xmlChildrenNode)))
            goto session_load_bail;
        state->desktop = parse_int(doc, n);
        if (!(n = parse_find_node("x", node->xmlChildrenNode)))
            goto session_load_bail;
        state->x = parse_int(doc, n);
        if (!(n = parse_find_node("y", node->xmlChildrenNode)))
            goto session_load_bail;
        state->y = parse_int(doc, n);
        if (!(n = parse_find_node("width", node->xmlChildrenNode)))
            goto session_load_bail;
        state->w = parse_int(doc, n);
        if (!(n = parse_find_node("height", node->xmlChildrenNode)))
            goto session_load_bail;
        state->h = parse_int(doc, n);

        state->shaded =
            parse_find_node("shaded", node->xmlChildrenNode) != NULL;
        state->iconic =
            parse_find_node("iconic", node->xmlChildrenNode) != NULL;
        state->skip_pager =
            parse_find_node("skip_pager", node->xmlChildrenNode) != NULL;
        state->skip_taskbar =
            parse_find_node("skip_taskbar", node->xmlChildrenNode) != NULL;
        state->fullscreen =
            parse_find_node("fullscreen", node->xmlChildrenNode) != NULL;
        state->above =
            parse_find_node("above", node->xmlChildrenNode) != NULL;
        state->below =
            parse_find_node("below", node->xmlChildrenNode) != NULL;
        state->max_horz =
            parse_find_node("max_horz", node->xmlChildrenNode) != NULL;
        state->max_vert =
            parse_find_node("max_vert", node->xmlChildrenNode) != NULL;
        
        /* save this */
        sm_saved_state = g_slist_prepend(sm_saved_state, state);
        goto session_load_ok;

    session_load_bail:
        session_state_free(state);

    session_load_ok:

        node = parse_find_node("window", node->next);
    }

    xmlFreeDoc(doc);
}

#endif
