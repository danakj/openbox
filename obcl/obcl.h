#ifndef __obcl_h
#define __obcl_h

#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <stdarg.h>

/* TEH MACROS FROM MOUNT DOOM */

#define CL_IS_NODE(X) \
    (((CLNode*)(X))->type == CL_LIST || \
     ((CLNode*)(X))->type == CL_BLOCK || \
     ((CLNode*)(X))->type == CL_LISTBLOCK)
#define CL_NODE(X)    ((CLNode*)(X))
#define CL_ID(X)      (((CLNode*)(X))->u.lb.id)
#define CL_LIST(X)    (((CLNode*)(X))->u.lb.list)
#define CL_BLOCK(X)   (((CLNode*)(X))->u.lb.block)
#define CL_NUMVAL(X)  (((CLNode*)(X))->u.num)
#define CL_STRVAL(X)  (((CLNode*)(X))->u.str)
#define CL_LINE(X)    (((CLNode*)(X))->lineno)

#define CL_ASSERT_NODE(X) \
    g_assert(CL_IS_NODE(X))
#define CL_ASSERT_NUM(X) \
    g_assert(((CLNode*)(X))->type == CL_NUM)
#define CL_ASSERT_STR(X) \
    g_assert(((CLNode*)(X))->type == CL_STR)

#define CL_LIST_NTH(X,Y)\
    CL_NODE(g_list_nth(CL_LIST(X),(Y))->data)

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
    int lineno;
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

typedef void (*CLProcFunc)(CLNode *);

struct CLProcHandler;

typedef struct CLProc {
    GHashTable *table;
    struct CLProcHandler *default_h;
} CLProc;

typedef enum CLProcHandlerType {
    CLPROC_FUNC,
    CLPROC_PROC
} CLProcHandlerType;

typedef struct CLProcHandler {
    CLProcHandlerType type;
    union {
        CLProcFunc func;
        CLProc *proc;
    } u;
} CLProcHandler;

GList *cl_parse(gchar *file);
GList *cl_parse_fh(FILE *file);

void cl_tree_free(GList *tree);
void cl_tree_print(GList *tree, int depth);

CLProcHandler *cl_proc_handler_new_func(CLProcFunc f);
CLProcHandler *cl_proc_handler_new_proc(CLProc *cp);
CLProc *cl_proc_new(void);
void cl_proc_free(CLProc *proc);
void cl_proc_add_handler(CLProc *proc, gchar *str,
                         CLProcHandler *handler);
void cl_proc_add_handler_func(CLProc *proc, gchar *str,
                              CLProcFunc func);
void cl_proc_set_default(CLProc *proc, CLProcHandler *pf);
void cl_proc_register_keywords(CLProc *proc, ...);
void cl_process(GList *tree, CLProc *proc);

#endif /* __obcl_h */
