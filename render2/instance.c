#include "instance.h"
#include <stdlib.h>
#include <assert.h>

/*
void truecolor_startup(void)
{
  unsigned long red_mask, green_mask, blue_mask;
  XImage *timage = NULL;

  timage = XCreateImage(ob_display, render_visual, render_depth,
                        ZPixmap, 0, NULL, 1, 1, 32, 0);
  assert(timage != NULL);
  /\* find the offsets for each color in the visual's masks *\/
  render_red_mask = red_mask = timage->red_mask;
  render_green_mask = green_mask = timage->green_mask;
  render_blue_mask = blue_mask = timage->blue_mask;

  render_red_offset = 0;
  render_green_offset = 0;
  render_blue_offset = 0;

  while (! (red_mask & 1))   { render_red_offset++;   red_mask   >>= 1; }
  while (! (green_mask & 1)) { render_green_offset++; green_mask >>= 1; }
  while (! (blue_mask & 1))  { render_blue_offset++;  blue_mask  >>= 1; }

  render_red_shift = render_green_shift = render_blue_shift = 8;
  while (red_mask)   { red_mask   >>= 1; render_red_shift--;   }
  while (green_mask) { green_mask >>= 1; render_green_shift--; }
  while (blue_mask)  { blue_mask  >>= 1; render_blue_shift--;  }
  XFree(timage);
}
*/

struct RrInstance *RrInstanceNew(Display *display,
                                 int screen,
                                 XVisualInfo visinfo)
{
    struct RrInstance *inst;

    inst = malloc(sizeof(struct RrInstance));
    inst->display = display;
    inst->screen = screen;
    inst->visinfo = visinfo;
    inst->cmap = XCreateColormap(display, RootWindow(display, screen),
                                 RrVisual(inst), AllocNone);
    inst->glx_context = glXCreateContext(display, &visinfo, NULL, True);

    assert(inst->glx_context);

    return inst;
}

void RrInstanceFree(struct RrInstance *inst)
{
    if (inst) {
        glXDestroyContext(inst->display, inst->glx_context);
        XFreeColormap(inst->display, inst->cmap);
        free(inst);
    }
}
