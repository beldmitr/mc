#pragma once

#include "lib/global.hpp"         /* gboolean */
#include "lib/vfs/vfs.hpp"        /* vfs_path_t */

class Args
{
public:
    struct mcedit_arg_t
    {
        vfs_path_t *file_vpath;
        long line_number;
    };
public:
    /* If true, assume we are running on an xterm terminal */
    static gboolean mc_args__force_xterm;

    static gboolean mc_args__nomouse;

    /* Force colors, only used by Slang */
    static gboolean mc_args__force_colors;

    /* Don't load keymap from file and use default one */
    static gboolean mc_args__nokeymap;

    static char* mc_args__last_wd_file;

    /* when enabled NETCODE, use folowing file as logfile */
    static char* mc_args__netfs_logfile;

    /* keymap file */
    static char* mc_args__keymap_file;

    /* Debug level */
#ifdef ENABLE_VFS_SMB
    static int mc_args__debug_level;
#endif
    /*
     * MC_RUN_FULL: dir for left panel
     * MC_RUN_EDITOR: list of files to edit
     * MC_RUN_VIEWER: file to view
     * MC_RUN_DIFFVIEWER: first file to compare
     */
    static void* mc_run_param0;

    /*
     * MC_RUN_FULL: dir for right panel
     * MC_RUN_EDITOR: unused
     * MC_RUN_VIEWER: unused
     * MC_RUN_DIFFVIEWER: second file to compare
     */
    static char* mc_run_param1;

public:
    static void mc_setup_run_mode(char** argv);

    static gboolean mc_args_parse(int* argc, char*** argv, const char* translation_domain, GError** mcerror);

    static gboolean mc_args_show_info();

    static gboolean mc_setup_by_args(int argc, char** argv, GError** mcerror);

    /**
     * Free the mcedit_arg_t object.
     *
     * @param arg mcedit_arg_t object
     */
    static void mcedit_arg_free(mcedit_arg_t* arg);
    //------------------------------------------------------------------------
private:
    /**
     * Get list of filenames (and line numbers) from command line, when mc called as editor
     *
     * @param argc count of all arguments
     * @param argv array of strings, contains arguments
     * @return list of mcedit_arg_t objects
     */
    static GList* parse_mcedit_arguments(int argc, char** argv);

    /**
     * Create mcedit_arg_t object from file name and the line number.
     *
     * @param file_name   file name
     * @param line_number line number. If value is 0, try to restore saved position.
     * @return mcedit_arg_t object
     */
    static mcedit_arg_t* mcedit_arg_new (const char* file_name, long line_number);

    /**
     * Create mcedit_arg_t object from vfs_path_t object and the line number.
     *
     * @param file_vpath  file path object
     * @param line_number line number. If value is 0, try to restore saved position.
     * @return mcedit_arg_t object
     */
    static mcedit_arg_t* mcedit_arg_vpath_new(vfs_path_t* file_vpath, long line_number);

    static gboolean parse_mc_v_argument(const char* option_name, const char* value, void* data, GError** mcerror);

    static gboolean parse_mc_e_argument(const char* option_name, const char* value, void* data, GError ** mcerror);

    static char* mc_args__convert_help_to_syscharset (const char* charset, const char* error_message_str, const char* help_str);

    static void mc_args_add_extended_info_to_help();

    static char* mc_args_add_usage_info();

    static GOptionGroup* mc_args_new_color_group();

    static void mc_args_clean_temp_help_strings();

private:
    /* If true, show version info and exit */
    static gboolean mc_args__show_version;

    static GOptionContext *context;

#ifdef ENABLE_SUBSHELL
    static gboolean mc_args__nouse_subshell;
#endif /* ENABLE_SUBSHELL */
    static gboolean mc_args__show_datadirs;
    static gboolean mc_args__show_datadirs_extended;
#ifdef ENABLE_CONFIGURE_ARGS
    static gboolean mc_args__show_configure_opts;
#endif

    static GOptionGroup* main_group;

    static const GOptionEntry argument_main_table[];

    static GOptionGroup* terminal_group;

    static constexpr int ARGS_TERM_OPTIONS = 0;
    static const GOptionEntry argument_terminal_table[];

    static GOptionGroup* color_group;

    static constexpr int ARGS_COLOR_OPTIONS = 0;
    static const GOptionEntry argument_color_table[];

    static char* mc_args__loc__colors_string;
    static char* mc_args__loc__footer_string;
    static char* mc_args__loc__header_string;
    static char* mc_args__loc__usage_string;
};
