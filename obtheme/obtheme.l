%{
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include "frame.h"
#include "misc.h"
#include "render.h"
#include "obtheme.h"
#include "obtheme.tab.h"

int yylex(YYSTYPE *yylval, struct parser_control *pc);

void obthemeerror(struct parser_control *pc, char *s)
{
	printf("Parse error in file %s on line %d\n%s"
	        , pc->currfile[pc->include_stack_ptr]
	        , pc->currline[pc->include_stack_ptr] + 1
		, pc->error_buf);
}

extern int parserparse(void *);

int yywrap(void)
{
	return 1;
}

int obtheme_parse(struct obthemedata *td, const char *filename)
{
	FILE *input;
	int ret;
	struct parser_control *pc;
//illdebug = 1;
	pc = parser_init(td);
	input = fopen(filename, "r");
	if (!input)
		return 2;
	yyin = input;
	yyrestart(input);
	BEGIN 0;
	pc->include_stack_ptr = 0;
	pc->currline[pc->include_stack_ptr] = 0;
	strncpy(pc->currfile[pc->include_stack_ptr], filename, 500);
	ret = obthemeparse(pc);
	if (ret != 0) {
	//XXX I THINK I NEED TO CLOSE ALL INCLUDE FILES HERE
	//probably also fclose input and call parser_finish?
		return ret;
	}
	fclose(input);
	parser_finish(pc);
	return ret;
}

%}
%x comment
%x incl
%%

include			BEGIN(incl);
<incl>[ \t]*		/* eat the whitespace */
<incl>[^ \t\n]+		{ /* got the include file name */
	char *incfile;
	if ( pc->include_stack_ptr >= MAX_INCLUDE_DEPTH) {
		fprintf( stderr, "Includes nested too deeply" );
		exit(1);
	}

	pc->include_stack[pc->include_stack_ptr++] = YY_CURRENT_BUFFER;
	incfile = malloc(strlen(yytext) + strlen("include/")+ 1);
	strcpy(incfile, "include/");
	strcat(incfile, yytext);
	yyin = fopen( incfile, "r" );
	free(incfile);
	strncpy(pc->currfile[pc->include_stack_ptr], yytext, 500);
	pc->currline[pc->include_stack_ptr] = 0;
	if ( ! yyin ) {
		printf("Could not find include file %s (%s)\n", yytext, incfile);
		exit(1);
	}
	yy_switch_to_buffer(yy_create_buffer(yyin, YY_BUF_SIZE));

	BEGIN(INITIAL);
	}

<<EOF>> {
        if ( --(pc->include_stack_ptr) < 0 )
            {
			yy_delete_buffer(YY_CURRENT_BUFFER);
            yyterminate();
            }

        else
            {
            yy_delete_buffer( YY_CURRENT_BUFFER );
			fclose(yyin);
			yy_switch_to_buffer(
                 pc->include_stack[pc->include_stack_ptr] );
            }
        }


"//"+[^\n]*		;
"/*"			BEGIN(comment);
<comment>[^*\n]*	;
<comment>"*"+[^*/\n]*   ;
<comment>"*"+"/"	BEGIN(INITIAL);
<comment>\n		LINE++;

theme			return THEME;
frame			return FRAME;
decor			return DECOR;
space			return SPACE;
geometry		return GEOMETRY;
material		return MATERIAL;
gradient		return GRADIENT;
context			return CONTEXT;
cursor			return CURSOR;
style			return STYLE;
up			return UP;
anchor			return ANCHOR;
opacity			return OPACITY;
shapeof			return SHAPEOF;
texture			return TEXTURE;
image			return IMAGE;
to			return TO;
box			return BOX;
NORTH			return NORTH;
NORTHEAST		return NORTHEAST;
EAST			return EAST;
SOUTHEAST		return SOUTHEAST;
SOUTH			return SOUTH;
SOUTHWEST		return SOUTHWEST;
WEST			return WEST;
NORTHWEST		return NORTHWEST;
NONE			return NONE;
UNCHANGED		return UNCHANGED;

0x[0-9A-Z]+		yylval->integer = strtol(yytext, (char **)NULL, 16); return NUMBER;
[0-9]+			yylval->integer = atoi(yytext); return NUMBER;
[a-zA-Z_][a-zA-Z0-9_]*	yylval->string = g_strdup(yytext); return ID;
\".*\" {
			yylval->string = g_strdup(yytext+1); 
			yylval->string[strlen(yylval->string)-1] = 0;
			return STRING;
		}
\`[^`]*\` {
			yylval->string = g_strdup(yytext+1); 
			yylval->string[strlen(yylval->string)-1] = 0;
			return STRING;
		}

\$[a-zA-Z_][a-zA-Z0-9_]*	yylval->string = g_strdup(yytext+1); return ID;
\$[0-9]*		yylval->integer = strtol(yytext + 1, (char **)NULL, 16); return SUBST;
\@			return AT;
:			return COLON;
;			return SEMICOLON;
"+"			return PLUS;
"-"			return MINUS;
"}"			return RCB;
"{"			return LCB;
")"			return RB;
"("			return LB;
"<-"			return LEFT_ARROW;
"->"			return RIGHT_ARROW;
"<->"			return DOUBLE_ARROW;
"."			return DOT;
"*"			return STAR;
"/"			return SLASH;
","			return COMMA;
\n			LINE++; /* zap EOL */
[ \t]+			; /* and whitespace */
.			return 0;
%%
