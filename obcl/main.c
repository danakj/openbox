#include "obcl.h"

int main()
{
    GList *lst = cl_parse("foo.conf");
    cl_tree_print(lst,0);
    cl_tree_free(lst);
    return 0;
}
