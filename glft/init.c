#include "glft.h"
#include "font.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include <fontconfig/fcfreetype.h>

FcBool init_done = 0;
FT_Library ft_lib;

FcBool GlftInit()
{
    return init_done = (FcInit() && !FT_Init_FreeType(&ft_lib));
}
