%pure-parser
%name-prefix "obtheme"
%parse-param {struct parser_control *pc}
%{
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <unistd.h>
#include "frame.h"
#include "misc.h"
#include "render.h"
#include "obtheme.h"
#include "obtheme.tab.h"

YY_DECL;

struct context_table_item {
	char *name;
	ObFrameContext val;
};

/* some of these contexts don't make any sense for a theme... */
static struct context_table_item contexts[OB_FRAME_NUM_CONTEXTS] = {
	{"none", OB_FRAME_CONTEXT_NONE},
	{"desktop", OB_FRAME_CONTEXT_DESKTOP},
	{"root", OB_FRAME_CONTEXT_ROOT},
	{"client", OB_FRAME_CONTEXT_CLIENT},
	{"titlebar", OB_FRAME_CONTEXT_TITLEBAR},
	{"frame", OB_FRAME_CONTEXT_FRAME},
	{"blcorner", OB_FRAME_CONTEXT_BLCORNER},
	{"brcorner", OB_FRAME_CONTEXT_BRCORNER},
	{"tlcorner", OB_FRAME_CONTEXT_TLCORNER},
	{"trcorner", OB_FRAME_CONTEXT_TRCORNER},
	{"top", OB_FRAME_CONTEXT_TOP},
	{"bottom", OB_FRAME_CONTEXT_BOTTOM},
	{"left", OB_FRAME_CONTEXT_LEFT},
	{"right", OB_FRAME_CONTEXT_RIGHT},
	{"maximize", OB_FRAME_CONTEXT_MAXIMIZE},
	{"alldesktops", OB_FRAME_CONTEXT_ALLDESKTOPS},
	{"shade", OB_FRAME_CONTEXT_SHADE},
	{"iconify", OB_FRAME_CONTEXT_ICONIFY},
	{"icon", OB_FRAME_CONTEXT_ICON},
	{"close", OB_FRAME_CONTEXT_CLOSE},
	{"moveresize", OB_FRAME_CONTEXT_MOVE_RESIZE}
};

static ObFrameContext context_from_string(char *str)
{
	int i;
	for (i = 0; i < OB_FRAME_NUM_CONTEXTS; i++)
		if (strcmp(contexts[i].name, str) == 1)
			return contexts[i].val;
	return -1;
}

struct gradient_table_item {
	char *name;
	RrSurfaceColorType val;
};

struct gradient_table_item gradients[RR_SURFACE_NUM_TYPES] = {
	{"none", RR_SURFACE_NONE},
	{"parentrel", RR_SURFACE_PARENTREL},
	{"solid", RR_SURFACE_SOLID},
	{"split", RR_SURFACE_SPLIT_VERTICAL},
	{"horizontal", RR_SURFACE_HORIZONTAL},
	{"vertical", RR_SURFACE_VERTICAL},
	{"diagonal", RR_SURFACE_DIAGONAL},
	{"cross_diagonal", RR_SURFACE_CROSS_DIAGONAL},
	{"pyramid", RR_SURFACE_PYRAMID},
	{"mirror_horizontal", RR_SURFACE_MIRROR_HORIZONTAL},
};

static RrSurfaceColorType gradient_from_string(char *str)
{
	int i;

	for (i = 0; i < RR_SURFACE_NUM_TYPES; i++)
		if (strcmp(gradients[i].name, str) == 1)
			return gradients[i].val;

	return -1;
}

struct parser_control *parser_init(struct obthemedata *otd)
{
	struct parser_control *out;
	out = calloc(1, sizeof(struct parser_control));
	out->include_stack_ptr = 0;
	out->target = otd;
	return out;
}

void parser_finish(struct parser_control *c)
{
	free(c);
}

struct expression *exdup(struct expression in)
{
	struct expression *out;
	out = malloc(sizeof(struct expression));
	*out = in;
	return out;
}

%}
%start theme_objects

%union {
	int integer;
	float realnum;
	char *string;
	struct decor decor;
	struct space space;
	struct theme theme;
	struct material material;
	struct style style;
	GSList *list;
	GHashTable *hash;
	struct vector vector;
	ObCursor dir;
	ObFrameContext context;
	RrSurfaceColorType gradient;
	struct expression expr;
	struct variable var;
	struct texture tex;
	struct material *matptr;
	struct box box;
	struct geometry geometry;
}

%token NORTH NORTHEAST EAST SOUTHEAST SOUTH SOUTHWEST
%token WEST NORTHWEST NONE UNCHANGED
%token LCB RCB LB RB TO
%token LEFT_ARROW RIGHT_ARROW DOUBLE_ARROW
%token SEMICOLON AT COLON DEFAULT NOT
%token PLUS MINUS STAR SLASH COMMA BOX
%token <string> STRING ID
%token <integer> NUMBER SUBST BULK BIG LITTLE
%token THEME FRAME SPACE GEOMETRY MATERIAL GRADIENT
%token CONTEXT CURSOR UP ANCHOR STYLE TEXTURE OPACITY
%token SHAPEOF DECOR DOT IMAGE
%type <decor> decor
%type <decor> decoritems styleitem
%type <space> space
%type <style> style styleitems
%type <hash> styles
%type <material> material_props
%type <realnum> opacity
%type <space> spaceconstraints
%type <vector> up anchor
%type <dir> cursor
%type <context> context
%type <gradient> gradient
%type <expr> expression
%type <var> variable
%type <tex> texture image texitems
%type <matptr> material_use
%type <box> box
%type <geometry> geometry geometry_item
%left PLUS MINUS
%left STAR SLASH
%left LB
%%

theme_object	: material_decl
		| theme
		;

theme_objects	: /* empty */
		| theme_objects theme_object
		;

theme		: THEME ID LCB styles RCB {
			struct theme *out;
			out = malloc(sizeof(struct theme));
			out->name = $2;
			out->styles = $4;
			g_hash_table_insert(pc->target->themes, $2, out);
		}
		;

opacity		: OPACITY LB NUMBER RB { $$ = $3; }
		;

material_props	: /* empty */ { memset(&$$, 0, sizeof($$)); }
		| material_props opacity { $$ = $1; $$.opacity = $2;}
		| material_props gradient
		;

material_decl	: MATERIAL ID LCB material_props RCB {
			struct material *out;
			out = malloc(sizeof(struct material));
			*out = $4;
			g_hash_table_insert(pc->target->materials, $2, out);
		}
		;

anchor		: ANCHOR LB expression COMMA expression COMMA expression RB {
			$$.x = $3;
			$$.y = $5;
			$$.z = $7;
		}
		;

up		: UP LB expression COMMA expression COMMA expression RB {
			$$.x = $3;
			$$.y = $5;
			$$.z = $7;
		}
		;

spaceconstraints: /* empty */ { memset(&$$, 0, sizeof($$));
			$$.anchor.x.op = OB_THEME_EQL;
			$$.anchor.x.v.number = 0;
			$$.anchor.y.op = OB_THEME_EQL;
			$$.anchor.y.v.number = 0;
			$$.anchor.z.op = OB_THEME_EQL;
			$$.anchor.z.v.number = 0;
			$$.up.x.op = OB_THEME_EQL;
			$$.up.x.v.number = 0;
			$$.up.y.op = OB_THEME_EQL;
			$$.up.y.v.number = 1;
			$$.up.z.op = OB_THEME_EQL;
			$$.up.z.v.number = 0;
		}
		| spaceconstraints anchor { $1.anchor = $2; $$ = $1; }
		| spaceconstraints up     { $1.up = $2; $$ = $1; }
		;

space		: SPACE LCB spaceconstraints RCB {
			$$ = $3;
		}
		;

shape		: SHAPEOF LB ID RB
		;

box		: BOX LB expression COMMA expression COMMA expression RB TO
		   LB expression COMMA expression COMMA expression RB {
			$$.start.x = $3;
			$$.start.y = $5;
			$$.start.z = $7;
			$$.end.x = $11;
			$$.end.y = $13;
			$$.end.z = $15;
		}
		;

geometry_item	: /* empty */
		| shape
		| box { $$.box = $1; }
		;

geometry	: GEOMETRY LCB geometry_item RCB {
			$$ = $3;
		}
		;

material_use	: MATERIAL LB ID RB {
			$$ = g_hash_table_lookup(pc->target->materials, $3);
			if ($$ == NULL) {
				snprintf(pc->error_buf, 4000, "No definition for material '%s'\n", $3);
				obthemeerror(pc, pc->error_buf);
				return 1;
			}
		}
		;

styleitem	: decor
		;

styleitems	: /* empty */ { $$.tree = NULL; $$.name = NULL; }
		| styleitems styleitem {
			struct decor *out = malloc(sizeof(struct decor));
			*out = $2;
			$1.tree = g_slist_prepend($1.tree, out);
			$$ = $1;
		}
		;

styles		: /* empty */ { $$ = g_hash_table_new(g_str_hash, g_str_equal); }
		| styles style {
			struct style *out = malloc(sizeof(struct style));
			*out = $2;
			$$ = $1;
			g_hash_table_insert($1, out->name, out);
		}
		;

style		: STYLE ID LCB styleitems RCB {
			$$ = $4;
			$$.name = $2;
		}
		;

decoritems	: /* empty */ {
			memset(&$$, 0, sizeof($$));
//			$$.space.up.y = -1;  XXX need a sane default!
			$$.cursor = OB_CURSOR_NONE;
			$$.space.anchor.x.op = OB_THEME_EQL;
			$$.space.anchor.x.v.number = 1;
			$$.space.anchor.y.op = OB_THEME_EQL;
			$$.space.anchor.y.v.number = 1;
			$$.space.anchor.z.op = OB_THEME_EQL;
			$$.space.anchor.z.v.number = 1;
			$$.space.up.x.op = OB_THEME_EQL;
			$$.space.up.x.v.number = 1;
			$$.space.up.y.op = OB_THEME_EQL;
			$$.space.up.y.v.number = 1;
			$$.space.up.z.op = OB_THEME_EQL;
			$$.space.up.z.v.number = 1;
			$$.texture.present = 0;
			$$.material = NULL;
		}
		| decoritems decor {
			struct decor *out = malloc(sizeof(struct decor));
			*out = $2;
			$$ = $1;
			$$.children = g_slist_append($1.children, out);
		}
		| decoritems space { $1.space = $2; $$ = $1; }
		| decoritems material_use { $1.material = $2; $$ = $1; }
		| decoritems geometry { $1.geometry = $2; $$ = $1;}
		| decoritems texture { $1.texture = $2; $$ = $1; } //XXX multitexture?
		| decoritems context { $1.context = $2; $$ = $1; }
		| decoritems cursor { $1.cursor = $2; $$ = $1; }
		;

		;

decor		: DECOR ID LCB decoritems RCB {
			$$ = $4;
			$$.name = $2;
		}
		;

image		: IMAGE LB STRING RB {
			$$.present = 1;
			$$.internal = 0;
			$$.name = $3;
		}
		| IMAGE LB ID RB {
			$$.present = 1;
			$$.internal = 1;
			$$.name = $3;
		}
		;

texitems	: image { $$ = $1; }
		;

texture		: TEXTURE LCB texitems RCB { $$ = $3; }
		;

context		: CONTEXT LB ID RB {
			ObFrameContext frc;
			frc = context_from_string($3);
			if (frc == -1) {
				snprintf(pc->error_buf, 4000, "Illegal context '%s'\n", $3);
				obthemeerror(pc, pc->error_buf);
				return 1;
			}
		}
		;

cursor		: CURSOR LB NORTH RB { $$ = OB_CURSOR_NORTH; }
		| CURSOR LB NORTHEAST RB { $$ = OB_CURSOR_NORTHEAST; }
		| CURSOR LB EAST RB { $$ = OB_CURSOR_EAST; }
		| CURSOR LB SOUTHEAST RB { $$ = OB_CURSOR_SOUTHEAST; }
		| CURSOR LB SOUTH RB { $$ = OB_CURSOR_SOUTH; }
		| CURSOR LB SOUTHWEST RB { $$ = OB_CURSOR_SOUTHWEST; }
		| CURSOR LB WEST RB { $$ = OB_CURSOR_WEST; }
		| CURSOR LB NORTHWEST RB { $$ = OB_CURSOR_NORTHWEST; }
//		| CURSOR LB NONE RB { $$ = OB_CURSOR_NORTHEAST; }
		| CURSOR LB UNCHANGED RB { $$ = OB_CURSOR_NONE; }
		;
gradient	: GRADIENT LB ID RB {
			$$ = gradient_from_string($3);
			if ($$ == -1) {
				snprintf(pc->error_buf, 4000, "No gradient named '%s'\n", $3);
				obthemeerror(pc, pc->error_buf);
				return 1;
			}
		}
		;

variable	: ID DOT ID { $$.base = $1; $$.member = $3; }
		| ID        { $$.base = $1; $$.member = NULL; }
		| NUMBER    { $$.base = NULL; $$.number = $1; }
		;

expression	: MINUS expression {
			$$.op = OB_THEME_INV;
			$$.a = exdup($2);
		}
		| expression STAR expression {
			$$.op = OB_THEME_MUL;
			$$.a = exdup($1);
			$$.b = exdup($3);
		}
		| expression SLASH expression {
			$$.op = OB_THEME_DIV;
			$$.a = exdup($1);
			$$.b = exdup($3);
		}
		| expression PLUS expression {
			$$.op = OB_THEME_ADD;
			$$.a = exdup($1);
			$$.b = exdup($3);
		}
		| expression MINUS expression {
			$$.op = OB_THEME_SUB;
			$$.a = exdup($1);
			$$.b = exdup($3);
		}
		| variable {
			$$.op = OB_THEME_EQL;
			$$.v = $1;
		}
		| LB expression RB { $$ = $2; }
		;

%%
