#include <GL/gl.h>
#include <glib.h>
#include "render.h"
#include "gradient.h"
#include "../kernel/openbox.h"
#include "color.h"

void render_gl_gradient(Surface *sf, int x, int y, int w, int h)
{
    float pr,pg,pb;
    float sr, sg, sb;
    float ar, ag, ab;

    pr = (float)sf->data.planar.primary->r/255.0;
    pg = (float)sf->data.planar.primary->g/255.0;
    pb = (float)sf->data.planar.primary->b/255.0;
    if (sf->data.planar.secondary) {
        sr = (float)sf->data.planar.secondary->r/255.0;
        sg = (float)sf->data.planar.secondary->g/255.0;
        sb = (float)sf->data.planar.secondary->b/255.0;
    }
    switch (sf->data.planar.grad) {
    case Background_Solid: /* already handled */
        glBegin(GL_TRIANGLES);
        glColor3f(pr, pg, pb);
        glVertex2i(x, y);
        glVertex2i(x+w, y);
        glVertex2i(x+w, y+h);

        glVertex2i(x+w, y+h);
        glVertex2i(x, y+h);
        glVertex2i(x, y);
        glEnd();
        return;
    case Background_Horizontal:
        glBegin(GL_TRIANGLES);
        glColor3f(pr, pg, pb);
        glVertex2i(x, y);
        glColor3f(sr, sg, sb);
        glVertex2i(x+w, y);
        glVertex2i(x+w, y+h);

        glVertex2i(x+w, y+h);
        glColor3f(pr, pg, pb);
        glVertex2i(x, y+h);
        glVertex2i(x, y);
        glEnd();
        break;
    case Background_Vertical:
        glBegin(GL_TRIANGLES);
        glColor3f(pr, pg, pb);
        glVertex2i(x, y);
        glVertex2i(x+w, y);
        glColor3f(sr, sg, sb);
        glVertex2i(x+w, y+h);

        glVertex2i(x+w, y+h);
        glVertex2i(x, y+h);
        glColor3f(pr, pg, pb);
        glVertex2i(x, y);
        glEnd();
        break;
    case Background_Diagonal:
	ar = (pr + sr) / 2.0;
	ag = (pg + sg) / 2.0;
	ab = (pb + sb) / 2.0;
        glBegin(GL_TRIANGLES);
        glColor3f(ar, ag, ab);
        glVertex2i(x, y);
        glColor3f(pr, pg, pb);
        glVertex2i(x+w, y);
        glColor3f(ar, ag, ab);
        glVertex2i(x+w, y+h);

        glColor3f(ar, ag, ab);
        glVertex2i(x+w, y+h);
        glColor3f(sr, sg, sb);
        glVertex2i(x, y+h);
        glColor3f(ar, ag, ab);
        glVertex2i(x, y);
        glEnd();
        break;
    case Background_CrossDiagonal:
	ar = (pr + sr) / 2.0;
	ag = (pg + sg) / 2.0;
	ab = (pb + sb) / 2.0;
        glBegin(GL_TRIANGLES);
        glColor3f(pr, pg, pb);
        glVertex2i(x, y);
        glColor3f(ar, ag, ab);
        glVertex2i(x+w, y);
        glColor3f(sr, sg, sb);
        glVertex2i(x+w, y+h);

        glColor3f(sr, sg, sb);
        glVertex2i(x+w, y+h);
        glColor3f(ar, ag, ab);
        glVertex2i(x, y+h);
        glColor3f(pr, pg, pb);
        glVertex2i(x, y);
        glEnd();
        break;
    case Background_Pyramid:
	ar = (pr + sr) / 2.0;
	ag = (pg + sg) / 2.0;
	ab = (pb + sb) / 2.0;
        glBegin(GL_TRIANGLES);
        glColor3f(pr, pg, pb);
        glVertex2i(x, y);
        glColor3f(sr, sg, sb);
        glVertex2i(x+w/2, y+h/2);
        glColor3f(ar, ag, ab);
        glVertex2i(x, y+h/2);

        glVertex2i(x, y+h/2);
        glColor3f(sr, sg, sb);
        glVertex2i(x+w/2, y+h/2);
        glColor3f(pr, pg, pb);
        glVertex2i(x, y+h);

        glVertex2i(x, y+h);
        glColor3f(sr, sg, sb);
        glVertex2i(x+w/2, y+h/2);
        glColor3f(ar, ag, ab);
        glVertex2i(x+w/2, y+h);

        glVertex2i(x+w/2, y+h);
        glColor3f(sr, sg, sb);
        glVertex2i(x+w/2, y+h/2);
        glColor3f(pr, pg, pb);
        glVertex2i(x+w, y+h);

        glVertex2i(x+w, y+h);
        glColor3f(sr, sg, sb);
        glVertex2i(x+w/2, y+h/2);
        glColor3f(ar, ag, ab);
        glVertex2i(x+w, y+h/2);

        glVertex2i(x+w, y+h/2);
        glColor3f(sr, sg, sb);
        glVertex2i(x+w/2, y+h/2);
        glColor3f(pr, pg, pb);
        glVertex2i(x+w, y);

        glVertex2i(x+w, y);
        glColor3f(sr, sg, sb);
        glVertex2i(x+w/2, y+h/2);
        glColor3f(ar, ag, ab);
        glVertex2i(x+w/2, y);

        glVertex2i(x+w/2, y);
        glColor3f(sr, sg, sb);
        glVertex2i(x+w/2, y+h/2);
        glColor3f(pr, pg, pb);
        glVertex2i(x, y);
        glEnd();
        break;
    case Background_PipeCross:
        glBegin(GL_TRIANGLES);
        glColor3f(pr, pg, pb);
        glVertex2i(x, y);
        glColor3f(sr, sg, sb);
        glVertex2i(x+w/2, y+h/2);
        glVertex2i(x, y+h/2);

        glVertex2i(x, y+h/2);
        glVertex2i(x+w/2, y+h/2);
        glColor3f(pr, pg, pb);
        glVertex2i(x, y+h);

        glVertex2i(x, y+h);
        glColor3f(sr, sg, sb);
        glVertex2i(x+w/2, y+h/2);
        glVertex2i(x+w/2, y+h);

        glVertex2i(x+w/2, y+h);
        glVertex2i(x+w/2, y+h/2);
        glColor3f(pr, pg, pb);
        glVertex2i(x+w, y+h);

        glVertex2i(x+w, y+h);
        glColor3f(sr, sg, sb);
        glVertex2i(x+w/2, y+h/2);
        glVertex2i(x+w, y+h/2);

        glVertex2i(x+w, y+h/2);
        glVertex2i(x+w/2, y+h/2);
        glColor3f(pr, pg, pb);
        glVertex2i(x+w, y);

        glVertex2i(x+w, y);
        glColor3f(sr, sg, sb);
        glVertex2i(x+w/2, y+h/2);
        glVertex2i(x+w/2, y);

        glVertex2i(x+w/2, y);
        glVertex2i(x+w/2, y+h/2);
        glColor3f(pr, pg, pb);
        glVertex2i(x, y);
        glEnd();
        break;
    case Background_Rectangle:
        glBegin(GL_TRIANGLES);
        glColor3f(pr, pg, pb);
        glVertex2i(x, y);
        glColor3f(sr, sg, sb);
        glVertex2i(x+w/2, y+h/2);
        glColor3f(pr, pg, pb);
        glVertex2i(x, y+h);

        glVertex2i(x, y+h);
        glColor3f(sr, sg, sb);
        glVertex2i(x+w/2, y+h/2);
        glColor3f(pr, pg, pb);
        glVertex2i(x+w, y+h);

        glVertex2i(x+w, y+h);
        glColor3f(sr, sg, sb);
        glVertex2i(x+w/2, y+h/2);
        glColor3f(pr, pg, pb);
        glVertex2i(x+w, y);

        glVertex2i(x+w, y);
        glColor3f(sr, sg, sb);
        glVertex2i(x+w/2, y+h/2);
        glColor3f(pr, pg, pb);
        glVertex2i(x, y);

        glEnd();
        break;
    default:
        g_message("unhandled gradient");
        return;
    }
}
