#include "kernel/parse.h"
#include "keyboard.h"

void keyparse(ParseToken *token)
{
    static char *top = NULL;
    static Action *action = NULL;
    static GList *chain = NULL;
    static gboolean err = FALSE;
    static char *arg_str = NULL;
    static int arg_int = 0;
    GList *it;

    if (err) {
        if (token->type == TOKEN_NEWLINE)
            err = FALSE;
        /* just fall through and free the token */
    } else if (top == NULL) {
        if (token->type == TOKEN_IDENTIFIER &&
            !g_ascii_strcasecmp("kbind", token->data.identifier)) {
            top = token->data.identifier;
            return;
        } else {
            yyerror("syntax error (expected kbind)");
            err = TRUE;
        }
    } else if (chain == NULL) {
        if (token->type == TOKEN_LIST) {
            for (it = token->data.list; it; it = it->next)
                if (((ParseToken*)it->data)->type != TOKEN_IDENTIFIER) break;
            if (it == NULL) {
                chain = token->data.list;
                return;
            } else {
                yyerror("invalid element in key chain");
                err = TRUE;
            }
        } else {
            yyerror("syntax error (expected key chain)");
            err = TRUE;
        }
    } else if (action == NULL) {
        if (token->type == TOKEN_IDENTIFIER) {
            action = action_from_string(token->data.identifier);

            /* no move/resize with the keyboard */
            if (action &&
                (action->func == action_move ||
                 action->func == action_resize)) {
                action_free(action);
                action = NULL;
            }

            if (action != NULL) {
                parse_free_token(token); /* its data isnt saved */
                return;
            } else {
                yyerror("invalid action");
                err = TRUE;
            }
        } else {
            yyerror("syntax error (expected action)");
            err = TRUE;
        }
    } else if (token->type == TOKEN_STRING) { /* string argument */
        arg_str = token->data.string;
        return;
    } else if (token->type == TOKEN_INTEGER) { /* number argument */
        arg_int = token->data.integer;
        return;
    } else if (token->type != TOKEN_NEWLINE) {
        yyerror("syntax error (unexpected trailing token)");
        err = TRUE;
    } else {
        GList *strchain = NULL;

        /* build a list of just char*'s */
        for (it = chain; it; it = it->next)
            strchain = g_list_append(strchain,
                                     ((ParseToken*)it->data)->data.identifier);

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

        if (kbind(strchain, action))
            action = NULL; /* don't free this if kbind succeeds */
        else
            yyerror("failed to add key binding");
        /* free the char*'s */
        g_list_free(strchain);

        err = FALSE;
    }    

    g_free(top); top = NULL;
    action_free(action); action = NULL;
    g_free(arg_str); arg_str = NULL;
    arg_int = 0;
    for (it = chain; it; it = it->next) {
        parse_free_token(it->data);
        g_free(it->data);
    }
    g_list_free(chain); chain = NULL;
    parse_free_token(token);
}
