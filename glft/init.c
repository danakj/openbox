#include "glft.h"
#include "font.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include <fontconfig/fcfreetype.h>

FcBool init_done = 0;
FT_Library ft_lib;

FcObjectType objs[] = {
    { GLFT_SHADOW,        FcTypeBool    },
    { GLFT_SHADOW_OFFSET, FcTypeInteger },
    { GLFT_SHADOW_ALPHA,  FcTypeDouble  }
};

#define NUM_OBJS (sizeof(objs) / sizeof(objs[0]))

FcBool GlftInit()
{
    if ((init_done = (FcInit() && !FT_Init_FreeType(&ft_lib))))
        FcNameRegisterObjectTypes(objs, NUM_OBJS);
    return init_done;
}
