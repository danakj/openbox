#ifndef __parse_h
#define __parse_h

#include <glib.h>
#include "parse.tab.h"

typedef enum {
    TOKEN_REAL       = REAL,
    TOKEN_INTEGER    = INTEGER,
    TOKEN_STRING     = STRING,
    TOKEN_IDENTIFIER = IDENTIFIER,
    TOKEN_BOOL       = BOOL,
    TOKEN_LIST,
    TOKEN_LBRACE     = '{',
    TOKEN_RBRACE     = '}',
    TOKEN_COMMA      = ',',
    TOKEN_NEWLINE    = '\n'
} ParseTokenType;

typedef struct {
    ParseTokenType type;
    union ParseTokenData data;
} ParseToken;

typedef void (*ParseFunc)(ParseToken *token);
typedef void (*AssignParseFunc)(char *name, ParseToken *value);

void parse_startup();
void parse_shutdown();

/* Parse the RC file
   found in parse.yacc
*/
void parse_rc();

void parse_reg_section(char *section, ParseFunc func, AssignParseFunc afunc);


/* Free a parsed token's allocated memory */
void parse_free_token(ParseToken *token);

/* Display an error message while parsing.
   found in parse.yacc */
void yyerror(char *err);

#endif
