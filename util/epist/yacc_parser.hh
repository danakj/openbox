#ifndef BISON_YACC_PARSER_HH
# define BISON_YACC_PARSER_HH

# ifndef YYSTYPE
#  define YYSTYPE int
# endif
# define	OBRACE	257
# define	EBRACE	258
# define	SEMICOLON	259
# define	DASH	260
# define	NUMBER	261
# define	QUOTES	262
# define	WORD	263
# define	BINDING	264
# define	OPTIONS	265


extern YYSTYPE yylval;

#endif /* not BISON_YACC_PARSER_HH */
