#include "parse.h"
#include "config.h"

static GHashTable *reg = NULL;
static ParseFunc func = NULL;

/* parse tokens from the [openbox] section of the rc file */
static void parse_rc_token(ParseToken *token);

void destkey(gpointer key) { g_free(key); }

void parse_startup()
{
    reg = g_hash_table_new_full(g_str_hash, g_str_equal, destkey, NULL);
    func = NULL;

    parse_reg_section("openbox", parse_rc_token);
}

void parse_shutdown()
{
    g_hash_table_destroy(reg);
}

void parse_reg_section(char *section, ParseFunc func)
{
    if (g_hash_table_lookup(reg, section) != NULL)
        g_warning("duplicate request for section '%s' in the rc file",
                  section);
    else
        g_hash_table_insert(reg, g_ascii_strdown(section, -1), (void*)func);
}

void parse_free_token(ParseToken *token)
{
    GSList *it;

    switch (token->type) {
    case TOKEN_STRING:
        g_free(token->data.string);
        break;
    case TOKEN_IDENTIFIER:
        g_free(token->data.identifier);
        break;
    case TOKEN_LIST:
        for (it = token->data.list; it; it = it->next) {
            parse_free_token(it->data);
            g_free(it->data);
        }
        g_slist_free(token->data.list);
        break;
    case TOKEN_REAL:
    case TOKEN_INTEGER:
    case TOKEN_BOOL:
    case TOKEN_LBRACE:
    case TOKEN_RBRACE:
    case TOKEN_EQUALS:
    case TOKEN_COMMA:
    case TOKEN_NEWLINE:
        break;
    }
}

void parse_set_section(char *section)
{
    func = (ParseFunc)g_hash_table_lookup(reg, section);
}

void parse_token(ParseToken *token)
{
    if (func != NULL)
        func(token);
}

static void parse_rc_token(ParseToken *token)
{
    static int got_eq = FALSE;
    static ParseTokenType got_val = 0;
    static char *id = NULL, *s = NULL;
    static int i;
    static gboolean b;

    if (id == NULL) {
        if (token->type == TOKEN_IDENTIFIER) {
            id = token->data.identifier;
            return;
        } else {
            yyerror("syntax error");
        }
    } else if (!got_eq) {
        if (token->type == TOKEN_EQUALS) {
            got_eq = TRUE;
            return;
        } else {
            yyerror("syntax error");
        }
    } else if (!got_val) {
        if (token->type == TOKEN_STRING) {
            s = token->data.string;
            got_val = token->type;
            return;
        } else if (token->type == TOKEN_BOOL) {
            b = token->data.bool;
            got_val = token->type;
            return;
        } else if (token->type == TOKEN_INTEGER) {
            i = token->data.integer;
            got_val = token->type;
            return;
        } else
            yyerror("syntax error");
    } else if (token->type != TOKEN_NEWLINE) {
        yyerror("syntax error");
    } else {
        ConfigValue v;

        switch (got_val) {
        case TOKEN_STRING:
            v.string = s;
            if (!config_set(id, Config_String, v))
                yyerror("invalid value type");
            break;
        case TOKEN_BOOL:
            v.bool = b;
            if (!config_set(id, Config_Bool, v))
                yyerror("invalid value type");
            break;
        case TOKEN_INTEGER:
            v.integer = i;
            if (!config_set(id, Config_Integer, v))
                yyerror("invalid value type");
            break;
        default:
            g_assert_not_reached(); /* unhandled type got parsed */
        }
    }

    g_free(id);
    g_free(s);
    id = s = NULL;
    got_eq = FALSE;
    got_val = 0;
    parse_free_token(token);
}
