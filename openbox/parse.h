#ifndef __parse_h
#define __parse_h

#include <glib.h>
#include "parse.tab.h"

typedef enum {
    TOKEN_REAL,
    TOKEN_INTEGER,
    TOKEN_STRING,
    TOKEN_IDENTIFIER,
    TOKEN_BOOL,
    TOKEN_LBRACKET   = '(',
    TOKEN_RBRACKET   = ')',
    TOKEN_LBRACE     = '{',
    TOKEN_RBRACE     = '}',
    TOKEN_EQUALS     = '=',
    TOKEN_COMMA      = ',',
    TOKEN_NEWLINE    = '\n'
} ParseTokenType;

typedef void (*ParseFunc)(ParseTokenType type, union ParseToken token);

void parse_startup();
void parse_shutdown();

/* Parse the RC file
   found in parse.yacc
*/
void parse_rc();

void parse_reg_section(char *section, ParseFunc func);


/* Free a parsed token's allocated memory */
void parse_free_token(ParseTokenType type, union ParseToken token);

/* Display an error message while parsing.
   found in parse.yacc */
void yyerror(char *err);

#endif
