#include "obcl.h"

void process_foo(CLNode *node)
{
    if (CL_IS_NODE(node)) {
        printf("foo name: %s\n"
               "foo age: %.2f\n",
               CL_STRVAL(CL_LIST_NTH(node,0)),
               CL_NUMVAL(CL_LIST_NTH(node,1)));
    }
}

void process_bah(CLNode *node)
{
    printf("handling bah\n");
}

int main()
{
    GList *lst = cl_parse("foo.conf");
/*     cl_tree_print(lst,0); */
/*     cl_tree_free(lst); */

    
    CLProc *p = cl_proc_new();
    cl_proc_add_handler_func(p, "foo", process_foo);
    cl_proc_add_handler_func(p, "bah", process_bah);

    cl_process(lst, p);

    return 0;
}
