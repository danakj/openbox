#ifndef __obcl_h
#define __obcl_h

#include <stdio.h>
#include <stdlib.h>
#include <glib.h>

typedef enum CLNodeType {
    CL_ID,
    CL_NUM,
    CL_STR,
    CL_LIST,
    CL_BLOCK,
    CL_LISTBLOCK
} CLNodeType;

typedef struct CLNode {
    CLNodeType type;
    union {
        struct {
            gchar *id;
            GList *list;
            GList *block;
        } lb;
        double num;
        gchar *str;
    } u;

} CLNode;

void free_cl_tree(GList *tree);
GList *cl_parse(gchar *file);
GList *cl_parse_fh(FILE *file);
void cl_print_tree(GList *tree, int depth);

GList *parse_file(FILE *fh);

#endif /* __obcl_h */
