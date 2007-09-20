#include "render.h"
#include "button.h"
#include "instance.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <string.h>

static void RrButtonFreeReal(RrButton* b);

RrButton *RrButtonNew (const RrInstance *inst)
{
    RrButton *out = NULL;

    out = g_new(RrButton, 1);
    out->inst = inst;
    out->ref = 1;

    /* no need to alloc colors, set them null (for freeing later) */
    out->focused_unpressed_color = NULL;
    out->unfocused_unpressed_color = NULL;
    out->focused_pressed_color = NULL;
    out->unfocused_pressed_color = NULL;
    out->disabled_focused_color = NULL;
    out->disabled_unfocused_color = NULL;
    out->hover_focused_color = NULL;
    out->hover_unfocused_color = NULL;
    out->toggled_hover_focused_color = NULL;
    out->toggled_hover_unfocused_color = NULL;
    out->toggled_focused_pressed_color = NULL;
    out->toggled_unfocused_pressed_color = NULL;
    out->toggled_focused_unpressed_color = NULL;
    out->toggled_unfocused_unpressed_color = NULL;

    /* same with masks */
    out->mask = NULL;
    out->pressed_mask = NULL;
    out->disabled_mask = NULL;
    out->hover_mask = NULL;
    out->toggled_mask = NULL;
    out->toggled_hover_mask = NULL;
    out->toggled_pressed_mask = NULL;

    /* allocate appearances */
    out->a_focused_unpressed = RrAppearanceNew(inst, 1);
    out->a_unfocused_unpressed = RrAppearanceNew(inst, 1);
    out->a_focused_pressed = RrAppearanceNew(inst, 1);
    out->a_unfocused_pressed = RrAppearanceNew(inst, 1);
    out->a_disabled_focused = RrAppearanceNew(inst, 1);
    out->a_disabled_unfocused = RrAppearanceNew(inst, 1);
    out->a_hover_focused = RrAppearanceNew(inst, 1);
    out->a_hover_unfocused = RrAppearanceNew(inst, 1);
    out->a_toggled_focused_unpressed = RrAppearanceNew(inst, 1);
    out->a_toggled_unfocused_unpressed = RrAppearanceNew(inst, 1);
    out->a_toggled_focused_pressed = RrAppearanceNew(inst, 1);
    out->a_toggled_unfocused_pressed = RrAppearanceNew(inst, 1);
    out->a_toggled_hover_focused = RrAppearanceNew(inst, 1);
    out->a_toggled_hover_unfocused = RrAppearanceNew(inst, 1);

    return out;
}

void RrButtonFree(RrButton *b)
{
    b->ref--;
    if (b->ref <= 0)
        RrButtonFreeReal(b);
}

void RrButtonFreeReal(RrButton* b)
{
    /* colors */
    if (b->focused_unpressed_color) 
        RrColorFree(b->focused_unpressed_color);
    if (b->unfocused_unpressed_color) 
        RrColorFree(b->unfocused_unpressed_color);
    if (b->focused_pressed_color) 
        RrColorFree(b->focused_pressed_color);
    if (b->unfocused_pressed_color) 
        RrColorFree(b->unfocused_pressed_color);
    if (b->disabled_focused_color) 
        RrColorFree(b->disabled_focused_color);
    if (b->disabled_unfocused_color) 
        RrColorFree(b->disabled_unfocused_color);
    if (b->hover_focused_color) 
        RrColorFree(b->hover_focused_color);
    if (b->hover_unfocused_color) 
        RrColorFree(b->hover_unfocused_color);
    if (b->toggled_hover_focused_color) 
        RrColorFree(b->toggled_hover_focused_color);
    if (b->toggled_hover_unfocused_color) 
        RrColorFree(b->toggled_hover_unfocused_color);
    if (b->toggled_focused_pressed_color) 
        RrColorFree(b->toggled_focused_pressed_color);
    if (b->toggled_unfocused_pressed_color) 
        RrColorFree(b->toggled_unfocused_pressed_color);
    if (b->toggled_focused_unpressed_color) 
        RrColorFree(b->toggled_focused_unpressed_color);
    if (b->toggled_unfocused_unpressed_color) 
        RrColorFree(b->toggled_unfocused_unpressed_color);

    /* masks */
}
