#include <stdio.h>
#include <stdarg.h>

void GlftDebug(char *a, ...)
{
#ifdef DEBUG
    va_list vl;
    va_start(vl, a);
    vprintf(a, vl);
#endif
}
