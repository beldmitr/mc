/*
   Execution routines for GNU Midnight Commander

   Copyright (C) 2003-2020
   Free Software Foundation, Inc.

   Written by:
   Slava Zanko <slavazanko@gmail.com>, 2013

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

/** \file  execute.c
 *  \brief Source: execution routines
 */

#include <signal.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>

#include "lib/global.hpp"

#include "lib/tty/tty.hpp"
#include "lib/tty/key.hpp"
#include "lib/tty/win.hpp"
#include "lib/vfs/vfs.hpp"
#include "lib/mcconfig.hpp"
#include "lib/util.hpp"
#include "lib/strutil.hpp"        /* str_replace_all_substrings() */
#include "lib/widget.hpp"

#include "filemanager/midnight.hpp"
#include "filemanager/layout.hpp" /* use_dash() */
#include "consaver/cons.saver.hpp"
#ifdef ENABLE_SUBSHELL
#include "subshell/subshell.hpp"
#endif
#include "setup.hpp"              /* clear_before_exec */

#include "execute.hpp"


/* Executes a command */
void Execute::shell_execute(const char* command, int flags)
{
    char *cmd = nullptr;

    if (flags & EXECUTE_HIDE)
    {
        cmd = g_strconcat (" ", command, (char *) NULL);
        flags ^= EXECUTE_HIDE;
    }

#ifdef ENABLE_SUBSHELL
    if (mc_global.tty.use_subshell)
    {
        if (subshell_state == INACTIVE)
            do_execute (mc_global.shell->path, cmd ? cmd : command, flags | EXECUTE_AS_SHELL);
        else
            message (D_ERROR, MSG_ERROR, "%s", _("The shell is already running a command"));
    }
    else
#endif /* ENABLE_SUBSHELL */
        do_execute (mc_global.shell->path, cmd ? cmd : command, flags | EXECUTE_AS_SHELL);

    g_free (cmd);
}

void Execute::toggle_subshell()
{
    static gboolean message_flag = TRUE;

#ifdef ENABLE_SUBSHELL
    vfs_path_t *new_dir_vpath = nullptr;   // TODO DB investigate me. This has to be always NULL
#endif /* ENABLE_SUBSHELL */

    SIG_ATOMIC_VOLATILE_T was_sigwinch = 0;

    if (!(mc_global.tty.xterm_flag || mc_global.tty.console_flag != '\0'
        || mc_global.tty.use_subshell || Setup::output_starts_shell))
    {
        if (message_flag)
            message (D_ERROR, MSG_ERROR,
                     _("Not an xterm or Linux console;\nthe panels cannot be toggled."));
        message_flag = FALSE;
        return;
    }

    channels_down();
    disable_mouse();
    disable_bracketed_paste();
    if (Setup::clear_before_exec)
        clr_scr();
    if (mc_global.tty.alternate_plus_minus)
        numeric_keypad_mode();
#ifndef HAVE_SLANG
    /* With slang we don't want any of this, since there
     * is no raw_mode supported
     */
    tty_reset_shell_mode ();
#endif /* !HAVE_SLANG */
    tty_noecho();
    tty_keypad(FALSE);
    tty_reset_screen();
    tty_exit_ca_mode();
    tty_raw_mode();
    if (mc_global.tty.console_flag != '\0')
        handle_console(CONSOLE_RESTORE);

#ifdef ENABLE_SUBSHELL
    if (mc_global.tty.use_subshell)
    {
        vfs_path_t **new_dir_p = vfs_current_is_local()? &new_dir_vpath : nullptr;
        invoke_subshell(NULL, VISIBLY, new_dir_p);
    }
    else
#endif /* ENABLE_SUBSHELL */
    {
        if (Setup::output_starts_shell)
        {
            fputs(_("Type 'exit' to return to the Midnight Commander"), stderr);
            fputs("\n\r\n\r", stderr);

            my_system(EXECUTE_INTERNAL, mc_global.shell->path, NULL);
        }
        else
            get_key_code(0);
    }

    if (mc_global.tty.console_flag != '\0')
        handle_console(CONSOLE_SAVE);

    tty_enter_ca_mode();

    tty_reset_prog_mode();
    tty_keypad(TRUE);

    /* Prevent screen flash when user did 'exit' or 'logout' within
       subshell */
    if ((Setup::quit & SUBSHELL_EXIT) != 0)
    {
        /* User did 'exit' or 'logout': quit MC */
        if (quiet_quit_cmd())
            return;

        Setup::quit = 0;
#ifdef ENABLE_SUBSHELL
        /* restart subshell */
        if (mc_global.tty.use_subshell)
            init_subshell();
#endif /* ENABLE_SUBSHELL */
    }

    enable_mouse();
    enable_bracketed_paste();
    channels_up();
    if (mc_global.tty.alternate_plus_minus)
        application_keypad_mode();

    /* HACK:
     * Save sigwinch flag that will be reset in mc_refresh() called via update_panels().
     * There is some problem with screen redraw in ncurses-based mc in this situation.
     */
    was_sigwinch = tty_got_winch();
    tty_flush_winch();

#ifdef ENABLE_SUBSHELL
    if (mc_global.tty.use_subshell)
    {
        if (mc_global.mc_run_mode == MC_RUN_FULL)
        {
            do_load_prompt();
            if (new_dir_vpath != NULL)
                do_possible_cd(new_dir_vpath);
        }
        else if (new_dir_vpath != NULL && mc_chdir (new_dir_vpath) != -1)
            vfs_setup_cwd();
    }

    vfs_path_free(new_dir_vpath);
#endif /* ENABLE_SUBSHELL */

    if (mc_global.mc_run_mode == MC_RUN_FULL)
    {
        update_panels(UP_OPTIMIZE, UP_KEEPSEL);
        update_xterm_title_path();
    }

    if (was_sigwinch != 0 || tty_got_winch())
        dialog_change_screen_size();
    else
        repaint_screen();
}

/* event callback */
gboolean Execute::execute_suspend(const char*, const char*, void*, void*)
{
    if (mc_global.mc_run_mode == MC_RUN_FULL)
        save_cwds_stat();

    do_suspend_cmd();
    if (mc_global.mc_run_mode == MC_RUN_FULL)
        update_panels(UP_OPTIMIZE, UP_KEEPSEL);
    do_refresh();

    return TRUE;
}

/**
 * Execute command on a filename that can be on VFS.
 * Errors are reported to the user.
 */
void Execute::execute_with_vfs_arg(const char* command, const vfs_path_t* filename_vpath)
{
    vfs_path_t *localcopy_vpath = NULL;
    const vfs_path_t *do_execute_vpath;
    time_t mtime;

    if (!execute_prepare_with_vfs_arg(filename_vpath, &localcopy_vpath, &mtime))
        return;

    do_execute_vpath = (localcopy_vpath == nullptr) ? filename_vpath : localcopy_vpath;

    do_execute(command, vfs_path_get_last_path_str (do_execute_vpath), EXECUTE_INTERNAL);

    execute_cleanup_with_vfs_arg(filename_vpath, &localcopy_vpath, &mtime);
}

void Execute::execute_external_editor_or_viewer(const char* command, const vfs_path_t* filename_vpath, long start_line)
{
    time_t mtime = 0;

    vfs_path_t *localcopy_vpath = NULL;
    if (!execute_prepare_with_vfs_arg(filename_vpath, &localcopy_vpath, &mtime))
        return;

    const vfs_path_t *do_execute_vpath = (localcopy_vpath == nullptr) ? filename_vpath : localcopy_vpath;

    char *extern_cmd_options = execute_get_external_cmd_opts_from_config(command, do_execute_vpath, start_line);

    if (extern_cmd_options != nullptr)
    {
        char **argv_cmd_options;
        int argv_count;

        if (g_shell_parse_argv (extern_cmd_options, &argv_count, &argv_cmd_options, NULL))
        {
            do_executev(command, EXECUTE_INTERNAL, argv_cmd_options);
            g_strfreev (argv_cmd_options);
        }
        else
            do_executev(command, EXECUTE_INTERNAL, NULL);

        g_free(extern_cmd_options);
    }

    execute_cleanup_with_vfs_arg(filename_vpath, &localcopy_vpath, &mtime);
}

void Execute::post_exec()
{
    edition_post_exec();
    use_dash(TRUE);
    repaint_screen();
}

void Execute::pre_exec()
{
    use_dash(FALSE);
    edition_pre_exec();
}

char* Execute::execute_get_external_cmd_opts_from_config(const char *command, const vfs_path_t * filename_vpath, long start_line)
{
    if (filename_vpath == nullptr)
        return g_strdup("");

    char* parameter = g_shell_quote(vfs_path_get_last_path_str (filename_vpath));

    if (start_line <= 0)
        return parameter;

    char* str_from_config = execute_get_opts_from_cfg(command, "%filename");

    char* return_str = str_replace_all(str_from_config, "%filename", parameter);
    g_free(parameter);
    g_free(str_from_config);
    str_from_config = return_str;

    parameter = g_strdup_printf("%ld", start_line);
    return_str = str_replace_all(str_from_config, "%lineno", parameter);
    g_free(parameter);
    g_free(str_from_config);

    return return_str;
}

void Execute::do_executev(const char *shell, int flags, char *const argv[])
{
#ifdef ENABLE_SUBSHELL
    vfs_path_t* new_dir_vpath = NULL;
#endif /* ENABLE_SUBSHELL */

    vfs_path_t* old_vfs_dir_vpath = NULL;

    if (!vfs_current_is_local())
        old_vfs_dir_vpath = vfs_path_clone(vfs_get_raw_current_dir());

    if (mc_global.mc_run_mode == MC_RUN_FULL)
        save_cwds_stat();
    pre_exec();
    if (mc_global.tty.console_flag != '\0')
        handle_console(CONSOLE_RESTORE);

    if (!mc_global.tty.use_subshell && *argv != nullptr && (flags & EXECUTE_INTERNAL) == 0)
    {
        printf("%s%s\n", mc_prompt, *argv);
        fflush(stdout);
    }
#ifdef ENABLE_SUBSHELL
    if (mc_global.tty.use_subshell && (flags & EXECUTE_INTERNAL) == 0)
    {
        do_update_prompt();

        /* We don't care if it died, higher level takes care of this */
        invoke_subshell(*argv, VISIBLY, old_vfs_dir_vpath != nullptr ? NULL : &new_dir_vpath);
    }
    else
#endif /* ENABLE_SUBSHELL */
        my_systemv_flags(flags, shell, argv);

    if ((flags & EXECUTE_INTERNAL) == 0)
    {
        if ((pause_after_run == pause_always
            || (pause_after_run == pause_on_dumb_terminals && !mc_global.tty.xterm_flag
                && mc_global.tty.console_flag == '\0')) && Setup::quit == 0
#ifdef ENABLE_SUBSHELL
            && subshell_state != RUNNING_COMMAND
#endif /* ENABLE_SUBSHELL */
            )
        {
            printf("%s", _("Press any key to continue..."));
            fflush(stdout);
            tty_raw_mode();
            get_key_code(0);
            printf("\r\n");
            fflush(stdout);
        }
        if (mc_global.tty.console_flag != '\0' && output_lines != 0 && mc_global.keybar_visible)
        {
            putchar('\n');
            fflush(stdout);
        }
    }

    if (mc_global.tty.console_flag != '\0')
        handle_console(CONSOLE_SAVE);
    edition_post_exec();

#ifdef ENABLE_SUBSHELL
    if (new_dir_vpath != nullptr)
    {
        do_possible_cd(new_dir_vpath);
        vfs_path_free(new_dir_vpath);
    }

#endif /* ENABLE_SUBSHELL */

    if (old_vfs_dir_vpath != nullptr)
    {
        mc_chdir(old_vfs_dir_vpath);
        vfs_path_free(old_vfs_dir_vpath);
    }

    if (mc_global.mc_run_mode == MC_RUN_FULL)
    {
        update_panels(UP_OPTIMIZE, UP_KEEPSEL);
        update_xterm_title_path();
    }

    do_refresh();
    use_dash(TRUE);
}

void Execute::do_execute(const char* shell, const char* command, int flags)
{
    GPtrArray* args_array = g_ptr_array_new();
    g_ptr_array_add(args_array, (char*)command);
    g_ptr_array_add(args_array, NULL);

    do_executev(shell, flags, (char *const *) args_array->pdata);

    g_ptr_array_free(args_array, TRUE);
}

char* Execute::execute_get_opts_from_cfg(const char* command, const char* default_str)
{
    char* str_from_config = mc_config_get_string_raw(mc_global.main_config, CONFIG_EXT_EDITOR_VIEWER_SECTION, command, NULL);
    if (str_from_config == nullptr)
    {
        mc_config_t *cfg = mc_config_init(Setup::global_profile_name, TRUE);
        if (cfg == nullptr)
            return g_strdup (default_str);

        str_from_config = mc_config_get_string_raw(cfg, CONFIG_EXT_EDITOR_VIEWER_SECTION, command, default_str);

        mc_config_deinit(cfg);
    }

    return str_from_config;
}

void Execute::execute_cleanup_with_vfs_arg(const vfs_path_t* filename_vpath, vfs_path_t** localcopy_vpath, time_t* mtime)
{
    if (*localcopy_vpath != nullptr)
    {
        struct stat st;

        /*
         * filename can be an entry on panel, it can be changed by executing
         * the command, so make a copy.  Smarter VFS code would make the code
         * below unnecessary.
         */
        mc_stat(*localcopy_vpath, &st);
        mc_ungetlocalcopy(filename_vpath, *localcopy_vpath, *mtime != st.st_mtime);
        vfs_path_free(*localcopy_vpath);
        *localcopy_vpath = NULL;
    }
}

gboolean Execute::execute_prepare_with_vfs_arg(const vfs_path_t* filename_vpath, vfs_path_t** localcopy_vpath, time_t* mtime)
{
    struct stat st;

    /* Simplest case, this file is local */
    if ((filename_vpath == nullptr && vfs_file_is_local(vfs_get_raw_current_dir ()))
        || vfs_file_is_local(filename_vpath))
        return TRUE;

    /* FIXME: Creation of new files on VFS is not supported */
    if (filename_vpath == nullptr)
        return FALSE;

    *localcopy_vpath = mc_getlocalcopy(filename_vpath);
    if (*localcopy_vpath == nullptr)
    {
        message(D_ERROR, MSG_ERROR, _("Cannot fetch a local copy of %s"),
                 vfs_path_as_str(filename_vpath));
        return FALSE;
    }

    mc_stat(*localcopy_vpath, &st);
    *mtime = st.st_mtime;
    return TRUE;
}

void Execute::do_suspend_cmd()
{
    pre_exec();

    if (mc_global.tty.console_flag != '\0' && !mc_global.tty.use_subshell)
        handle_console(CONSOLE_RESTORE);

#ifdef SIGTSTP
    {
        struct sigaction sigtstp_action;

        memset(&sigtstp_action, 0, sizeof(sigtstp_action));
        /* Make sure that the SIGTSTP below will suspend us directly,
           without calling ncurses' SIGTSTP handler; we *don't* want
           ncurses to redraw the screen immediately after the SIGCONT */
        sigaction(SIGTSTP, &startup_handler, &sigtstp_action);

        kill(getpid(), SIGTSTP);

        /* Restore previous SIGTSTP action */
        sigaction(SIGTSTP, &sigtstp_action, NULL);
    }
#endif /* SIGTSTP */

    if (mc_global.tty.console_flag != '\0' && !mc_global.tty.use_subshell)
        handle_console(CONSOLE_SAVE);

    edition_post_exec();
}

#ifdef ENABLE_SUBSHELL
void Execute::do_possible_cd(const vfs_path_t* new_dir_vpath)
{
    if (!do_cd(new_dir_vpath, cd_exact))
        message(D_ERROR, _("Warning"), "%s",
                 _("The Commander can't change to the directory that\n"
                   "the subshell claims you are in. Perhaps you have\n"
                   "deleted your working directory, or given yourself\n"
                   "extra access permissions with the \"su\" command?"));
}
#endif /* ENABLE_SUBSHELL */

void Execute::edition_pre_exec()
{
    if (Setup::clear_before_exec)
        clr_scr();
    else
    {
        if (!(mc_global.tty.console_flag != '\0' || mc_global.tty.xterm_flag))
            printf("\n\n");
    }

    channels_down();
    disable_mouse();
    disable_bracketed_paste();

    tty_reset_shell_mode();
    tty_keypad(FALSE);
    tty_reset_screen();

    numeric_keypad_mode();

    /* on xterms: maybe endwin did not leave the terminal on the shell
     * screen page: do it now.
     *
     * Do not move this before endwin: in some systems rmcup includes
     * a call to clear screen, so it will end up clearing the shell screen.
     */
    tty_exit_ca_mode();
}

void Execute::edition_post_exec()
{
    tty_enter_ca_mode();

    /* FIXME: Missing on slang endwin? */
    tty_reset_prog_mode();
    tty_flush_input();

    tty_keypad(TRUE);
    tty_raw_mode();
    channels_up();
    enable_mouse();
    enable_bracketed_paste();
    if (mc_global.tty.alternate_plus_minus)
        application_keypad_mode();
}
