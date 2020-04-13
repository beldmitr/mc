#pragma once

#include "lib/global.hpp"         /* gboolean */
#include "lib/vfs/vfs.hpp"        /* vfs_path_t */

/*** typedefs(not structures) and defined constants **********************************************/

/*** enums ***************************************************************************************/

/*** structures declarations (and typedefs of structures)*****************************************/

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
    static void mcedit_arg_free (Args * arg)
    {
        vfs_path_free (arg->file_vpath);
        g_free (arg);
    }

    static Global::RunMode mc_setup_run_mode (const char* base)
    {
        if (std::strncmp (base, "mce", 3) == 0 || std::strcmp (base, "vi") == 0)
        {
            /* mce* or vi is link to mc */
            return Global::RunMode::MC_RUN_EDITOR;
        }
        else if (std::strncmp (base, "mcv", 3) == 0 || std::strcmp (base, "view") == 0)
        {
            /* mcv* or view is link to mc */
            return Global::RunMode::MC_RUN_VIEWER;
        }
#ifdef USE_DIFF_VIEW
        else if (std::strncmp (base, "mcd", 3) == 0 || std::strcmp (base, "diff") == 0)
        {
            /* mcd* or diff is link to mc */
            return Global::RunMode::MC_RUN_DIFFVIEWER;
        }
#endif /* USE_DIFF_VIEW */

        return Global::RunMode::MC_RUN_FULL;
    }
private:
    vfs_path_t *file_vpath;
    long line_number;
};

/*** global variables defined in .c file *********************************************************/

extern bool mc_args__force_xterm;
extern bool mc_args__nomouse;
extern bool mc_args__force_colors;
extern bool mc_args__nokeymap;
extern char *mc_args__last_wd_file;
extern char *mc_args__netfs_logfile;
extern char *mc_args__keymap_file;
#ifdef ENABLE_VFS_SMB
extern int mc_args__debug_level;
#endif

/*
 * MC_RUN_FULL: dir for left panel
 * MC_RUN_EDITOR: list of files to edit
 * MC_RUN_VIEWER: file to view
 * MC_RUN_DIFFVIEWER: first file to compare
 */
extern void *mc_run_param0;
/*
 * MC_RUN_FULL: dir for right panel
 * MC_RUN_EDITOR: unused
 * MC_RUN_VIEWER: unused
 * MC_RUN_DIFFVIEWER: second file to compare
 */
extern char *mc_run_param1;

/*** declarations of public functions ************************************************************/

bool mc_args_parse (int *argc, char ***argv, const char *translation_domain, GError ** mcerror);
bool mc_args_show_info ();
bool mc_setup_by_args (int argc, char **argv, GError ** mcerror);



/*** inline functions ****************************************************************************/
