/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   parse.h for the Openbox window manager
   Copyright (c) 2003        Ben Jansens

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

#ifndef __parse_h
#define __parse_h

#include "version.h"

#include <libxml/parser.h>
#include <glib.h>

G_BEGIN_DECLS

typedef struct _ObParseInst ObParseInst;

typedef void (*ParseCallback)(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node,
                              gpointer data);

ObParseInst* parse_startup();
void parse_shutdown(ObParseInst *inst);

/* Loads Openbox's rc, from the normal paths */
gboolean parse_load_rc(xmlDocPtr *doc, xmlNodePtr *root);
/* Loads an Openbox menu, from the normal paths */
gboolean parse_load_menu(const gchar *file, xmlDocPtr *doc, xmlNodePtr *root);

void parse_register(ObParseInst *inst, const gchar *tag,
                    ParseCallback func, gpointer data);
void parse_tree(ObParseInst *inst, xmlDocPtr doc, xmlNodePtr node);


/* open/close */

gboolean parse_load(const gchar *path, const gchar *rootname,
                    xmlDocPtr *doc, xmlNodePtr *root);
gboolean parse_load_mem(gpointer data, guint len, const gchar *rootname,
                        xmlDocPtr *doc, xmlNodePtr *root);
void parse_close(xmlDocPtr doc);


/* helpers */

xmlNodePtr parse_find_node(const gchar *tag, xmlNodePtr node);

gchar *parse_string(xmlDocPtr doc, xmlNodePtr node);
gint parse_int(xmlDocPtr doc, xmlNodePtr node);
gboolean parse_bool(xmlDocPtr doc, xmlNodePtr node);

gboolean parse_contains(const gchar *val, xmlDocPtr doc, xmlNodePtr node);
gboolean parse_attr_contains(const gchar *val, xmlNodePtr node,
                             const gchar *name);

gboolean parse_attr_string(const gchar *name, xmlNodePtr node, gchar **value);
gboolean parse_attr_int(const gchar *name, xmlNodePtr node, gint *value);

/* paths */

void parse_paths_startup();
void parse_paths_shutdown();

const gchar* parse_xdg_config_home_path();
const gchar* parse_xdg_data_home_path();
GSList* parse_xdg_config_dir_paths();
GSList* parse_xdg_data_dir_paths();

/*! Expands the ~ character to the home directory throughout the given
  string */
gchar *parse_expand_tilde(const gchar *f);
/*! Makes a directory */
gboolean parse_mkdir(const gchar *path, gint mode);
/*! Makes a directory and all its parents */
gboolean parse_mkdir_path(const gchar *path, gint mode);

G_END_DECLS

#endif
