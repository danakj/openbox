/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   themetoxml.c for the Openbox window manager
   Copyright (c) 2007        Dana Jansens

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

#include "rgb.h"

#include <X11/Xlib.h>
#include <X11/Xresource.h>
#include <glib.h>
#include <libxml/parser.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>

static gboolean read_int(XrmDatabase db, const gchar *rname, gint *value);
static gboolean read_string(XrmDatabase db, const gchar *rname,
                            const gchar **value);
static gboolean read_color(XrmDatabase db, const gchar *rname,
                           gint *r, gint *g, gint *b);

static int parse_inline_number(const char *p)
{
    int neg = 1;
    int res = 0;
    if (*p == '-') {
        neg = -1;
        ++p;
    }
    for (; isdigit(*p); ++p)
        res = res * 10 + *p - '0';
    res *= neg;
    return res;
}

static gchar *create_class_name(const gchar *rname)
{
    gchar *rclass = g_strdup(rname);
    gchar *p = rclass;

    while (TRUE) {
        *p = toupper(*p);
        p = strchr(p+1, '.');
        if (p == NULL) break;
        ++p;
        if (*p == '\0') break;
    }
    return rclass;
}

static gboolean read_int(XrmDatabase db, const gchar *rname, gint *value)
{
    gboolean ret = FALSE;
    gchar *rclass = create_class_name(rname);
    gchar *rettype, *end;
    XrmValue retvalue;
  
    if (XrmGetResource(db, rname, rclass, &rettype, &retvalue) &&
        retvalue.addr != NULL) {
        *value = (gint)strtol(retvalue.addr, &end, 10);
        if (end != retvalue.addr)
            ret = TRUE;
    }

    g_free(rclass);
    return ret;
}

static gboolean read_string(XrmDatabase db, const gchar *rname,
                            const gchar **value)
{
    gboolean ret = FALSE;
    gchar *rclass = create_class_name(rname);
    gchar *rettype;
    XrmValue retvalue;
  
    if (XrmGetResource(db, rname, rclass, &rettype, &retvalue) &&
        retvalue.addr != NULL) {
        *value = g_strstrip(retvalue.addr);
        ret = TRUE;
    }

    g_free(rclass);
    return ret;
}

static gchar hextodec(gchar h)
{
    if (h >= '0' && h <= '9') return h - '0';
    else if (h >= 'a' && h <= 'f') return h - 'a' + 10;
    else if (h >= 'A' && h <= 'F') return h - 'A' + 10;
    return -1;
}

static gboolean parse_color(const gchar *c, gint *r, gint *g, gint *b)
{
    int dig1, dig2, i, color[3];
    int len = strlen(c);

    if (len > 4 && c[0] == 'r' && c[1] == 'g' && c[2] == 'b' && c[3] == ':') {
        c += 4;
        for (i = 0; i < 3; ++i) {
            dig1 = hextodec(c[0]);
            if (c[1] == '/' || (c[1] == '\0' && i == 2)) { dig2 = dig1; c+=2; }
            else { dig2 = hextodec(c[1]); c+=3; }

            if (dig1 < 0 || dig2 < 0) return FALSE;
            
            color[i] = dig1*16 + dig2;
        }
        *r = color[0]; *g = color[1]; *b = color[2];
        return TRUE;
    } else if ((len == 4 || len == 7) && c[0] == '#') {
        c++;
        for (i = 0; i < 3; ++i) {
            dig1 = hextodec(c[0]);
            if (len == 4) { dig2 = dig1; c++; }
            else { dig2 = hextodec(c[1]); c+=2; }

            if (dig1 < 0 || dig2 < 0) return FALSE;
            
            color[i] = dig1*16 + dig2;
        }
        *r = color[0]; *g = color[1]; *b = color[2];
        return TRUE;
    } else {
        int i;

        for (i = 0; colornames[i].name != NULL; ++i) {
            if (!strcmp(colornames[i].name, c)) {
                *r = colornames[i].r;
                *g = colornames[i].g;
                *b = colornames[i].b;
                return TRUE;
            }
        }
    }
    return FALSE;
}

static gboolean read_color(XrmDatabase db, const gchar *rname,
                           gint *r, gint *g, gint *b)
{
    gboolean ret = FALSE;
    gchar *rclass = create_class_name(rname);
    gchar *rettype;
    XrmValue retvalue;
  
    if (XrmGetResource(db, rname, rclass, &rettype, &retvalue) &&
        retvalue.addr != NULL) {
        ret = parse_color(retvalue.addr, r, g, b);
    }

    g_free(rclass);
    return ret;
}

xmlNodePtr go(xmlNodePtr node, const char *s)
{
    xmlNodePtr p;

    for (p = node->children; p; p = p->next) {
        if (!xmlStrcasecmp(p->name, (const xmlChar*) s))
            return p;
    }
    return xmlAddChild(node, xmlNewNode(NULL, (const xmlChar*) s));
}

static gchar number[20];
static inline gchar* NUM(int i) {
    g_snprintf(number, 20, "%d", i); return number;
}
static xmlDocPtr doc;
static xmlNodePtr root;

#define GO1(a) (go(root, a))
#define GO2(a,b) (go(GO1(a), b))
#define GO3(a,b,c) (go(GO2(a, b), c))
#define GO4(a,b,c,d) (go(GO3(a, b, c), d))
#define GO5(a,b,c,d,e) (go(GO4(a, b, c, d), e))
#define GO6(a,b,c,d,e,f) (go(GO5(a, b, c, d, e), f))
#define GO7(a,b,c,d,e,f,g) (go(GO6(a, b, c, d, e, f), g))

#define CONT1(a,cont) (xmlNodeSetContent(GO1(a), (const xmlChar*)cont))
#define CONT2(a,b,cont) (xmlNodeSetContent(GO2(a,b), (const xmlChar*)cont))
#define CONT3(a,b,c,cont) (xmlNodeSetContent(GO3(a,b,c), (const xmlChar*)cont))
#define CONT4(a,b,c,d,cont) (xmlNodeSetContent(GO4(a,b,c,d), (const xmlChar*)cont))
#define CONT5(a,b,c,d,e,cont) (xmlNodeSetContent(GO5(a,b,c,d,e), (const xmlChar*)cont))
#define CONT6(a,b,c,d,e,f,cont) (xmlNodeSetContent(GO6(a,b,c,d,e,f), (const xmlChar*)cont))

#define ATTR1(a,name,cont) (xmlSetProp(GO1(a), (const xmlChar*)name, (const xmlChar*)cont))
#define ATTR2(a,b,name,cont) (xmlSetProp(GO2(a,b), (const xmlChar*)name, (const xmlChar*)cont))
#define ATTR3(a,b,c,name,cont) (xmlSetProp(GO3(a,b,c), (const xmlChar*)name, (const xmlChar*)cont))
#define ATTR4(a,b,c,d,name,cont) (xmlSetProp(GO4(a,b,c,d), (const xmlChar*)name, (const xmlChar*)cont))
#define ATTR5(a,b,c,d,e,name,cont) (xmlSetProp(GO5(a,b,c,d,e), (const xmlChar*)name, (const xmlChar*)cont))
#define ATTR6(a,b,c,d,e,f,name,cont) (xmlSetProp(GO6(a,b,c,d,e,f), (const xmlChar*)name, (const xmlChar*)cont))
#define ATTR7(a,b,c,d,e,f,g,name,cont) (xmlSetProp(GO7(a,b,c,d,e,f,g), (const xmlChar*)name, (const xmlChar*)cont))

#define COLOR1(a,R,G,B,A) (ATTR1(a,"r",NUM(R)), \
                           ATTR1(a,"g",NUM(G)), \
                           ATTR1(a,"b",NUM(B)), \
                           ATTR1(a,"a",NUM(A)))
#define COLOR2(a,b,R,G,B,A) (ATTR2(a,b,"r",NUM(R)), \
                             ATTR2(a,b,"g",NUM(G)), \
                             ATTR2(a,b,"b",NUM(B)), \
                             ATTR2(a,b,"a",NUM(A)))
#define COLOR3(a,b,c,R,G,B,A) (ATTR3(a,b,c,"r",NUM(R)), \
                               ATTR3(a,b,c,"g",NUM(G)), \
                               ATTR3(a,b,c,"b",NUM(B)), \
                               ATTR3(a,b,c,"a",NUM(A)))
#define COLOR4(a,b,c,d,R,G,B,A) (ATTR4(a,b,c,d,"r",NUM(R)), \
                                 ATTR4(a,b,c,d,"g",NUM(G)), \
                                 ATTR4(a,b,c,d,"b",NUM(B)), \
                                 ATTR4(a,b,c,d,"a",NUM(A)))
#define COLOR5(a,b,c,d,e,R,G,B,A) (ATTR5(a,b,c,d,e,"r",NUM(R)), \
                                   ATTR5(a,b,c,d,e,"g",NUM(G)), \
                                   ATTR5(a,b,c,d,e,"b",NUM(B)), \
                                   ATTR5(a,b,c,d,e,"a",NUM(A)))
#define COLOR6(a,b,c,d,e,f,R,G,B,A) (ATTR6(a,b,c,d,e,f,"r",NUM(R)), \
                                     ATTR6(a,b,c,d,e,f,"g",NUM(G)), \
                                     ATTR6(a,b,c,d,e,f,"b",NUM(B)), \
                                     ATTR6(a,b,c,d,e,f,"a",NUM(A)))

#define APPEARANCE2(res,a,b) \
{ \
    if (read_string(db, res, &s)) \
        CONT3(a, b, "style", s); \
    if (read_color(db, res".color", &i, &j, &k)) \
        COLOR3(a, b, "primary", i, j, k, 255); \
    if (read_color(db, res".colorTo", &i, &j, &k)) \
        COLOR3(a, b, "secondary", i, j, k, 255); \
    if (read_color(db, res".border.color", &i, &j, &k)) \
        COLOR3(a, b, "border", i, j, k, 255); \
    if (read_color(db, res".interlace.color", &i, &j, &k)) \
        COLOR3(a, b, "interlace", i, j, k, 255); \
}

#define APPEARANCE3(res,a,b,c) \
{ \
    if (read_string(db, res, &s)) \
        CONT4(a, b, c, "style", s); \
    if (read_color(db, res".color", &i, &j, &k)) \
        COLOR4(a, b, c, "primary", i, j, k, 255); \
    if (read_color(db, res".colorTo", &i, &j, &k)) \
        COLOR4(a, b, c, "secondary", i, j, k, 255); \
    if (read_color(db, res".border.color", &i, &j, &k)) \
        COLOR4(a, b, c, "border", i, j, k, 255); \
    if (read_color(db, res".interlace.color", &i, &j, &k)) \
        COLOR4(a, b, c, "interlace", i, j, k, 255); \
}

#define APPEARANCE4(res,a,b,c,d) \
{ \
    if (read_string(db, res, &s)) \
        CONT5(a, b, c, d, "style", s); \
    if (read_color(db, res".color", &i, &j, &k)) \
        COLOR5(a, b, c, d, "primary", i, j, k, 255); \
    if (read_color(db, res".colorTo", &i, &j, &k)) \
        COLOR5(a, b, c, d, "secondary", i, j, k, 255); \
    if (read_color(db, res".border.color", &i, &j, &k)) \
        COLOR5(a, b, c, d, "border", i, j, k, 255); \
    if (read_color(db, res".interlace.color", &i, &j, &k)) \
        COLOR5(a, b, c, d, "interlace", i, j, k, 255); \
}

int main(int argc, char **argv)
{
    XrmDatabase db;
    int i,j,k;
    const gchar *s;
    int ret = 0;

    if (argc > 2) {
        fprintf(stderr, "themetoxml (C) 2007 Dana Jansens\n"
                "This tool takes an older Openbox3 themerc file and prints "
                "out the theme in\n"
                "theme in the newer XML (themerc.xml) style.\n");
        return 0;
    }

    if (argc == 1 || !strcmp(argv[1], "-")) {
        gchar *buf = g_new(gchar, 1000);
        gint sz = 1000;
        gint r = 0, rthis;

        while ((rthis = read(0, buf + r, sz - r)) > 0) {
            r+=rthis;
            if (r==sz) {
                sz+=1000;
                buf = g_renew(gchar,buf,sz);
            }
        }
        

        if ((db = XrmGetStringDatabase(buf)) == NULL) {
            fprintf(stderr, "Unable to read the database from stdin\n");
            return 1;
        }
        g_free(buf);
    }
    else if (argc == 2) {
        if ((db = XrmGetFileDatabase(argv[1])) == NULL) {
            fprintf(stderr, "Unable to read the database from %s\n", argv[1]);
            return 1;
        }
    }


    doc = xmlNewDoc((const xmlChar*) "1.0");
    xmlDocSetRootElement
        (doc,(root = xmlNewNode(NULL, (const xmlChar*)"openbox_theme")));
    xmlSetProp(root, (const xmlChar*)"engine", (const xmlChar*)"box");
    xmlSetProp(root, (const xmlChar*)"version", (const xmlChar*)"1");
    xmlSetProp(root, (const xmlChar*)"xmlns",
               (const xmlChar*)"http://openbox.org/4.0/themerc");
    CONT2("about", "author", "");
    CONT2("about", "email", "");
    CONT2("about", "webpage", "");
    CONT2("about", "comment", "");

    if (read_int(db, "window.handle.width", &i))
        CONT2("dimensions", "handle", NUM(i));

    if (read_int(db, "padding.width", &i)) {
        ATTR2("dimensions", "padding", "x", NUM(i));
        ATTR2("dimensions", "padding", "y", NUM(i));
    }
    
    if (read_int(db, "borderWidth", &i)) {
        CONT3("dimensions", "window", "border", NUM(i));
        CONT3("dimensions", "menu", "border", NUM(i));
        CONT3("dimensions", "osd", "border", NUM(i));
    } else if (read_int(db, "border.width", &i)) {
        CONT3("dimensions", "window", "border", NUM(i));
        CONT3("dimensions", "menu", "border", NUM(i));
        CONT3("dimensions", "osd", "border", NUM(i));
    }
    if (read_int(db, "menu.border.width", &i)) 
        CONT3("dimensions", "menu", "border", NUM(i));
    if (read_int(db, "osd.border.width", &i)) 
        CONT3("dimensions", "osd", "border", NUM(i));

    if (read_color(db, "border.color", &i, &j, &k)) {
        COLOR3("window", "active", "border", i, j, k, 255);
        COLOR3("window", "active", "titleseparator", i, j, k, 255);
        COLOR3("window", "inactive", "border", i, j, k, 255);
        COLOR3("window", "inactive", "titleseparator", i, j, k, 255);
        COLOR2("menu", "border", i, j, k, 255);
    }
    if (read_color(db, "window.active.border.color", &i, &j, &k)) {
        COLOR3("window", "active", "border", i, j, k, 255);
        COLOR3("window", "active", "titleseparator", i, j, k, 255);
    }
    if (read_color(db, "window.active.title.separator.color", &i, &j, &k))
        COLOR3("window", "active", "titleseparator", i, j, k, 255);
    if (read_color(db, "window.inactive.border.color", &i, &j, &k)) {
        COLOR3("window", "inactive", "border", i, j, k, 255);
        COLOR3("window", "inactive", "titleseparator", i, j, k, 255);
    }
    if (read_color(db, "window.inactive.title.separator.color", &i, &j, &k))
        COLOR3("window", "inactive", "titleseparator", i, j, k, 255);
    if (read_color(db, "menu.border.color", &i, &j, &k))
        COLOR2("menu", "border", i, j, k, 255);
    if (read_color(db, "osd.border.color", &i, &j, &k))
        COLOR2("osd", "border", i, j, k, 255);

    if (read_int(db, "window.client.padding.width", &i)) {
        ATTR3("dimensions", "window", "clientpadding", "x", NUM(i));
        ATTR3("dimensions", "window", "clientpadding", "y", NUM(i));
    }

    if (read_string(db, "window.label.text.justify", &s)) {
        if (!g_ascii_strcasecmp(s, "right")) s = "right";
        else if (!g_ascii_strcasecmp(s, "center")) s = "center";
        else s = "left";
        CONT2("window", "justify", s);
    }

    if (read_string(db, "menu.title.text.justify", &s)) {
        if (!g_ascii_strcasecmp(s, "right")) s = "right";
        else if (!g_ascii_strcasecmp(s, "center")) s = "center";
        else s = "left";
        CONT2("menu", "justify", s);
    }

    if (read_int(db, "menu.overlap", &i))
        CONT2("menu", "overlap", NUM(i));

    if (read_color(db, "window.active.client.color", &i, &j, &k))
        COLOR3("window","active","clientpadding",i,j,k,255);
    
    if (read_color(db, "window.inactive.client.color", &i, &j, &k))
        COLOR3("window","inactive","clientpadding",i,j,k,255);

    if (read_color(db, "window.active.label.text.color", &i, &j, &k))
        COLOR5("window","active","label","text","primary",i,j,k,255);

    if (read_color(db, "window.inactive.label.text.color", &i, &j, &k))
        COLOR5("window","inactive","label","text","primary",i,j,k,255);

    if (read_color(db, "window.active.button.unpressed.image.color",
                   &i, &j, &k))
        COLOR5("window","active","buttons","unpressed","image",i,j,k,255);

    if (read_color(db, "window.inactive.button.unpressed.image.color",
                   &i, &j, &k))
        COLOR5("window","inactive","buttons","unpressed","image",i,j,k,255);

    if (read_color(db, "window.active.button.pressed.image.color",
                   &i, &j, &k))
        COLOR5("window","active","buttons","pressed","image",i,j,k,255);

    if (read_color(db, "window.inactive.button.pressed.image.color",
                   &i, &j, &k))
        COLOR5("window","inactive","buttons","pressed","image",i,j,k,255);

    if (read_color(db, "window.active.button.disabled.image.color",
                   &i, &j, &k))
        COLOR5("window","active","buttons","disabled","image",i,j,k,255);

    if (read_color(db, "window.inactive.button.disabled.image.color",
                   &i, &j, &k))
        COLOR5("window","inactive","buttons","disabled","image",i,j,k,255);

    if (read_color(db, "window.active.button.hover.image.color",
                   &i, &j, &k))
        COLOR5("window","active","buttons","hover","image",i,j,k,255);

    if (read_color(db, "window.inactive.button.hover.image.color",
                   &i, &j, &k))
        COLOR5("window","inactive","buttons","hover","image",i,j,k,255);

    if (read_color(db, "window.active.button.toggled.image.color",
                   &i, &j, &k))
        COLOR5("window","active","buttons","toggled-unpressed","image",i,j,k,255);

    if (read_color(db, "window.inactive.button.toggled.image.color",
                   &i, &j, &k))
        COLOR5("window","inactive","buttons","toggled-unpressed","image",i,j,k,255);

    if (read_color(db, "menu.title.text.color",
                   &i, &j, &k))
        COLOR4("menu","title","text","primary",i,j,k,255);

    if (read_color(db, "menu.items.text.color",
                   &i, &j, &k))
        COLOR3("menu","inactive","primary",i,j,k,255);

    if (read_color(db, "menu.items.disabled.text.color",
                   &i, &j, &k)) {
        COLOR3("menu","disabled","primary",i,j,k,255);
        read_color(db, "menu.items.active.disabled.text.color",
                   &i, &j, &k); /* read this if we can */
        COLOR4("menu","active-disabled","text","primary",i,j,k,255);
    }

    if (read_color(db, "menu.items.active.text.color",
                   &i, &j, &k))
        COLOR4("menu","active","text","primary",i,j,k,255);

    if (read_color(db, "osd.label.text.color", &i, &j, &k))
        COLOR4("osd","label","text","primary",i,j,k,255);

    APPEARANCE3("window.active.title.bg", "window", "active", "titlebar");
    APPEARANCE3("window.inactive.title.bg", "window", "inactive", "titlebar");
    APPEARANCE3("window.active.label.bg", "window", "active", "label");
    APPEARANCE3("window.inactive.label.bg", "window", "inactive", "label");
    APPEARANCE3("window.active.handle.bg", "window", "active", "handle");
    APPEARANCE3("window.inactive.handle.bg", "window", "inactive", "handle");
    APPEARANCE3("window.active.grip.bg", "window", "active", "grip");
    APPEARANCE3("window.inactive.grip.bg", "window", "inactive", "grip");
    APPEARANCE2("menu.items.bg", "menu", "entries");
    APPEARANCE2("menu.items.active.bg", "menu", "active");
    APPEARANCE2("menu.items.active.bg", "menu", "active-disabled");
    APPEARANCE2("menu.title.bg", "menu", "title");

    APPEARANCE4("window.active.button.disabled.bg",
                "window", "active", "buttons", "disabled");
    APPEARANCE4("window.inactive.button.disabled.bg",
                "window", "inactive", "buttons", "disabled");
    APPEARANCE4("window.active.button.pressed.bg",
                "window", "active", "buttons", "pressed");
    APPEARANCE4("window.inactive.button.pressed.bg",
                "window", "inactive", "buttons", "pressed");
    APPEARANCE4("window.active.button.toggled.bg",
                "window", "active", "buttons", "toggled-unpressed");
    APPEARANCE4("window.inactive.button.toggled.bg",
                "window", "inactive", "buttons", "toggled-unpressed");
    APPEARANCE4("window.active.button.unpressed.bg",
                "window", "active", "buttons", "unpressed");
    APPEARANCE4("window.inactive.button.unpressed.bg",
                "window", "inactive", "buttons", "unpressed");
    APPEARANCE4("window.active.button.hover.bg",
                "window", "active", "buttons", "hover");
    APPEARANCE4("window.inactive.button.hover.bg",
                "window", "inactive", "buttons", "hover");

    APPEARANCE2("osd.bg", "osd", "background");
    APPEARANCE2("osd.label.bg", "osd", "label");
    APPEARANCE2("osd.hilight.bg", "osd", "hilight");
    APPEARANCE2("osd.unhilight.bg", "osd", "unhilight");

    if (read_string(db, "window.active.label.text.font", &s)) {
        char *p;
        if (strstr(s, "shadow=y")) {
            if ((p = strstr(s, "shadowoffset=")))
                i = parse_inline_number(p + strlen("shadowoffset="));
            else
                i = 1;
            ATTR6("window","active","label","text","shadow","offset",
                     "x",NUM(i));
            ATTR6("window","active","label","text","shadow","offset",
                     "y",NUM(i));
        }
        if ((p = strstr(s, "shadowtint=")))
        {
            i = parse_inline_number(p + strlen("shadowtint="));
            j = (i > 0 ? 0 : 255);
            i = ABS(i*255/100);
            COLOR6("window","active","label","text","shadow","primary",
                   j,j,j,i);
        }
    }

    if (read_string(db, "window.inactive.label.text.font", &s)) {
        char *p;
        if (strstr(s, "shadow=y")) {
            if ((p = strstr(s, "shadowoffset=")))
                i = parse_inline_number(p + strlen("shadowoffset="));
            else
                i = 1;
            ATTR6("window","inactive","label","text","shadow","offset",
                     "x",NUM(i));
            ATTR6("window","inactive","label","text","shadow","offset",
                     "y",NUM(i));
        }
        if ((p = strstr(s, "shadowtint=")))
        {
            i = parse_inline_number(p + strlen("shadowtint="));
            j = (i > 0 ? 0 : 255);
            i = ABS(i*255/100);
            COLOR6("window","inactive","label","text","shadow","primary",
                   j,j,j,i);
        }
    }

    if (read_string(db, "menu.title.text.font", &s)) {
        char *p;
        if (strstr(s, "shadow=y")) {
            if ((p = strstr(s, "shadowoffset=")))
                i = parse_inline_number(p + strlen("shadowoffset="));
            else
                i = 1;
            ATTR5("menu","title","text","shadow","offset","x",NUM(i));
            ATTR5("menu","title","text","shadow","offset","y",NUM(i));
        }
        if ((p = strstr(s, "shadowtint=")))
        {
            i = parse_inline_number(p + strlen("shadowtint="));
            j = (i > 0 ? 0 : 255);
            i = ABS(i*255/100);
            COLOR5("menu","title","text","shadow","primary",j,j,j,i);
        }
    }

    if (read_string(db, "menu.items.font", &s)) {
        char *p;
        if (strstr(s, "shadow=y")) {
            if ((p = strstr(s, "shadowoffset=")))
                i = parse_inline_number(p + strlen("shadowoffset="));
            else
                i = 1;
            ATTR4("menu","inactive","shadow","offset","x",NUM(i));
            ATTR4("menu","inactive","shadow","offset","y",NUM(i));
            ATTR5("menu","active","text","shadow","offset","x",NUM(i));
            ATTR5("menu","active","text","shadow","offset","y",NUM(i));
            ATTR4("menu","disabled","shadow","offset","x",NUM(i));
            ATTR4("menu","disabled","shadow","offset","y",NUM(i));
            ATTR5("menu","active-disabled","text","shadow","offset","x",
                  NUM(i));
            ATTR5("menu","active-disabled","text","shadow","offset","y",
                  NUM(i));
        }
        if ((p = strstr(s, "shadowtint=")))
        {
            i = parse_inline_number(p + strlen("shadowtint="));
            j = (i > 0 ? 0 : 255);
            i = ABS(i*255/100);
            COLOR4("menu","inactive","shadow","primary",j,j,j,i);
            COLOR5("menu","active","text","shadow","primary",j,j,j,i);
            COLOR4("menu","disabled","shadow","primary",j,j,j,i);
            COLOR5("menu","active-disabled","text","shadow","primary",j,j,j,i);
        }
    }

    if (read_string(db, "osd.label.text.font", &s)) {
        char *p;
        if (strstr(s, "shadow=y")) {
            if ((p = strstr(s, "shadowoffset=")))
                i = parse_inline_number(p + strlen("shadowoffset="));
            else
                i = 1;
            ATTR5("osd","label","text","shadow","offset","x",NUM(i));
            ATTR5("osd","label","text","shadow","offset","y",NUM(i));
        }
        if ((p = strstr(s, "shadowtint=")))
        {
            i = parse_inline_number(p + strlen("shadowtint="));
            j = (i > 0 ? 0 : 255);
            i = ABS(i*255/100);
            COLOR5("osd","label","text","shadow","primary",j,j,j,i);
        }
    }

    if (xmlSaveFormatFile("-", doc, 1) < 0) {
        fprintf(stderr, "Error writing the xml tree\n");
        ret = 1;
    }

    xmlFreeDoc(doc);
    return ret;
}
