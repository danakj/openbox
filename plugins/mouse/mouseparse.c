#include "kernel/action.h"
#include "kernel/parse.h"
#include "mouse.h"

void mouseparse(ParseToken *token)
{
    static char *top = NULL;
    static char *context = NULL, *button = NULL;
    static char *arg_str = NULL;
    static int arg_int = 0;
    static MouseAction event = -1;
    static Action *action = NULL;
    static gboolean err = FALSE;

    if (err) {
        if (token->type == TOKEN_NEWLINE)
            err = FALSE;
        /* just fall through and free the token */
    } else if (top == NULL) {
        if (token->type == TOKEN_IDENTIFIER &&
            !g_ascii_strcasecmp("mbind", token->data.identifier)) {
            top = token->data.identifier;
            return;
        } else {
            yyerror("syntax error (expected mbind)");
            err = TRUE;
        }
    } else if (context == NULL) {
        if (token->type == TOKEN_IDENTIFIER) {
            context = token->data.identifier;
            return;
        } else {
            yyerror("syntax error (expected Key)");
            err = TRUE;
        }
    } else if (event == (unsigned) -1) {
        if (token->type == TOKEN_IDENTIFIER) {
            if (!g_ascii_strcasecmp("press", token->data.identifier))
                event = MouseAction_Press;
            else if (!g_ascii_strcasecmp("release", token->data.identifier))
                event = MouseAction_Release;
            else if (!g_ascii_strcasecmp("click", token->data.identifier))
                event = MouseAction_Click;
            else if (!g_ascii_strcasecmp("doubleclick",token->data.identifier))
                event = MouseAction_DClick;
            else if (!g_ascii_strcasecmp("drag", token->data.identifier))
                event = MouseAction_Motion;
            if (event != (unsigned) -1)
                return;
            else {
                yyerror("invalid event");
                err = TRUE;
            }
        } else {
            yyerror("syntax error (expected event)");
            err = TRUE;
        }
    } else if (button == NULL) {
        if (token->type == TOKEN_IDENTIFIER) {
            button = token->data.identifier;
            return;
        } else {
            yyerror("syntax error (expected button)");
            err = TRUE;
        }
    } else if (action == NULL) {
        if (token->type == TOKEN_IDENTIFIER) {
            action = action_from_string(token->data.identifier);

            /* check for valid actions for motion events */
            if ((event == MouseAction_Motion) ^
                (action &&
                 (action->func == action_move ||
                  action->func == action_resize))) {
                action_free(action);
                action = NULL;
            }

            if (action != NULL) {
                return;
            } else {
                yyerror("invalid action");
                err = TRUE;
            }
        } else {
            yyerror("syntax error (expected action)");
            err = TRUE;
        }
    } else if (token->type == TOKEN_STRING) {
        arg_str = token->data.string;
        return;
    } else if (token->type == TOKEN_INTEGER) {
        arg_int = token->data.integer;
        return;
    } else if (token->type != TOKEN_NEWLINE) {
        yyerror("syntax error (unexpected trailing token)");
    } else {

        /* these use the argument */
        if (action->func == action_execute || action->func == action_restart)
            action->data.execute.path = g_strdup(arg_str);
        if ((action->func == action_desktop ||
             action->func == action_send_to_desktop) &&
            arg_int)
            action->data.desktop.desk = (unsigned) arg_int - 1;
        if (action->func == action_move_relative_horz ||
            action->func == action_move_relative_vert ||
            action->func == action_resize_relative_horz ||
            action->func == action_resize_relative_vert)
            action->data.relative.delta = arg_int;

        if (mbind(button, context, event, action))
            action = NULL; /* don't free this if mbind succeeds */
        else
            yyerror("failed to add mouse binding");
    }

    g_free(top); top = NULL;
    g_free(context); context = NULL;
    g_free(button); button = NULL;
    g_free(arg_str); arg_str = NULL;
    arg_int = 0;
    event = -1;
    action_free(action); action = NULL;
    parse_free_token(token);
}
