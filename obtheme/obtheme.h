#ifndef __THEME_PARSE_H__
#define __THEME_PARSE_H__

#include <glib.h>
#include "frame.h"
#include "misc.h"

#undef YY_DECL
#define YY_DECL int obthemelex(YYSTYPE *yylval, struct parser_control *pc)

#define YYDEBUG 1
#define YYLEX_PARAM pc

#define MAX_INCLUDE_DEPTH 32
#define LINE pc->currline[pc->include_stack_ptr]

extern int themedebug;

struct boundrect {
	double x1, y1, x2, y2;
};

typedef enum {
	OB_THEME_INV,
	OB_THEME_ADD,
	OB_THEME_SUB,
	OB_THEME_MUL,
	OB_THEME_DIV,
	OB_THEME_EQL
} ObThemeOp;

struct variable {
	char *base;
	char *member;
	double number;
};

struct expression {
	ObThemeOp op;
	struct expression *a;
	struct expression *b;
	struct variable v;
};

struct material {
	float opacity;
};

struct style {
	char *name;
	GSList *tree;
};

struct obthemedata {
	GHashTable *themes;
	GHashTable *materials;
};

struct theme {
	char *name;
	GHashTable *styles;
};

struct vector {
	struct expression x;
	struct expression y;
	struct expression z;
};

struct space {
	struct vector anchor;
	struct vector up;
};

struct texture {
	gboolean present;
	gboolean internal;
	char *name;
};

struct box {
	struct vector start;
	struct vector end;
};

struct geometry {
	struct box box;
};

struct decor {
	char *name;
	GSList *children;
	struct space space;
	ObDirection cursor;
	ObFrameContext context;
	struct texture texture;
	struct material *material;
	struct geometry geometry;
};

struct parser_control {
	struct yy_buffer_state *include_stack[MAX_INCLUDE_DEPTH];
	int currline[MAX_INCLUDE_DEPTH];
	char currfile[MAX_INCLUDE_DEPTH][501];
	int include_stack_ptr;
	char error_buf[4096];
	struct obthemedata *target;
};

void obthemeerror(struct parser_control *pc, char *s);
int obtheme_parse(struct obthemedata *td, const char *filename);
struct parser_control *parser_init(struct obthemedata *td);
int obthemeparse(struct parser_control *);
void parser_finish(struct parser_control *);
void obtheme_calc_bound(struct theme *thm, char *name, Rect *r, Strut *s);
void obtheme_decorate_window(struct theme *thm, char *name);

#endif /* __THEME_PARSE_H__ */
