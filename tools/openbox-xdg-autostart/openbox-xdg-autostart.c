/*
 *      openbox-xdg-autostart.c - Handle autostart spec of freedesktop.org
 *
 *      Copyright 2008 PCMan <pcman.tw@gmail.com>
 *      Copyright 2014 J. M. Becker <zilla@techzilla.info>
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *      MA 02110-1301, USA.
 */

#include <glib.h>
#include <stdio.h>
#include <string.h>

static const char DesktopEntry[] = "Desktop Entry";
char* de_name;

static void launch_autostart_file( const char* desktop_id, const char* desktop_file, GKeyFile* kf )
{
    if( g_key_file_load_from_file( kf, desktop_file, 0, NULL ) ) {
        char* exec;
        char** only_show_in, **not_show_in;
        gsize n;

        if( g_key_file_get_boolean( kf, DesktopEntry, "Hidden", NULL ) )
            return;

        /* check if this desktop entry is desktop-specific */
        only_show_in = g_key_file_get_string_list( kf, DesktopEntry, "OnlyShowIn", &n, NULL );
        if( only_show_in ) {
            /* The format of this list is like:  OnlyShowIn=GNOME;XFCE */
            gsize i = 0;
            for( i = 0; i < n; ++i ) {
                /* Only start this program if we are in the "OnlyShowIn" list */
                if( 0 == strcmp( de_name, only_show_in[ i ] ) )
                    break;
            }
            if( i >= n ) {  /* our session name is not found in the list */
                g_strfreev( only_show_in );
                return;   /* read next desktop file */
            }
            g_strfreev( only_show_in );
        } else { /* OnlyShowIn and NotShowIn cannot be set at the same time. */
            /* check if this desktop entry is not allowed in our session */
            not_show_in = g_key_file_get_string_list( kf, DesktopEntry, "NotShowIn", &n, NULL );
            if( not_show_in ) {
                /* The format of this list is like:  NotShowIn=KDE;IceWM */
                gsize i = 0;
                for( i = 0; i < n; ++i ) {
                    /* Only start this program if we are in the "OnlyShowIn" list */
                    if( 0 == strcmp( de_name, not_show_in[ i ] ) )
                        break;
                }
                if( i < n ) {  /* our session name is found in the "NotShowIn" list */
                    g_strfreev( not_show_in );
                    return;   /* read next desktop file */
                }
                g_strfreev( not_show_in );
            }
        }

        exec = g_key_file_get_string( kf, DesktopEntry, "TryExec", NULL );
        if( G_UNLIKELY(exec) ) { /* If we are asked to tryexec first */
            if( ! g_path_is_absolute( exec ) ) {
                char* full = g_find_program_in_path( exec );
                g_free( exec );
                exec = full;
            }
            /* If we cannot match the TryExec key with an installed executable program */
            if( ! g_file_test( exec, G_FILE_TEST_IS_EXECUTABLE ) ) {
                g_free( exec );
                return;   /* bypass this desktop file, and read next */
            }
            g_free( exec );
        }

        /* get the real command line */
        exec = g_key_file_get_string( kf, DesktopEntry, "Exec", NULL );
        if( G_LIKELY(exec) ) {
            /* according to the spec, the Exec command line should be translated
             *  with some rules, but that's normally for file managers who needs to
             *  pass selected file as arguments. The probability we need this is
             *  very low, so just omit it.
             */

            /* FIXME: Exec key should be handled correctly */

            /* launch the program */
            if( g_spawn_command_line_async( exec, NULL ) ) {
            }
        }
    }
}

static void get_autostart_files_in_dir( GHashTable* hash, const char* de_name, const char* base_dir )
{
    char* dir_path = g_build_filename( base_dir, "autostart", NULL );
    GDir* dir = g_dir_open( dir_path, 0, NULL );

    if( dir ) {
        char *path;
        const char *name;

        while( (name = g_dir_read_name( dir )) != NULL ) {
            if(g_str_has_suffix(name, ".desktop")) {
                path = g_build_filename( dir_path, name, NULL );
                g_hash_table_replace( hash, g_strdup(name), path );
            }
        }
        g_dir_close( dir );
    }
    g_free( dir_path );
}

void xdg_autostart( const char* de_name )
{
    const char* const *dirs = g_get_system_config_dirs();
    const char* const *dir;
    GHashTable* hash = g_hash_table_new_full( g_str_hash, g_str_equal, g_free, g_free );

    /* get system-wide autostart files */
    for( dir = dirs; *dir; ++dir )
        get_autostart_files_in_dir( hash, de_name, *dir );

    /* get user-specific autostart files */
    get_autostart_files_in_dir( hash, de_name, g_get_user_config_dir() );

    if( g_hash_table_size( hash ) > 0 ) {
        GKeyFile* kf = g_key_file_new();
        g_hash_table_foreach( hash, (GHFunc)launch_autostart_file, kf );
        g_key_file_free( kf );
    }

    g_hash_table_destroy( hash );
}

static void show_help()
{
    g_printf ("Run XDG autostart .desktop files.\n");
    g_printf ("\n");
    g_printf ("Usage:\n");
    g_printf ("  xdg-autostart <DE_NAME>\n");
    g_printf ("\n");
}

int main (int argc, char *argv[])
{
    if(argc<2) {
        show_help ();
        return 1;
    }

    de_name = g_strdup(argv[1]);

    xdg_autostart (de_name);

    g_free(de_name);
    return 0;
}
