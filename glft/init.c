#include "glft.h"

FcBool init_done = 0;

FcBool GlftInit()
{
    return init_done = FcInit();
}
