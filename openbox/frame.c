#include "frame.h"

Context frame_context_from_string(char *name)
{
    if (!g_ascii_strcasecmp("root", name))
        return Context_Root;
    else if (!g_ascii_strcasecmp("client", name))
        return Context_Client;
    else if (!g_ascii_strcasecmp("titlebar", name))
        return Context_Titlebar;
    else if (!g_ascii_strcasecmp("handle", name))
        return Context_Handle;
    else if (!g_ascii_strcasecmp("frame", name))
        return Context_Frame;
    else if (!g_ascii_strcasecmp("blcorner", name))
        return Context_BLCorner;
    else if (!g_ascii_strcasecmp("tlcorner", name))
        return Context_TLCorner;
    else if (!g_ascii_strcasecmp("brcorner", name))
        return Context_BRCorner;
    else if (!g_ascii_strcasecmp("trcorner", name))
        return Context_TRCorner;
    else if (!g_ascii_strcasecmp("maximize", name))
        return Context_Maximize;
    else if (!g_ascii_strcasecmp("alldesktops", name))
        return Context_AllDesktops;
    else if (!g_ascii_strcasecmp("shade", name))
        return Context_Shade;
    else if (!g_ascii_strcasecmp("iconify", name))
        return Context_Iconify;
    else if (!g_ascii_strcasecmp("icon", name))
        return Context_Icon;
    else if (!g_ascii_strcasecmp("close", name))
        return Context_Close;
    return Context_None;
}

void frame_client_gravity(Frame *self, int *x, int *y)
{
    /* horizontal */
    switch (self->client->gravity) {
    default:
    case NorthWestGravity:
    case SouthWestGravity:
    case WestGravity:
	break;

    case NorthGravity:
    case SouthGravity:
    case CenterGravity:
	*x -= (self->size.left + self->size.right) / 2;
	break;

    case NorthEastGravity:
    case SouthEastGravity:
    case EastGravity:
	*x -= self->size.left + self->size.right;
	break;

    case ForgetGravity:
    case StaticGravity:
	*x -= self->size.left;
	break;
    }

    /* vertical */
    switch (self->client->gravity) {
    default:
    case NorthWestGravity:
    case NorthEastGravity:
    case NorthGravity:
	break;

    case CenterGravity:
    case EastGravity:
    case WestGravity:
	*y -= (self->size.top + self->size.bottom) / 2;
	break;

    case SouthWestGravity:
    case SouthEastGravity:
    case SouthGravity:
	*y -= self->size.top + self->size.bottom;
	break;

    case ForgetGravity:
    case StaticGravity:
	*y -= self->size.top;
	break;
    }
}

void frame_frame_gravity(Frame *self, int *x, int *y)
{
    /* horizontal */
    switch (self->client->gravity) {
    default:
    case NorthWestGravity:
    case WestGravity:
    case SouthWestGravity:
	break;
    case NorthGravity:
    case CenterGravity:
    case SouthGravity:
	*x += (self->size.left + self->size.right) / 2;
	break;
    case NorthEastGravity:
    case EastGravity:
    case SouthEastGravity:
	*x += self->size.left + self->size.right;
	break;
    case StaticGravity:
    case ForgetGravity:
	*x += self->size.left;
	break;
    }

    /* vertical */
    switch (self->client->gravity) {
    default:
    case NorthWestGravity:
    case WestGravity:
    case SouthWestGravity:
	break;
    case NorthGravity:
    case CenterGravity:
    case SouthGravity:
	*y += (self->size.top + self->size.bottom) / 2;
	break;
    case NorthEastGravity:
    case EastGravity:
    case SouthEastGravity:
	*y += self->size.top + self->size.bottom;
	break;
    case StaticGravity:
    case ForgetGravity:
	*y += self->size.top;
	break;
    }
}
