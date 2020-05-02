/*
   Print features specific for this build

   Copyright (C) 2000-2020
   Free Software Foundation, Inc.

   This file is part of the Midnight Commander.

   The Midnight Commander is free software: you can redistribute it
   and/or modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation, either version 3 of the License,
   or (at your option) any later version.

   The Midnight Commander is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/** \file textconf.c
 *  \brief Source: prints features specific for this build
 */

#include <limits.h>
#include <stdio.h>
#include <sys/types.h>

#include "lib/global.hpp"
#include "lib/fileloc.hpp"
#include "lib/mcconfig.hpp"
#include "lib/util.hpp"           /* mc_get_profile_root() */
#include "lib/tty/tty.hpp"        /* S-Lang or ncurses version */

#include "src/textconf.hpp"

void TextConf::show_version()
{
    size_t i;

    printf (_("GNU Midnight Commander %s\n"), VERSION);

    printf (_("Built with GLib %d.%d.%d\n"),
            GLIB_MAJOR_VERSION, GLIB_MINOR_VERSION, GLIB_MICRO_VERSION);

#ifdef HAVE_SLANG
    printf (_("Built with S-Lang %s with terminfo database\n"), SLANG_VERSION_STRING);
#elif defined(USE_NCURSES)
    #ifdef NCURSES_VERSION
    printf (_("Built with ncurses %s\n"), NCURSES_VERSION);
#else
    puts (_("Built with ncurses (unknown version)"));
#endif /* !NCURSES_VERSION */
#elif defined(USE_NCURSESW)
#ifdef NCURSES_VERSION
    printf (_("Built with ncursesw %s\n"), NCURSES_VERSION);
#else
    puts (_("Built with ncursesw (unknown version)"));
#endif /* !NCURSES_VERSION */
#else
#error "Cannot compile mc without S-Lang or ncurses"
#endif /* !HAVE_SLANG && !USE_NCURSES */

    for (i = 0; features[i] != NULL; i++)
        puts (_(features[i]));

#ifdef ENABLE_VFS
    puts (_("Virtual File Systems:"));
    for (i = 0; vfs_supported[i] != NULL; i++)
        printf ("%s %s", i == 0 ? "" : ",", _(vfs_supported[i]));
    (void) puts ("");
#endif /* ENABLE_VFS */

    (void) puts (_("Data types:"));
#define TYPE_INFO(T) \
    (void)printf(" %s: %d;", #T, (int) (CHAR_BIT * sizeof(T)))
    TYPE_INFO (char);
    TYPE_INFO (int);
    TYPE_INFO (long);
    TYPE_INFO (void *);
    TYPE_INFO (size_t);
    TYPE_INFO (off_t);
#undef TYPE_INFO
    (void) puts ("");
}


#define PRINTF_GROUP(a) \
    (void) printf ("[%s]\n", a)
#define PRINTF_SECTION(a,b) \
    (void) printf ("    %-17s %s\n", a, b)
#define PRINTF_SECTION2(a,b) \
    (void) printf ("    %-17s %s/\n", a, b)
#define PRINTF(a, b, c) \
    (void) printf ("\t%-15s %s/%s\n", a, b, c)
#define PRINTF2(a, b, c) \
    (void) printf ("\t%-15s %s%s\n", a, b, c)

void TextConf::show_datadirs_extended()
{
    (void) printf ("%s %s\n", _("Home directory:"), mc_config_get_home_dir ());
    (void) printf ("%s %s\n", _("Profile root directory:"), mc_get_profile_root ());
    (void) puts ("");

    PRINTF_GROUP (_("System data"));

    PRINTF_SECTION (_("Config directory:"), mc_global.sysconfig_dir.c_str());
    PRINTF_SECTION (_("Data directory:"), mc_global.share_data_dir.c_str());

    PRINTF_SECTION (_("File extension handlers:"), EXTHELPERSDIR);

#if defined ENABLE_VFS_EXTFS || defined ENABLE_VFS_FISH
    PRINTF_SECTION (_("VFS plugins and scripts:"), LIBEXECDIR);
#ifdef ENABLE_VFS_EXTFS
    PRINTF2 ("extfs.d:", LIBEXECDIR, MC_EXTFS_DIR PATH_SEP_STR);
#endif
#ifdef ENABLE_VFS_FISH
    PRINTF2 ("fish:", LIBEXECDIR, FISH_PREFIX PATH_SEP_STR);
#endif
#endif /* ENABLE_VFS_EXTFS || defiined ENABLE_VFS_FISH */
    (void) puts ("");

    PRINTF_GROUP (_("User data"));

    PRINTF_SECTION2 (_("Config directory:"), mc_config_get_path ());
    PRINTF_SECTION2 (_("Data directory:"), mc_config_get_data_path ());
    PRINTF ("skins:", mc_config_get_data_path (), MC_SKINS_SUBDIR PATH_SEP_STR);
#ifdef ENABLE_VFS_EXTFS
    PRINTF ("extfs.d:", mc_config_get_data_path (), MC_EXTFS_DIR PATH_SEP_STR);
#endif
#ifdef ENABLE_VFS_FISH
    PRINTF ("fish:", mc_config_get_data_path (), FISH_PREFIX PATH_SEP_STR);
#endif
#ifdef USE_INTERNAL_EDIT
    PRINTF ("mcedit macros:", mc_config_get_data_path (), MC_MACRO_FILE);
    PRINTF ("mcedit external macros:", mc_config_get_data_path (), MC_EXTMACRO_FILE ".*");
#endif
    PRINTF_SECTION2 (_("Cache directory:"), mc_config_get_cache_path ());
}

#undef PRINTF
#undef PRINTF_SECTION
#undef PRINTF_GROUP

#ifdef ENABLE_CONFIGURE_ARGS
void TextConf::show_configure_options (void)
{
    (void) puts (MC_CONFIGURE_ARGS);
}
#endif