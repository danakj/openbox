#include "cwmcc_internal.h"
#include "atom.h"

Display *cwmcc_display;

void cwmcc_startup(Display *d)
{
    cwmcc_display = d;
    atom_startup();
}
