/** \file  execute.h
 *  \brief Header: execution routines
 */

#pragma once

#include "lib/utilunix.hpp"
#include "lib/vfs/vfs.hpp"

class Execute
{
private:
    /* If true, after executing a command, wait for a keystroke */
    enum
    {
        pause_never,
        pause_on_dumb_terminals,
        pause_always
    };

public:
    static inline int pause_after_run = pause_on_dumb_terminals;

public:
    /* Execute functions that use the shell to execute */
    static void shell_execute(const char* command, int flags);

    /* Handle toggling panels by Ctrl-O */
    static void toggle_subshell();

    /* Handle toggling panels by Ctrl-Z */
    static gboolean execute_suspend(const gchar* event_group_name, const gchar* event_name, gpointer init_data, gpointer data);

    /* Execute command on a filename that can be on VFS */
    static void execute_with_vfs_arg(const char* command, const vfs_path_t* filename_vpath);

    /**
     * Execute external editor or viewer.
     *
     * @param command editor/viewer to run
     * @param filename_vpath path for edit/view
     * @param start_line cursor will be placed at the 'start_line' position after opening file
     *        if start_line is 0 or negative, no start line will be passed to editor/viewer
     */
    static void execute_external_editor_or_viewer(const char* command, const vfs_path_t* filename_vpath, long start_line);

    /** Hide the terminal after executing a program */
    static void post_exec();

    /** Set up the terminal before executing a program */
    static void pre_exec();

private:
    static char* execute_get_external_cmd_opts_from_config(const char* command, const vfs_path_t* filename_vpath, long start_line);

    static void do_executev(const char* shell, int flags, char* const argv[]);

    static void do_execute(const char* shell, const char* command, int flags);

private:
    static char* execute_get_opts_from_cfg (const char* command, const char* default_str);

    static void execute_cleanup_with_vfs_arg(const vfs_path_t* filename_vpath, vfs_path_t** localcopy_vpath, time_t* mtime);

    static gboolean execute_prepare_with_vfs_arg (const vfs_path_t* filename_vpath, vfs_path_t** localcopy_vpath, time_t* mtime);

    static void do_suspend_cmd();

#ifdef ENABLE_SUBSHELL
    static void do_possible_cd(const vfs_path_t* new_dir_vpath);
#endif /* ENABLE_SUBSHELL */

    static void edition_pre_exec();

    static void edition_post_exec();
};
