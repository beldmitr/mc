/** \file setup.h
 *  \brief Header: setup loading/saving
 */

#pragma once

#include "lib/global.hpp"         /* GError */

#include "filemanager/layout.hpp" /* panel_view_mode_t */
#include "filemanager/panel.hpp"  /* WPanel */

class Setup
{
public:
    /* TAB length for editor and viewer */
    static const int DEFAULT_TAB_SPACING = 8;

    static const int MAX_MACRO_LENGTH = 1024;

public:
    enum qsearch_mode_t
    {
        QSEARCH_CASE_INSENSITIVE = 0,       /* quick search in case insensitive mode */
        QSEARCH_CASE_SENSITIVE = 1, /* quick search in case sensitive mode */
        QSEARCH_PANEL_CASE = 2,     /* quick search get value from panel case_sensitive */
        QSEARCH_NUM
    };

private:
    /* panels ini options; [Panels] section */
    struct panels_options_t
    {
        gboolean show_mini_info = TRUE;    /* If true, show the mini-info on the panel */
        gboolean kilobyte_si = FALSE;       /* If TRUE, SI units (1000 based) will be used for larger units
                                 * (kilobyte, megabyte, ...). If FALSE, binary units (1024 based) will be used */
        gboolean mix_all_files = FALSE;     /* If FALSE then directories are shown separately from files */
        gboolean show_backups = TRUE;      /* If TRUE, show files ending in ~ */
        gboolean show_dot_files = TRUE;    /* If TRUE, show files starting with a dot */
        gboolean fast_reload = FALSE;       /* If TRUE then use stat() on the cwd to determine directory changes */
        gboolean fast_reload_msg_shown = FALSE;     /* Have we shown the fast-reload warning in the past? */
        gboolean mark_moves_down = TRUE;   /* If TRUE, marking a files moves the cursor down */
        gboolean reverse_files_only = TRUE;        /* If TRUE, only selection of files is inverted */
        gboolean auto_save_setup = FALSE;
        gboolean navigate_with_arrows = FALSE;      /* If TRUE: l&r arrows are used to chdir if the input line is empty */
        gboolean scroll_pages = TRUE;      /* If TRUE, panel is scrolled by half the display when the cursor reaches
                                            the end or the beginning of the panel */
        gboolean scroll_center = FALSE;     /* If TRUE, scroll when the cursor hits the middle of the panel */
        gboolean mouse_move_pages = TRUE;  /* Scroll page/item using mouse wheel */
        gboolean filetype_mode = TRUE;     /* If TRUE then add per file type hilighting */
        gboolean permission_mode = FALSE;   /* If TRUE, we use permission hilighting */
        qsearch_mode_t qsearch_mode = QSEARCH_PANEL_CASE;        /* Quick search mode */
        gboolean torben_fj_mode = FALSE;    /* If TRUE, use some usability hacks by Torben */

        /* Select/unselect file flags */
        panel_select_flags_t select_flags = static_cast<panel_select_flags_t>(SELECT_MATCH_CASE | SELECT_SHELL_PATTERNS);
    };
public:
    struct macro_action_t
    {
        long action;
        int ch;
    };

    struct macros_t
    {
        int hotkey;
        GArray *macro;
    };
public:
    static inline char *global_profile_name = nullptr;      /* mc.lib */

    /* Asks for confirmation before deleting a file */
    static inline gboolean confirm_delete = TRUE;

    /* Asks for confirmation before deleting a hotlist entry */
    static inline gboolean confirm_directory_hotlist_delete = FALSE;

    /* Asks for confirmation before executing a program by pressing enter */
    static inline gboolean confirm_execute = FALSE;

    /* Asks for confirmation before leaving the program */
    static inline gboolean confirm_exit = FALSE;

    /* Asks for confirmation before overwriting a file */
    static inline gboolean confirm_overwrite = TRUE;

    /* Asks for confirmation when using F3 to view a directory and there are tagged files */
    static inline gboolean confirm_view_dir = FALSE;

    /* If on, default for "No" in delete operations */
    static inline gboolean safe_delete = FALSE;

    /* If on, default for "No" in overwrite files */
    static inline gboolean safe_overwrite = FALSE;

    /* Controls screen clearing before an exec */
    static inline gboolean clear_before_exec = TRUE;

    /* If true, at startup the user-menu is invoked */
    static inline gboolean auto_menu = FALSE;

    /* This flag indicates if the pull down menus by default drop down */
    static inline gboolean drop_menus = FALSE;

    static inline gboolean verbose = TRUE;

    static inline gboolean copymove_persistent_attr = TRUE;

    static inline gboolean classic_progressbar = TRUE;

    static inline gboolean easy_patterns = TRUE;

    /* Tab size */
    static inline int option_tab_spacing = DEFAULT_TAB_SPACING;

    /* It true saves the setup when quitting */
    static inline gboolean auto_save_setup = TRUE;

    /* If true, then the +, - and \ keys have their special meaning only if the
 * command line is empty, otherwise they behave like regular letters
 */
    static inline gboolean only_leading_plus_minus = TRUE;

    /* Automatically fills name with current selected item name on mkdir */
    static inline gboolean auto_fill_mkdir_name = TRUE;

    /* If set and you don't have subshell support, then C-o will give you a shell */
    static inline gboolean output_starts_shell = FALSE;

    /* If set, we execute the file command to check the file type */
    static inline gboolean use_file_to_check_type = TRUE;

    /*
     * Whether the Midnight Commander tries to provide more
     * information about copy/move sizes and bytes transferred
     * at the expense of some speed
     */
    static inline gboolean file_op_compute_totals = TRUE;

    /* Ask file name before start the editor */
    static inline gboolean editor_ask_filename_before_edit = FALSE;

    static panels_options_t panels_options;

    static inline panel_view_mode_t startup_left_mode;

    static inline panel_view_mode_t startup_right_mode;

    /* Only used at program boot */
    static inline gboolean boot_current_is_left = TRUE;

    /* If true use the internal viewer */
    static inline gboolean use_internal_view = TRUE;

    /* If set, use the builtin editor */
    static inline gboolean use_internal_edit = TRUE;

#ifdef HAVE_CHARSET
    /* Numbers of (file I/O) and (input/display) codepages. -1 if not selected */
    static inline int default_source_codepage = -1;

    static inline char* autodetect_codeset = nullptr;

    static inline gboolean is_autodetect_codeset_enabled = FALSE;
#endif /* !HAVE_CHARSET */

#ifdef HAVE_ASPELL
    static inline char* spell_language = nullptr;
#endif

    /* Value of "other_dir" key in ini file */
    static inline char* saved_other_dir = nullptr;

    /* If set, then print to the given file the last directory we were at */
    static inline char *last_wd_string = nullptr;

    /* Set when main loop should be terminated */
    static inline int quit = 0;

    /* Set to TRUE to suppress printing the last directory */
    static inline int print_last_revert = FALSE;

#ifdef USE_INTERNAL_EDIT
    /* index to record_macro_buf[], -1 if not recording a macro */
    static inline int macro_index = -1;

    /* macro stuff */
    static inline struct macro_action_t record_macro_buf[MAX_MACRO_LENGTH];

    static inline GArray *macros_list;
#endif /* USE_INTERNAL_EDIT */

    /* Ugly hack to allow panel_save_setup to work as a place holder for */
    /* default panel values */
    static inline int saving_setup;

public:
    static const char* setup_init();

    static void load_setup();

    static gboolean save_setup(gboolean save_options, gboolean save_panel_options);

    static void done_setup();

    static void setup_save_config_show_error(const char *filename, GError **mcerror);

    static void load_key_defs();

#ifdef ENABLE_VFS_FTP
    static char* load_anon_passwd();
#endif /* ENABLE_VFS_FTP */

    static void load_keymap_defs(gboolean load_from_file);

    static void free_keymap_defs();

    static void panel_load_setup(WPanel *panel, const char *section);

    static void panel_save_setup(WPanel *panel, const char *section);
private:
    struct list_formats_t
    {
        const char *key;
        int list_format;
    };

    struct bool_options_t
    {
        const char *opt_name;
        gboolean *opt_addr;
    };

    struct panel_types_t
    {
        const char *opt_name;
        panel_view_mode_t opt_type;
    };

    struct layout_int_options_t
    {
        const char *opt_name;
        int *opt_addr;
    };

    struct layout_bool_options_t
    {
        const char *opt_name;
        gboolean *opt_addr;
    };

    struct int_options_t
    {
        const char *opt_name;
        int *opt_addr;
    };

    struct str_options_t
    {
        const char *opt_name;
        char **opt_addr;
        const char *opt_defval;
    };

    struct panels_ini_options_t
    {
        const char *opt_name;
        gboolean *opt_addr;
    };
private:
    /* ${XDG_CONFIG_HOME}/mc/ini */
    static inline char* profile_name = nullptr;

    /* ${XDG_CACHE_HOME}/mc/panels.ini */
    static inline char* panels_profile_name = nullptr;

    static const list_formats_t list_formats[];

    static const panel_types_t panel_types[];

    static const layout_int_options_t layout_int_options[];

    static const layout_bool_options_t layout_bool_options[];

    static const bool_options_t bool_options[];

    static const int_options_t int_options[];

    static const str_options_t str_options[];

    static const panels_ini_options_t panels_ini_options[];

private:
    /**
     * Get name of config file.
     *
     * @param subdir If not NULL, config is also searched in specified subdir.
     * @param config_file_name If relative, file if searched in standard paths.
     *
     * @return newly allocated string with config name or NULL if file is not found.
     */
    static char* load_setup_get_full_config_name(const char *subdir, const char *config_file_name);

    static const char* setup__is_cfg_group_must_panel_config(const char *grp);

    static void setup__move_panels_config_into_separate_file(const char *profile);

    /**
      Create new mc_config object from specified ini-file or
      append data to existing mc_config object from ini-file
    */
    static void load_setup_init_config_from_file(mc_config_t **config, const char *fname, gboolean read_only);

    static void load_config();

    static panel_view_mode_t setup__load_panel_state(const char *section);

    static void load_layout();

    static void load_keys_from_section(const char *terminal, mc_config_t *cfg);

    static void load_keymap_from_section(const char *section_name, GArray *keymap, mc_config_t *cfg);

    static mc_config_t* load_setup_get_keymap_profile_config(gboolean load_from_file);

    static void panel_save_type(const char *section, panel_view_mode_t type);

    /**
     * Load panels options from [Panels] section.
     */
    static void panels_load_options();

    /**
     * Save panels options in [Panels] section.
     */
    static void panels_save_options();

    static void save_config();

    static void save_layout();

    /* save panels.ini */
    static void save_panel_types();
};

