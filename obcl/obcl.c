#include "obcl.h"

void free_cl_tree(GList *tree)
{

}

GList *cl_parse(gchar *file)
{
    FILE *fh = fopen(file, "r");
    if (fh)
        return cl_parse_fh(fh);
    else {
        printf("can't open file %s\n", file);
        return 0;
    }
}

void cl_print_tree(GList *tree, int depth)
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
            cl_print_tree(tmp->u.lb.list, depth+2);
            break;
        case CL_BLOCK:
            printf("--BLOCK-- %s\n", tmp->u.lb.id);
            cl_print_tree(tmp->u.lb.block, depth+2);
            break;
        case CL_LISTBLOCK:
            printf("--LISTBLOCK-- %s\n", tmp->u.lb.id);
            cl_print_tree(tmp->u.lb.list, depth+2);
            printf("\n");
            cl_print_tree(tmp->u.lb.block, depth+2);
            break;
        }
    }
}
