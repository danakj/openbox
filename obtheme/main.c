#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <glib.h>
#include "obtheme.h"

struct obthemedata themedata;

int main(int argc, char **argv)
{
	int err;
	struct theme *thm;
	Rect br;
	Strut bstrut;

	themedata.themes = g_hash_table_new(g_str_hash, g_str_equal);
	themedata.materials = g_hash_table_new(g_str_hash, g_str_equal);
	err = obtheme_parse(&themedata, argv[1]);
	if (err) {
		printf("Fix the script\n");
		exit(1);
	}
	printf("err = %d\n", err);

//	g_hash_table_foreach(themedata.materials, material_print, NULL);

//	g_hash_table_foreach(themedata.themes, theme_print, NULL);

	thm = g_hash_table_lookup(themedata.themes, "awesome");
	obtheme_decorate_window(thm, "regular_window");
	obtheme_calc_bound(thm, "regular_window", &br, &bstrut);
	printf("bounding rectangle: (%d %d) - (%d %d)\n", br.x, br.y, br.x + br.width, br.y + br.height);

	return 0;
}
