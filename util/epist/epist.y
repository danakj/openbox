%{
#include <stdio.h>
#include <string.h>
#include "parser.hh"
    
#define YYPARSE_PARAM parser_obj
#define YYSTYPE char*
    
extern "C" {
    int yylex();
    int yywrap() {
        return 1;
    }
}

void yyerror(const char *c) {
    printf("ERROR: %s\n", c);
}


%}

%token OBRACE EBRACE SEMICOLON DASH NUMBER QUOTES WORD BINDING OPTIONS

%%

commands:
    | commands command
    | commands options_block
    ;

command:
    action_command | chain_command
    ;

action_command:
    binding WORD parameter SEMICOLON
    {
        ((parser*)parser_obj)->setAction($2);
        ((parser*)parser_obj)->endAction();
    }
    
    ;

chain_command:
    binding obrace commands ebrace
    {
        ((parser*)parser_obj)->endChain();
    }
    ;

options_block:
    options_keyword OBRACE options EBRACE
    ;

binding:
    binding_w_modifier bind_key
    ;

obrace:
    OBRACE { ((parser*)parser_obj)->startChain(); }
    ;

ebrace:
    EBRACE { /* ((parser*)parser_obj)->endChain(); */ }
    ;

binding_w_modifier:
    | BINDING DASH binding_w_modifier { ((parser*)parser_obj)->addModifier($1); }
    ;

bind_key:
    OBRACE       { ((parser*)parser_obj)->setKey($1); }
    | EBRACE     { ((parser*)parser_obj)->setKey($1); }
    | DASH       { ((parser*)parser_obj)->setKey($1); }
    | SEMICOLON  { ((parser*)parser_obj)->setKey($1); }
    | NUMBER     { ((parser*)parser_obj)->setKey($1); }
    | WORD       { ((parser*)parser_obj)->setKey($1); }
    ;

parameter:
    | NUMBER      { ((parser*)parser_obj)->setArgumentNum($1); }
    | DASH NUMBER { ((parser*)parser_obj)->setArgumentNegNum($2); }
    | QUOTES      { ((parser*)parser_obj)->setArgumentStr($1); }
    ;

options_keyword:
    OPTIONS
    ;

options:
    | options option
    ;

option:
    WORD parameter SEMICOLON
    { ((parser*)parser_obj)->setOption($1); }
    ;

%%

