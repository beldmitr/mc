/*
   Main program for the Midnight Commander

   Copyright (C) 1994-2020
   Free Software Foundation, Inc.

   Written by:
   Miguel de Icaza, 1994, 1995, 1996, 1997
   Janne Kukonlehto, 1994, 1995
   Norbert Warmuth, 1997

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

/** \file main.c
 *  \brief Source: this is a main module
 */
// TODO DB Remove unused headers

//#include <cctype>
//#include <cerrno>
//#include <clocale>
#include <pwd.h>                /* for username in xterm title */
#include <cstdio>
#include <cstdlib>
#include <cstring>
//#include <sys/types.h>
#include <csignal>
#include <unistd.h>             /* getsid() */
#include <filesystem>

#include "lib/global.hpp"

#include "lib/event.hpp"
#include "lib/tty/tty.hpp"
#include "lib/tty/key.hpp"        /* For init_key() */
#include "lib/tty/mouse.hpp"      /* init_mouse() */
#include "lib/timer.hpp"
#include "lib/skin.hpp"
//#include "lib/filehighlight.hpp"
//#include "lib/fileloc.hpp"
#include "lib/strutil.hpp"
#include "lib/util.hpp"
#include "lib/vfs/vfs.hpp"        /* vfs_init(), vfs_shut() */

#include "filemanager/midnight.hpp"       /* current_panel */
#include "filemanager/treestore.hpp"      /* tree_store_save */
//#include "filemanager/layout.hpp"
#include "filemanager/ext.hpp"    /* flush_extension_file() */
#include "filemanager/command.hpp"        /* cmdline */
//#include "filemanager/panel.hpp"  /* panalized_panel */

#include "vfs/plugins_init.hpp"

#include "events_init.hpp"
#include "args.hpp"
#ifdef ENABLE_SUBSHELL
#include "subshell/subshell.hpp"
#endif
#include "setup.hpp"              /* load_setup() */

//#ifdef HAVE_CHARSET
//#include "lib/charsets.hpp"
//#include "selcodepage.hpp"
//#endif /* HAVE_CHARSET */

#include "consaver/cons.saver.hpp"        /* cons_saver_pid */
#include "util.hpp"


int main (int argc, char *argv[])
{

    int exit_code = EXIT_FAILURE;

    mc_global.run_from_parent_mc = !Util::CheckSid();

    mc_global.timer = std::make_shared<Timer>();

    /* We had LC_CTYPE before, LC_ALL includs LC_TYPE as well */
#ifdef HAVE_SETLOCALE
    (void) setlocale (LC_ALL, "");
#endif
    (void) bindtextdomain (PACKAGE, LOCALEDIR);
    (void) textdomain (PACKAGE);

    /* do this before args parsing */
    str_init_strings (nullptr);

    // set up mc_global
    mc_global.SetRunMode(Args::SetupRunMode(argv[0]));   /* are we mc? editor? viewer? etc... */

    GError *mcerror = nullptr;
    if (!Args::Parse (&argc, &argv, "mc", &mcerror))
    {
      startup_exit_falure:
        fprintf (stderr, _("Failed to run:\n%s\n"), mcerror->message);
        g_error_free (mcerror);
      startup_exit_ok:
        str_uninit_strings ();
        return exit_code;
    }

    /* do this before mc_args_show_info () to view paths in the --datadir-info output */
    Util::OS_Setup();

    std::filesystem::path path(mc_config_get_home_dir());
    if (path.is_relative())
    {
        mc_propagate_error(&mcerror, 0, "%s: %s", _("Home directory path is not absolute"), mc_config_get_home_dir());
        mc_event_deinit(nullptr);
        goto startup_exit_falure;
    }

    if (!Args::ShowInfo())
    {
        exit_code = EXIT_SUCCESS;
        goto startup_exit_ok;
    }

    if (!events_init (&mcerror))
        goto startup_exit_falure;

    mc_config_init_config_paths(&mcerror);
    char *config_migrate_msg = nullptr;
    bool config_migrated = mc_config_migrate_from_old_place(&mcerror, &config_migrate_msg);
    if (mcerror)
    {
        mc_event_deinit (nullptr);
        goto startup_exit_falure;
    }

    vfs_init ();
    Plugins::VFSPluginsInit ();

    load_setup ();

    /* Must be done after load_setup because depends on mc_global.vfs.cd_symlinks */
    vfs_setup_work_dir ();

    /* Set up temporary directory after VFS initialization */
    mc_tmpdir ();

    /* do this after vfs initialization and vfs working directory setup
       due to mc_setctl() and mcedit_arg_vpath_new() calls in mc_setup_by_args() */
    if (!Args::MCSetupByArgs(argc, argv, &mcerror))
    {
        vfs_shut ();
        done_setup ();
        g_free (saved_other_dir);
        mc_event_deinit (nullptr);
        goto startup_exit_falure;
    }

    /* Resolve the other_dir panel option.
     * 1. Must be done after vfs_setup_work_dir().
     * 2. Must be done after mc_setup_by_args() because of mc_run_mode.
     */
    if (mc_global.GetRunMode() == Global::RunMode::MC_RUN_FULL)
    {
        char *buffer = mc_config_get_string (mc_global.panels_config, "Dirs", "other_dir", ".");
        vfs_path_t *vpath = vfs_path_from_str (buffer);
        if (vfs_file_is_local (vpath))
            saved_other_dir = buffer;
        else
            g_free (buffer);
        vfs_path_free (vpath);
    }

    /* check terminal type
     * $TERM must be set and not empty
     * mc_global.tty.xterm_flag is used in init_key() and tty_init()
     * Do this after mc_args_handle() where mc_args__force_xterm is set up.
     */
    mc_global.tty.xterm_flag = tty_check_term (Args::bForceXterm);

    /* NOTE: This has to be called before tty_init or whatever routine
       calls any define_sequence */
    init_key ();

    /* Must be done before installing the SIGCHLD handler [[FIXME]] */
    handle_console (CONSOLE_INIT);

#ifdef ENABLE_SUBSHELL
    /* Disallow subshell when invoked as standalone viewer or editor from running mc */
    if (mc_global.GetRunMode() != Global::RunMode::MC_RUN_FULL && mc_global.run_from_parent_mc)
        mc_global.tty.use_subshell = FALSE;

    if (mc_global.tty.use_subshell)
        subshell_get_console_attributes ();
#endif /* ENABLE_SUBSHELL */

    /* Install the SIGCHLD handler; must be done before init_subshell() */
    Util::InitSigchld ();

    /* We need this, since ncurses endwin () doesn't restore the signals */
    save_stop_handler ();

    /* Must be done before init_subshell, to set up the terminal size: */
    /* FIXME: Should be removed and LINES and COLS computed on subshell */
    tty_init (!Args::bNoMouse, mc_global.tty.xterm_flag);

    /* start check mc_global.display_codepage and mc_global.source_codepage */
    Util::check_codeset ();

    /* Removing this from the X code let's us type C-c */
    load_key_defs ();

    load_keymap_defs (!Args::bNoKeymap);

#ifdef USE_INTERNAL_EDIT
    macros_list = g_array_new (TRUE, FALSE, sizeof (macros_t));
#endif /* USE_INTERNAL_EDIT */

    tty_init_colors (mc_global.tty.disable_colors, Args::bForceColors);

    mc_skin_init (nullptr, &mcerror);
    dlg_set_default_colors ();
    input_set_default_colors ();
    if (mc_global.GetRunMode() == Global::RunMode::MC_RUN_FULL)
        command_set_default_colors ();

    mc_error_message (&mcerror, nullptr);

#ifdef ENABLE_SUBSHELL
    /* Done here to ensure that the subshell doesn't  */
    /* inherit the file descriptors opened below, etc */
    if (mc_global.tty.use_subshell && mc_global.run_from_parent_mc)
    {
        int r = query_dialog (_("Warning"),
                          _("GNU Midnight Commander\nis already running on this terminal.\n"
                            "Subshell support will be disabled."),
                          D_ERROR, 2, _("&OK"), _("&Quit"));
        if (r == 0)
        {
            /* parent mc was found and the user wants to continue */
        }
        else
        {
            /* parent mc was found and the user wants to quit mc */
            mc_global.midnight_shutdown = TRUE;
        }

        mc_global.tty.use_subshell = FALSE;
    }

    if (mc_global.tty.use_subshell)
        init_subshell ();
#endif /* ENABLE_SUBSHELL */

    if (!mc_global.midnight_shutdown)
    {
        /* Also done after init_subshell, to save any shell init file messages */
        if (mc_global.tty.console_flag != '\0')
            handle_console (CONSOLE_SAVE);

        if (mc_global.tty.alternate_plus_minus)
            application_keypad_mode ();

        /* Done after subshell initialization to allow select and paste text by mouse
           w/o Shift button in subshell in the native console */
        init_mouse ();

        /* Done after tty_enter_ca_mode (tty_init) because in VTE bracketed mode is
           separate for the normal and alternate screens */
        enable_bracketed_paste ();

        /* subshell_prompt is NULL here */
        mc_prompt = (geteuid () == 0) ? "# " : "$ ";

        if (config_migrated)
        {
            message (D_ERROR, _("Warning"), "%s", config_migrate_msg);
            g_free (config_migrate_msg);
        }
    }

    /* Program main loop */
    if (mc_global.midnight_shutdown)
        exit_code = EXIT_SUCCESS;
    else
        exit_code = do_nc ()? EXIT_SUCCESS : EXIT_FAILURE;

    disable_bracketed_paste ();

    disable_mouse ();

    /* Save the tree store */
    tree_store_save ();

    free_keymap_defs ();

    /* Virtual File System shutdown */
    vfs_shut ();

    flush_extension_file ();    /* does only free memory */

    mc_skin_deinit ();
    tty_colors_done ();

    tty_shutdown ();

    done_setup ();

    if (mc_global.tty.console_flag != '\0' && (quit & SUBSHELL_EXIT) == 0)
        handle_console (CONSOLE_RESTORE);
    if (mc_global.tty.alternate_plus_minus)
        numeric_keypad_mode ();

    (void) signal (SIGCHLD, SIG_DFL);   /* Disable the SIGCHLD handler */

    if (mc_global.tty.console_flag != '\0')
        handle_console (CONSOLE_DONE);

    if (mc_global.GetRunMode() == Global::RunMode::MC_RUN_FULL && Args::last_wd_file
        && last_wd_string && !print_last_revert)
    {
        int last_wd_fd = open (Args::last_wd_file, O_WRONLY | O_CREAT | O_TRUNC | O_EXCL, S_IRUSR | S_IWUSR);
        if (last_wd_fd != -1)
        {
            /* ssize_t ret1 = */ write(last_wd_fd, last_wd_string, strlen (last_wd_string));
            /* int ret2 = */ close(last_wd_fd);
        }
    }
    g_free (last_wd_string);

    done_key ();

#ifdef USE_INTERNAL_EDIT
    if (macros_list)
    {
        for (size_t i = 0; i < macros_list->len; i++)
        {
            macros_t *macros = &g_array_index (macros_list, struct macros_t, i);
            if (macros && macros->macro)
                (void) g_array_free (macros->macro, TRUE);
        }
        (void) g_array_free (macros_list, TRUE);
    }
#endif /* USE_INTERNAL_EDIT */

    str_uninit_strings ();

    if (mc_global.GetRunMode() != Global::RunMode::MC_RUN_EDITOR)
        g_free (Args::mc_run_param0);
    else
        g_list_free_full ((GList *) Args::mc_run_param0, (GDestroyNotify) Args::Free);

    g_free (Args::mc_run_param1);
    g_free (saved_other_dir);

    mc_config_deinit_config_paths ();

    (void) mc_event_deinit (&mcerror);
    if (mcerror)
    {
        fprintf (stderr, _("\nFailed while close:\n%s\n"), mcerror->message);
        g_error_free (mcerror);
        exit_code = EXIT_FAILURE;
    }

    // FIXME DB: no changes when I remove this
    (void) putchar ('\n');      /* Hack to make shell's prompt start at left of screen */

    return exit_code;
}

/* --------------------------------------------------------------------------------------------- */
