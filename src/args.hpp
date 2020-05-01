#pragma once

#include <iostream>

#include "lib/global.hpp"         /* gboolean */
#include "lib/vfs/vfs.hpp"        /* vfs_path_t */


#include "src/textconf.hpp" // show_version();
#ifdef ENABLE_VFS_SMB
#include "src/vfs/smbfs/smbfs.hpp"        /* smbfs_set_debugf()  */
#endif
#include "lib/util.hpp"           /* x_basename() */
#include "lib/strutil.hpp"  // str_detect_termencoding(), str_isutf8(...), str_convert(...)

class Args
{
public:
    Args(vfs_path_t *file_vpath, long line_number)
        :   file_vpath(file_vpath),
            line_number(line_number)
    {}

    ~Args()
    {
        vfs_path_free (this->file_vpath);
    }

    void SetFileVPath(vfs_path_t* file_vpath)
    {
        this->file_vpath = file_vpath;
    }
    void SetLineNumber(long line_number)
    {
        this->line_number = line_number;
    }

    const vfs_path_t* GetFileVPath() const
    {
        return file_vpath;
    }

    long GetLineNumber() const
    {
        return line_number;
    }
public:
    // FIXME WB: get rid of this function, use an deconstructor instead
    static void Free (Args * arg);

    static Global::RunMode SetupRunMode (const char* base);

private:
    vfs_path_t* file_vpath;
    long line_number;
public:
    /* If true, assume we are running on an xterm terminal */
    static inline bool bForceXterm = false;

    static inline bool bNoMouse = false;

    /* Force colors, only used by Slang */
    static inline bool bForceColors = false;

    /* Don't load keymap from file and use default one */
    static inline bool bNoKeymap = false;

    static inline char* last_wd_file = nullptr;

    /* when enabled NETCODE, use folowing file as logfile */
    static inline char *netfs_logfile = nullptr;

    /* keymap file */
    static inline char *keymap_file = nullptr;

    /* Debug level */
#ifdef ENABLE_VFS_SMB
    static inline int debug_level = 0;
#endif

    /*
     * MC_RUN_FULL: dir for left panel
     * MC_RUN_EDITOR: list of files to edit
     * MC_RUN_VIEWER: file to view
     * MC_RUN_DIFFVIEWER: first file to compare
     */
    static inline void* mc_run_param0 = nullptr;

    /*
     * MC_RUN_FULL: dir for right panel
     * MC_RUN_EDITOR: unused
     * MC_RUN_VIEWER: unused
     * MC_RUN_DIFFVIEWER: second file to compare
     */
    static inline char* mc_run_param1 = nullptr;
public:
    static bool Parse (int *argc, char ***argv, const char *translation_domain, GError ** mcerror);
    static bool ShowInfo();
    static bool MCSetupByArgs (int argc, char **argv, GError ** mcerror);

private:
    static bool ParseMCArgumentE (const char* option_name, const char* value, void* data, GError ** mcerror);
    static bool ParseMCArgumentV (const char* option_name, const char* value, void* data, GError ** mcerror);

private:
    /* If true, show version info and exit */
    static inline gboolean bShowVersion = FALSE;

    static inline GOptionContext *context = nullptr;

#ifdef ENABLE_SUBSHELL
    static inline gboolean bNouseSubshell = FALSE;
#endif /* ENABLE_SUBSHELL */
    static inline gboolean bShowDatadirs = FALSE;
    static inline gboolean bShowDatadirsExtended = FALSE;
#ifdef ENABLE_CONFIGURE_ARGS
    static inline gboolean bShowConfigureOpts = FALSE;
#endif

    static inline GOptionGroup* mainGroup = nullptr;

    static inline const GOptionEntry argument_main_table[] = {
        /* generic options */
        {
            "version",
            'V',
            G_OPTION_FLAG_IN_MAIN,
            G_OPTION_ARG_NONE,
            &bShowVersion,
            N_("Displays the current version"),
            nullptr
        },
        /* options for wrappers */
        {
            "datadir",
            'f',
            G_OPTION_FLAG_IN_MAIN,
            G_OPTION_ARG_NONE,
            &bShowDatadirs,
            N_("Print data directory"),
            nullptr
        },
        /* show extended information about used data directories */
        {
            "datadir-info",
            'F',
            G_OPTION_FLAG_IN_MAIN,
            G_OPTION_ARG_NONE,
            &bShowDatadirsExtended,
            N_("Print extended info about used data directories"),
            nullptr
        },
#ifdef ENABLE_CONFIGURE_ARGS
        /* show configure options */
        {
            "configure-options",
            '\0',
            G_OPTION_FLAG_IN_MAIN,
            G_OPTION_ARG_NONE,
            &bShowConfigureOpts,
            N_("Print configure options"),
            nullptr
        },
#endif

        {
            "printwd",
            'P',
            G_OPTION_FLAG_IN_MAIN,
            G_OPTION_ARG_STRING,
            &Args::last_wd_file,
            N_("Print last working directory to specified file"),
            N_("<file>")
        },

#ifdef ENABLE_SUBSHELL
        {
            "subshell",
            'U',
            G_OPTION_FLAG_IN_MAIN,
            G_OPTION_ARG_NONE,
            &mc_global.tty.use_subshell,
            N_("Enables subshell support (default)"),
            nullptr
        },
        {
            "nosubshell",
            'u',
            G_OPTION_FLAG_IN_MAIN,
            G_OPTION_ARG_NONE,
            &bNouseSubshell,
            N_("Disables subshell support"),
            nullptr
        },
#endif

        /* debug options */
#ifdef ENABLE_VFS_FTP
        {
            "ftplog",
            'l',
            G_OPTION_FLAG_IN_MAIN,
            G_OPTION_ARG_STRING,
            &Args::netfs_logfile,
            N_("Log ftp dialog to specified file"),
            N_("<file>")
        },
#endif /* ENABLE_VFS_FTP */
#ifdef ENABLE_VFS_SMB
        {
            "debuglevel",
            'D',
            G_OPTION_FLAG_IN_MAIN,
            G_OPTION_ARG_INT,
            &Args::debug_level,
            N_("Set debug level"),
            N_("<integer>")
        },
#endif /* ENABLE_VFS_SMB */

        /* handle arguments manually */
        {
            "view",
            'v',
            G_OPTION_FLAG_IN_MAIN | G_OPTION_FLAG_NO_ARG,
            G_OPTION_ARG_CALLBACK,
            (void*)ParseMCArgumentV,
            N_("Launches the file viewer on a file"),
            N_("<file>")
        },
        /* handle arguments manually */
        {
            "edit",
            'e',
            G_OPTION_FLAG_IN_MAIN | G_OPTION_FLAG_NO_ARG,
            G_OPTION_ARG_CALLBACK,
            (void*)ParseMCArgumentE,
            N_("Edit files"),
            N_("<file> ...")
        },
        /* Complete struct initialization */
        {
            nullptr,
            '\0',
            0,
            G_OPTION_ARG_NONE,
            nullptr,
            nullptr,
            nullptr
        }
    };

    static inline GOptionGroup* terminalGroup;
    const static int argsTermObject = 0;
    static inline const GOptionEntry argument_terminal_table[] = {
        /* terminal options */
        {
            "xterm",
            'x',
            argsTermObject,
            G_OPTION_ARG_NONE,
            &Args::bForceXterm,
            N_("Forces xterm features"),
            nullptr
        },
        {
            "no-x11",
            'X',
            argsTermObject,
            G_OPTION_ARG_NONE,
            &mc_global.tty.disable_x11,
            N_("Disable X11 support"),
            nullptr
        },
        {
            "oldmouse",
            'g',
            argsTermObject,
            G_OPTION_ARG_NONE,
            &mc_global.tty.old_mouse,
            N_("Tries to use an old highlight mouse tracking"),
            nullptr
        },
        {
            "nomouse",
            'd',
            argsTermObject,
            G_OPTION_ARG_NONE,
            &Args::bNoMouse,
            N_("Disable mouse support in text version"),
            nullptr
        },
#ifdef HAVE_SLANG
        {
            "termcap",
            't',
            argsTermObject,
            G_OPTION_ARG_NONE,
            &SLtt_Try_Termcap,
            N_("Tries to use termcap instead of terminfo"),
            nullptr
        },
#endif
        {
            "slow",
            's',
            argsTermObject,
            G_OPTION_ARG_NONE,
            &mc_global.tty.slow_terminal,
            N_("To run on slow terminals"),
            nullptr
        },
        {
            "stickchars",
            'a',
            argsTermObject,
            G_OPTION_ARG_NONE,
            &mc_global.tty.ugly_line_drawing,
            N_("Use stickchars to draw"),
            nullptr
        },

#ifdef HAVE_SLANG
        {
            "resetsoft",
            'k',
            argsTermObject,
            G_OPTION_ARG_NONE,
            &reset_hp_softkeys,
            N_("Resets soft keys on HP terminals"),
            nullptr
        },
#endif
        {
            "keymap",
            'K',
            argsTermObject,
            G_OPTION_ARG_STRING,
            &Args::keymap_file,
            N_("Load definitions of key bindings from specified file"),
            N_("<file>")
        },
        {
            "nokeymap",
            '\0',
            argsTermObject,
            G_OPTION_ARG_NONE,
            &Args::bNoKeymap,
            N_("Don't load definitions of key bindings from file, use defaults"),
            nullptr
        },
        /* Complete struct initialization */
        {
            nullptr,
            '\0',
            0,
            G_OPTION_ARG_NONE,
            nullptr,
            nullptr,
            nullptr
        }
    };



    static inline GOptionGroup* colorGroup;
    const static int argsColorOption = 0;
    /* #define ARGS_COLOR_OPTIONS G_OPTION_FLAG_IN_MAIN */
    static inline const GOptionEntry argument_color_table[] = {
        /* color options */
        {
            "nocolor",
            'b',
            argsColorOption,
            G_OPTION_ARG_NONE,
            &mc_global.tty.disable_colors,
            N_("Requests to run in black and white"),
            nullptr
        },
        {
            "color",
            'c',
            argsColorOption,
            G_OPTION_ARG_NONE,
            &Args::bForceColors,
            N_("Request to run in color mode"),
            nullptr
        },
        {
            "colors",
            'C',
            argsColorOption,
            G_OPTION_ARG_STRING,
            &mc_global.tty.command_line_colors,
            N_("Specifies a color configuration"),
            N_("<string>")
        },
        {
            "skin",
            'S',
            argsColorOption,
            G_OPTION_ARG_STRING,
            &mc_global.tty.skin,
            N_("Show mc with specified skin"),
            N_("<string>")
        },
        {
            nullptr,
            '\0',
            0,
            G_OPTION_ARG_NONE,
            nullptr,
            nullptr,
            nullptr /* Complete struct initialization */
        }
    };

    static inline char* locColorsString = nullptr;
    static inline char* locFooterString = nullptr;
    static inline char* locHeaderString = nullptr;
    static inline char* locUsageString = nullptr;

private:
    static void CleanTempHelpStrings();
    static GOptionGroup* NewColorGroup();
    static char* AddUsageInfo();
    static void AddExtendedInfoToHelp();
    static char* ConvertHelpToSyscharset (const char* charset, const char* error_message_str, const char* help_str);

    /**
     * Get list of filenames (and line numbers) from command line, when mc called as editor
     *
     * @param argc count of all arguments
     * @param argv array of strings, contains arguments
     * @return list of mcedit_arg_t objects
     */
    static GList* ParseMceditArguments (int argc, char **argv);
};


