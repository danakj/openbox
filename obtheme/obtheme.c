#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <glib.h>
#include "obtheme.h"
#include "geom.h"

static double variable_lookup(struct variable *in)
{
	if (strcmp("client", in->base) == 0) {
		if (strcmp("width", in->member) == 0) {
			return 640;
		} else
			return 480;
	}
	if (strcmp("font", in->base) == 0) {
		return 15.0;
	}
	return 0.0;
}

static double expression_eval(struct expression *in)
{
	switch (in->op) {
		case OB_THEME_INV:
			return - expression_eval(in->a);
			break;
		case OB_THEME_MUL:
			return expression_eval(in->a) * expression_eval(in->b);
			break;
		case OB_THEME_DIV:
			return expression_eval(in->a) / expression_eval(in->b);
			break;
		case OB_THEME_ADD:
			return expression_eval(in->a) + expression_eval(in->b);
			break;
		case OB_THEME_SUB:
			return expression_eval(in->a) - expression_eval(in->b);
			break;
		case OB_THEME_EQL:
			if (in->v.base == NULL) {
				return in->v.number;
			} else return variable_lookup(&in->v);
			break;
		default:
			assert(!!!"OH NOES!!!");
	}
	return 0;
}

static void decor_print(gpointer data, gpointer user_data)
{
	struct decor *decor = data;
	printf("    decor id %s\n", decor->name);
printf("   anchor.x = %f\n", expression_eval(&decor->space.anchor.x));
	if (decor->texture.present) {
		if (decor->texture.internal)
			printf("    texture internal: %s\n", decor->texture.name);
		else
			printf("    texture file: %s\n", decor->texture.name);
	}
//	printf("      anchor (%d %d %d)\n", decor->space.anchor.x, decor->space.anchor.y, decor->space.anchor.z);
//	printf("      up     (%d %d %d)\n", decor->space.up.x, decor->space.up.y, decor->space.up.z);
	if (decor->children)
		g_slist_foreach(decor->children, decor_print, NULL);
}

static void style_print(gpointer key, gpointer value, gpointer user_data)
{
	char *stylename = key;
	struct style *style = value;

	printf("  style %s\n", stylename);
	g_slist_foreach(style->tree, decor_print, NULL);
}

static void theme_print(gpointer key, gpointer value, gpointer user_data)
{
	char *name = key;
	struct theme *thm = value;
	printf("name = %s\n", name);
	g_hash_table_foreach(thm->styles, style_print, NULL);
}

static void material_print(gpointer key, gpointer value, gpointer user_data)
{
	char *name = key;
	struct material *mat = value;
	printf("name = %s\n", name);
	printf(" opacity = %f\n", mat->opacity);
}

static void decor_render(gpointer data, gpointer user_data)
{
	struct decor *decor = data;
	struct box *box = &decor->geometry.box;
	struct vector *a = &decor->space.anchor;
	double x, y;

	printf("%s:", decor->name);
	x = expression_eval(&a->x);
	y = expression_eval(&a->y);
	printf("rectangle(%f %f) - (%f %f)\n", x + expression_eval(&box->start.x), y + expression_eval(&box->start.y),
				    x + expression_eval(&box->end.x), y + expression_eval(&box->end.y));
	if (decor->children)
		g_slist_foreach(decor->children, decor_render, NULL);
}

struct boundstuff {
	Rect r;
	Strut s;
};

static void decor_bound(gpointer data, gpointer user_data)
{
	struct boundstuff *bs = user_data;
	struct decor *decor = data;
	struct box *box = &decor->geometry.box;
	struct vector *a = &decor->space.anchor;
	Rect newrect, temprect;
	double x1, y1, x2, y2;

	x1 = expression_eval(&a->x);
	y1 = expression_eval(&a->y);
	x2 = x1;
	y2 = y1;
	x1 += expression_eval(&box->start.x);
	y1 += expression_eval(&box->start.y);
	x2 += expression_eval(&box->end.x);
	y2 += expression_eval(&box->end.y);
	RECT_SET(temprect, x1, y1, x2-x1, y2-y2);
	RECT_ADD(newrect, temprect, bs->r);
	bs->r = newrect;

	if (decor->children)
		g_slist_foreach(decor->children, decor_bound, bs);
}

void obtheme_calc_bound(struct theme *thm, char *name, Rect *r, Strut *s)
{
	struct style *sty;
	struct boundstuff bs;

	RECT_SET(bs.r, 0, 0 ,0 ,0);
	sty = g_hash_table_lookup(thm->styles, name);
	assert(sty);
	g_slist_foreach(sty->tree, decor_bound, &bs);
	*r = bs.r;
}

void obtheme_decorate_window(struct theme *thm, char *name)
{
	struct style *sty;
	sty = g_hash_table_lookup(thm->styles, name);
	g_slist_foreach(sty->tree, decor_render, NULL);
}
