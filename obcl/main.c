#include "obcl.h"

int main()
{
    GList *lst = cl_parse("foo.conf");
    cl_print_tree(lst,0);
    return 0;
}
