/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   actions_parser.c for the Openbox window manager
   Copyright (c) 2011        Dana Jansens

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

#include "actions_parser.h"
#include "actions.h"
#include "actions_list.h"
#include "actions_value.h"
#include "gettext.h"

#ifdef HAVE_STRING_H
#  include <string.h>
#endif

struct _ObActionsList;
struct _ObActionsListTest;
struct _ObActionsValue;

#define SKIP " \t"
#define IDENTIFIER_FIRST G_CSET_a_2_z G_CSET_A_2_Z G_CSET_DIGITS "-_"
#define IDENTIFIER_NTH IDENTIFIER_FIRST G_CSET_LATINS G_CSET_LATINC
#define ESCAPE_SEQS "\"()"

struct _ObActionsList* parse_list(ObActionsParser *p,
                                  GTokenType end,
                                  gboolean *e);
struct _ObActionsList* parse_action(ObActionsParser *p, gboolean *e);
struct _ObActionsList* parse_filter(ObActionsParser *p, gboolean *e);
struct _ObActionsListTest* parse_filter_test(ObActionsParser *p, gboolean *e);
struct _ObActionsValue* parse_value(ObActionsParser *p,
                                        gboolean allow_actions,
                                        gboolean *e);
gchar* parse_string(ObActionsParser *p, guchar end, gboolean *e);

struct _ObActionsParser
{
    gint ref;
    GScanner *scan;
};

ObActionsParser* actions_parser_new(void)
{
    ObActionsParser *p;
    GScannerConfig config = {
        .cset_skip_characters = SKIP,
        .cset_identifier_first =  IDENTIFIER_FIRST,
        .cset_identifier_nth = IDENTIFIER_NTH,
        .cpair_comment_single = "#\n",
        .case_sensitive = FALSE,
        .skip_comment_multi = TRUE,
        .skip_comment_single = TRUE,
        .scan_comment_multi = FALSE,
        .scan_identifier = TRUE,
        .scan_identifier_1char = TRUE,
        .scan_identifier_NULL = FALSE,
        .scan_symbols = TRUE,
        .scan_binary = FALSE,
        .scan_octal = FALSE,
        .scan_float = TRUE,
        .scan_hex = FALSE,
        .scan_hex_dollar = FALSE,
        .scan_string_sq = FALSE,
        .scan_string_dq = FALSE,
        .numbers_2_int = TRUE,
        .int_2_float = FALSE,
        .identifier_2_string = FALSE,
        .char_2_token = TRUE,
        .symbol_2_token = FALSE,
        .scope_0_fallback = FALSE,
        .store_int64 = FALSE
    };

    p = g_slice_new(ObActionsParser);
    p->ref = 1;
    p->scan = g_scanner_new(&config);
    return p;
}

void actions_parser_ref(ObActionsParser *p)
{
    ++p->ref;
}

void actions_parser_unref(ObActionsParser *p)
{
    if (p && --p->ref < 1) {
        g_scanner_destroy(p->scan);
        g_slice_free(ObActionsParser, p);
    }
}

ObActionsList* actions_parser_read_string(ObActionsParser *p,
                                          const gchar *text)
{
    gboolean e;

    g_scanner_input_text(p->scan, text, strlen(text));
    p->scan->input_name = "(console)";

    e = FALSE;
    return parse_list(p, G_TOKEN_EOF, &e);
}

ObActionsList* actions_parser_read_file(ObActionsParser *p,
                                        const gchar *file,
                                        GError **error)
{
    GIOChannel *ch;
    gboolean e;

    ch = g_io_channel_new_file(file, "r", error);
    if (!ch) return NULL;

    g_scanner_input_file(p->scan, g_io_channel_unix_get_fd(ch));
    p->scan->input_name = file;

    e = FALSE;
    return parse_list(p, G_TOKEN_EOF, &e);
}

/*************  Parser functions.  The list is the entry point.  ************
BNF for the language:

TEST       := KEY=VALUE | KEY
ACTION     := [FILTER] ACTION ELSE END | ACTIONNAME ACTIONOPTS | {ACTIONLIST}
ELSE       := nil | \| ACTION
END        := \n | ; | EOF
ACTIONLIST := ACTION ACTIONLIST | ACTION
FILTER     := FILTERORS
FILTERORS  := FILTERANDS \| FILTERORS | FILTERANDS
FILTERANDS := TEST, FILTERANDS | TEST
ACTIONOPTS := ACTIONOPT ACTIONOPTS | ACTIONOPT
ACTIONOPT  := ATTRIBUTE:WORD | ATTRIBUTE:STRING | ATTRIBUTE:{ACTIONLIST}
WORD   := string of text without any spaces
STRING := "TEXT" | (TEXT) |
  where TEXT is a string, any occurance of the closing quote character must
  be escaped by an backslash.
  \\ \( \) and \" are all valid escaped characters.
**************                                                   ************/

gpointer parse_error(ObActionsParser *p, GTokenType exp, const gchar *message,
                     gboolean *e)
{
    g_scanner_unexp_token(p->scan, exp, NULL, NULL, NULL, message, TRUE);
    *e = TRUE;
    return NULL;
}

ObActionsList* parse_list(ObActionsParser *p, GTokenType end, gboolean *e)
{
    ObActionsList *first, *last;
    GTokenType t;

    first = last = NULL;

    t = g_scanner_peek_next_token(p->scan);
    while (t != end && t != G_TOKEN_EOF) {
        if (t == '\n')
            g_scanner_get_next_token(p->scan); /* skip empty lines */
        else if (t == ';') {
            g_scanner_get_next_token(p->scan); /* separator */
        }
        else if (t == G_TOKEN_IDENTIFIER) {
            ObActionsList *next;

            /* parse the next action and stick it on the end of the list */
            next = parse_action(p, e);
            if (last) last->next = next;
            if (!first) first = next;
            last = next;
        }
        else {
            g_scanner_get_next_token(p->scan);
            parse_error(p, (end ? end : G_TOKEN_NONE),
                        _("Expected an action or end of action list"), e);
        }

        if (*e) break; /* don't parse any more after an error */

        t = g_scanner_peek_next_token(p->scan);
    }

    /* eat the ending character */
    g_scanner_get_next_token(p->scan);

    return first;
}

ObActionsList* parse_action(ObActionsParser *p, gboolean *e)
{
    GTokenType t;
    ObActionsList *al;
    gchar *name;
    GHashTable *config;

    t = g_scanner_get_next_token(p->scan);

    if (t == '[') return parse_filter(p, e);
    if (t == '{') return parse_list(p, '}', e);

    /* check for a name */
    if (t != G_TOKEN_IDENTIFIER)
        return parse_error(p, G_TOKEN_NONE, _("Expected an action name"), e);

    /* read the action's name */
    name = g_strdup(p->scan->value.v_string);
    config = g_hash_table_new_full(g_str_hash, g_str_equal, g_free,
                                   (GDestroyNotify)actions_value_unref);

    /* read the action's options key:value pairs */
    t = g_scanner_peek_next_token(p->scan);
    while (t == G_TOKEN_IDENTIFIER || t == '\\') {
        if (t == '\\') {
            g_scanner_get_next_token(p->scan); /* eat it */
            t = g_scanner_get_next_token(p->scan); /* check for '\n' */
            if (t != '\n')
                parse_error(p, G_TOKEN_NONE, _("Expected newline"), e);
        }
        else {
            gchar *key;
            ObActionsValue *value;

            g_scanner_get_next_token(p->scan); /* eat the key */
            t = g_scanner_peek_next_token(p->scan); /* check for ':' */
            if (t != ':') {
                g_scanner_get_next_token(p->scan);
                parse_error(p, ':', NULL, e);
                break; /* don't read any more options */
            }

            /* save the key */
            key = g_strdup(p->scan->value.v_string);
            g_scanner_get_next_token(p->scan); /* eat the ':' */

            /* read the value */
            value = parse_value(p, TRUE, e);

            /* check if we read a value (regardless of errors), and save
               the key:value pair if we did. */
            if (value)
                g_hash_table_replace(config, key, value);
            else
                g_free(key); /* didn't read any value */
        }

        if (*e) break; /* don't parse any more if there was an error */

        t = g_scanner_peek_next_token(p->scan);
    }

    al = g_slice_new(ObActionsList);
    al->ref = 1;
    al->isfilter = FALSE;
    al->u.action = actions_act_new(name, config);
    al->next = NULL;
    g_free(name);
    g_hash_table_unref(config);
    return al;
}

ObActionsList* parse_filter(ObActionsParser *p, gboolean *e)
{
    GTokenType t;
    ObActionsList *al, *thendo, *elsedo;
    ObActionsListTest *test;

    /* read the filter tests */
    test = parse_filter_test(p, e);
    if (*e) {
        actions_list_test_destroy(test);
        return NULL;
    }

    /* read the action for the filter */
    thendo = parse_action(p, e);
    elsedo = NULL;

    if (!*e) {
        /* check for else case */
        t = g_scanner_peek_next_token(p->scan);
        if (t == '|') {
            g_scanner_get_next_token(p->scan); /* eat it */

            /* read the else action for the filter */
            elsedo = parse_action(p, e);
        }
    }

    al = g_slice_new(ObActionsList);
    al->ref = 1;
    al->isfilter = TRUE;
    al->u.f.test = test;
    al->u.f.thendo = thendo;
    al->u.f.elsedo = elsedo;
    al->next = NULL;
    return al;
}

ObActionsListTest* parse_filter_test(ObActionsParser *p, gboolean *e)
{
    GTokenType t;
    gchar *key;
    ObActionsValue *value;
    gboolean and;
    ObActionsListTest *next;

    t = g_scanner_get_next_token(p->scan);
    if (t == ']') /* empty */
        return NULL;

    else if (t != G_TOKEN_IDENTIFIER)
        return parse_error(p, G_TOKEN_NONE,
                           _("Expected a filter test lvalue"), e);

    /* got a key */
    key = g_strdup(p->scan->value.v_string);
    value = NULL;
    and = FALSE;
    next = NULL;

    /* check if it has a value also */
    t = g_scanner_peek_next_token(p->scan);
    if (t == '=') {
        g_scanner_get_next_token(p->scan); /* eat the = */
        value = parse_value(p, FALSE, e);
    }

    /* don't allow any errors (but value can be null if not present) */
    if (*e) {
        g_free(key);
        actions_value_unref(value);
        return NULL;
    }

    /* check if there is another test and how we're connected */
    t = g_scanner_get_next_token(p->scan);
    if (t == ',') { /* and */
        and = TRUE;
        next = parse_filter_test(p, e);
    }
    else if (t == '|') { /* or */
        and = FALSE;
        next = parse_filter_test(p, e);
    }
    else if (t == ']') /* end of the filter */
        ;
    else
        parse_error(p, ']', NULL, e);

    /* don't allow any errors */
    if (*e) {
        g_free(key);
        actions_value_unref(value);
        actions_list_test_destroy(next);
        return NULL;
    }
    else {
        ObActionsListTest *test;

        test = g_slice_new(ObActionsListTest);
        test->key = key;
        test->value = value;
        test->and = and;
        test->next = next;
        return test;
    }
}

ObActionsValue* parse_value(ObActionsParser *p,
                                gboolean allow_actions,
                                gboolean *e)
{
    GTokenType t;
    ObActionsValue *v;

    v = NULL;
    t = g_scanner_get_next_token(p->scan);
    if (t == G_TOKEN_IDENTIFIER) {
        v = actions_value_new_string(p->scan->value.v_string);
    }
    else if (t == '"')
        v = actions_value_new_string(parse_string(p, '"', e));
    else if (t == '(')
        v = actions_value_new_string(parse_string(p, ')', e));
    else if (t == '{' && allow_actions) {
        ObActionsList *l = parse_list(p, '}', e);
        if (l) v = actions_value_new_actions_list(l);
    }
    else
        parse_error(p, G_TOKEN_NONE, _("Expected an option value"), e);
    return v;
}


gchar* parse_string(ObActionsParser *p, guchar end, gboolean *e)
{
    GString *buf;
    GTokenType t;
    const gchar *error_message = NULL;

    /* inside a string we want everything to be parsed as text (identifiers) */
    p->scan->config->cset_skip_characters = "";
    p->scan->config->cset_identifier_first = IDENTIFIER_NTH " ";
    p->scan->config->cset_identifier_nth = IDENTIFIER_NTH " ";

    buf = g_string_new("");

    t = g_scanner_get_next_token(p->scan);
    while (t != end) {
        if (t == G_TOKEN_IDENTIFIER)
            g_string_append(buf, p->scan->value.v_string);
        else if (t == G_TOKEN_EOF) {
            error_message = _("Missing end of quoted string");
            goto parse_string_error;
        }
        else if (t == '\\') { /* escape sequence */
            t = g_scanner_get_next_token(p->scan);
            if (!strchr(ESCAPE_SEQS, t)) {
                error_message = _("Unknown escape sequence");
                goto parse_string_error;
            }
            g_string_append_c(buf, t);
        }
        else /* other single character */
            g_string_append_c(buf, t);

        t = g_scanner_get_next_token(p->scan);
    }

    /* reset to their default values */
    p->scan->config->cset_skip_characters = SKIP;
    p->scan->config->cset_identifier_first = IDENTIFIER_FIRST;
    p->scan->config->cset_identifier_nth = IDENTIFIER_NTH;

    {
        gchar *v = buf->str;
        g_string_free(buf, FALSE);
        return v;
    }

parse_string_error:
    g_scanner_unexp_token(p->scan, G_TOKEN_NONE, NULL, NULL, NULL,
                          error_message, TRUE);
    g_string_free(buf, TRUE);
    *e = TRUE;
    return NULL;
}
