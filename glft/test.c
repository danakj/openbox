#include "glft.h"
#include <stdio.h>

int main(int argc, char **argv)
{
    struct GlftFont *font;

    if (argc < 2) {
        printf("Usage: %s fontname\n", argv[0]);
        return 1;
    }

    if (!GlftInit()) return 1;

    font = GlftFontOpen(argv[1]);
    GlftFontClose(font);

    return 0;
}
