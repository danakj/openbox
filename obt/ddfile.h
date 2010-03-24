/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-
 
   obt/ddfile.h for the Openbox window manager
   Copyright (c) 2009        Dana Jansens
 
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

#ifndef __obt_ddfile_h
#define __obt_ddfile_h

#include <glib.h>

G_BEGIN_DECLS

typedef enum {
	OBT_DDFILE_TYPE_APPLICATION = 1,
	OBT_DDFILE_TYPE_LINK        = 2,
	OBT_DDFILE_TYPE_DIRECTORY   = 3
} ObtDDFileType;

typedef enum {
	OBT_DDFILE_APP_STARTUP_NO_SUPPORT,
	OBT_DDFILE_APP_STARTUP_PROTOCOL_SUPPORT,
	OBT_DDFILE_APP_STARTUP_LEGACY_SUPPORT
} ObtDDFileAppStartup;

typedef enum {
	/*! The app can be launched with a single local file */
	OBT_DDFILE_APP_SINGLE_LOCAL = 1 << 0,
	/*! The app can be launched with multiple local files */
	OBT_DDFILE_APP_MULTI_LOCAL  = 1 << 1,
	/*! The app can be launched with a single URL */
	OBT_DDFILE_APP_SINGLE_URL   = 1 << 2,
	/*! The app can be launched with multiple URLs */
	OBT_DDFILE_APP_MULTI_URL    = 1 << 3
} ObtDDFileAppOpen;

typedef struct _ObtDDFile     ObtDDFile;

ObtDDFile* obt_ddfile_new_from_file(const gchar *name, GSList *paths);

void obt_ddfile_ref(ObtDDFile *e);
void obt_ddfile_unref(ObtDDFile *e);

/*! Returns TRUE if the file exists but says it should be ignored, with
    the Hidden flag.  No other functions can be used for the ObtDDFile
    in this case. */
gboolean obt_ddfile_deleted (ObtDDFile *e);

/*! Returns the type of object refered to by the .desktop file. */
ObtDDFileType obt_ddfile_type (ObtDDFile *e);

/*! Returns TRUE if the .desktop file should be displayed to users, given the
    current	environment.  If FALSE,	the .desktop file should not be showed.
	This also uses the TryExec option if it is present.
    @env A semicolon-deliminated list of environemnts.  Can be one or more of:
         GNOME, KDE, ROX, XFCE.  Other environments not listed here may also
         be supported.  This can be null also if not listing any environment. */
gboolean obt_ddfile_display(ObtDDFile *e, const gchar *env);

const gchar* obt_ddfile_name           (ObtDDFile *e);
const gchar* obt_ddfile_generic_name   (ObtDDFile *e);
const gchar* obt_ddfile_comment        (ObtDDFile *e);
/*! Returns the icon for the object referred to by the .desktop file.
    Returns either an absolute path, or a string which can be used to find the
    icon using the algorithm given by:
    http://freedesktop.org/wiki/Specifications/icon-theme-spec?action=show&redirect=Standards/icon-theme-spec
*/
const gchar* obt_ddfile_icon           (ObtDDFile *e);

const gchar *obt_ddfile_link_url(ObtDDFile *e);

const gchar*  obt_ddfile_app_executable      (ObtDDFile *e);
/*! Returns the path in which the application should be run */
const gchar*  obt_ddfile_app_path            (ObtDDFile *e);
gboolean      obt_ddfile_app_run_in_terminal (ObtDDFile *e);
const gchar** obt_ddfile_app_mime_types      (ObtDDFile *e);
/*! Returns a combination of values in the ObtDDFileAppOpen enum,
    specifying if the application can be launched to open one or more files
    and URLs. */
ObtDDFileAppOpen obt_ddfile_app_open(ObtDDFile *e);

ObtDDFileAppStartup obt_ddfile_app_startup_notify(ObtDDFile *e);
const gchar* obt_ddfile_app_startup_wmclass(ObtDDFile *e);


G_END_DECLS

#endif
