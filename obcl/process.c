#include "obcl.h"

static void cl_proc_intern_handler(CLNode *node)
{
    CL_ASSERT_NODE(node);
    g_warning("Unhandled node %s on line %d\n",
              CL_ID(node), CL_LINE(node));
}

static CLProcHandler *default_handler(void)
{
    static CLProcHandler *ph = 0;
    if (!ph)
        ph = cl_proc_handler_new_func(cl_proc_intern_handler);
    return ph;
}

CLProcHandler *cl_proc_handler_new_func(CLProcFunc f)
{
    CLProcHandler *cph = g_new(CLProcHandler,1);
    cph->type = CLPROC_FUNC;
    cph->u.func = f;
    return cph;
}

CLProcHandler *cl_proc_handler_new_proc(CLProc *cp)
{
    CLProcHandler *cph = g_new(CLProcHandler,1);
    cph->type = CLPROC_PROC;
    cph->u.proc = cp;
    return cph;
}

CLProc *cl_proc_new(void)
{
    CLProc *ret = g_new(CLProc,1);
    ret->table = g_hash_table_new(g_str_hash,g_str_equal);
    ret->default_h = default_handler();
    return ret;
}

void cl_proc_free(CLProc *proc)
{

}

void cl_proc_add_handler(CLProc *proc, gchar *str,
                         CLProcHandler *handler)
{
    g_assert(proc != NULL);
    g_hash_table_replace(proc->table, str, handler);
}

void cl_proc_add_handler_func(CLProc *proc, gchar *str,
                              CLProcFunc func)
{
    CLProcHandler *ph;

    g_assert(proc != NULL);
    ph = cl_proc_handler_new_func(func);
    cl_proc_add_handler(proc, str, ph);
}

void cl_proc_set_default(CLProc *proc, CLProcHandler *ph)
{
    g_assert(proc != NULL);
    proc->default_h = ph;
}

void cl_proc_register_keywords(CLProc *proc, ...)
{
    va_list args;
    g_assert(proc != NULL);

    va_start(args,proc);
    for (;;) {
        gchar *k = va_arg(args, gchar*);
        if (k == NULL)
            break;
        if (g_hash_table_lookup(proc->table, k) != NULL)
            g_hash_table_insert(proc->table, k, default_handler());
    }
    va_end(args);
}

void cl_process(GList *tree, CLProc *proc)
{
    GList *lst;
    CLProcHandler *handler;

    g_assert(proc != NULL);

    if (!tree) return;

    for (lst = tree; lst != NULL; lst = lst->next) {
        CL_ASSERT_NODE(lst->data);
        handler = g_hash_table_lookup(proc->table, CL_ID(lst->data));
        if (!handler)
            handler = default_handler();
        if (handler->type == CLPROC_FUNC)
            handler->u.func(CL_NODE(lst->data));
        else
            cl_process(CL_BLOCK(lst->data), handler->u.proc);
    }
}
