#include <stdio.h>
#include <stdarg.h>

void RrDebug(char *a, ...)
{
#ifdef DEBUG
    va_list vl;
    va_start(vl, a);
    vprintf(a, vl);
#endif
}
