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

%token OBRACE EBRACE SEMICOLON DASH ACTION BINDING NUMBER QUOTES WORD  

%%

commands:
    | commands command
    ;

command:
    action_command | chain_command
    ;

action_command:
    binding ACTION parameter SEMICOLON
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
    | NUMBER      { ((parser*)parser_obj)->setArgument($1); }
    | DASH NUMBER { ((parser*)parser_obj)->setArgument($1); }
    | QUOTES      { ((parser*)parser_obj)->setArgument($1); }
    ;

%%

