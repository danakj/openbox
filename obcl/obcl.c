#include "obcl.h"

void cl_tree_free(GList *tree)
{
    CLNode *tmp;

    if (!tree) return;

    for (; tree; tree = tree->next) {
        tmp = (CLNode*)tree->data;
        switch(tmp->type) {
        case CL_ID:
        case CL_STR:
            g_free(tmp->u.str);
            break;
        case CL_LIST:
        case CL_BLOCK:
        case CL_LISTBLOCK:
            g_free(tmp->u.lb.id);
            cl_tree_free(tmp->u.lb.list);
            cl_tree_free(tmp->u.lb.block);
            break;
        default:
            break;
        }
        g_free(tmp);
    }
    g_list_free(tree);
}

GList *cl_parse(gchar *file)
{
    FILE *fh = fopen(file, "r");
    GList *ret = NULL;

    if (fh) {
        ret = cl_parse_fh(fh);
        fclose(fh);
    } else {
        perror(file);
    }

    return ret;
}

void cl_tree_print(GList *tree, int depth)
{
    CLNode *tmp;
    int tmpd = depth;

    for (; tree; tree = tree->next) {
        tmp = (CLNode*)tree->data;

        while (tmpd-- > 0)
            printf(" ");
        tmpd = depth;

        switch(tmp->type) {
        case CL_ID:
            printf("--ID-- %s\n", tmp->u.str);
            break;
        case CL_STR:
            printf("--STR-- %s\n", tmp->u.str);
            break;
        case CL_NUM:
            printf("--NUM-- %.2f\n", tmp->u.num);
            break;
        case CL_LIST:
            printf("--LIST-- %s\n", tmp->u.lb.id);
            cl_tree_print(tmp->u.lb.list, depth+2);
            break;
        case CL_BLOCK:
            printf("--BLOCK-- %s\n", tmp->u.lb.id);
            cl_tree_print(tmp->u.lb.block, depth+2);
            break;
        case CL_LISTBLOCK:
            printf("--LISTBLOCK-- %s\n", tmp->u.lb.id);
            cl_tree_print(tmp->u.lb.list, depth+2);
            printf("\n");
            cl_tree_print(tmp->u.lb.block, depth+2);
            break;
        }
    }
}
