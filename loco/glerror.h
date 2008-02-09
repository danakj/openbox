/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   glerror.h for the Openbox compositor
   Copyright (c) 2008        Derek Foreman
   Copyright (c) 2008        Dana Jansens

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   See the COPYING file for a copy of the GNU General Public License.
*/

#ifndef loco__glerror_h
#define loco__glerror_h

#include <GL/glx.h>

#define glError()                                      \
{                                                      \
    /*const GLchar *err_file = strrchr(err_path, '/');*/ \
    GLenum        gl_error = glGetError();             \
                                                       \
    /*++err_file;*/                                        \
                                                       \
    for (; (gl_error); gl_error = glGetError())        \
      g_print("%s: %d caught at line %u\n",    \
              __FUNCTION__, gl_error, __LINE__);       \
              /*(const GLchar*)gluErrorString(gl_error)*/ \
}

#endif
