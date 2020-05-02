/*
   Handle command line arguments.

   Copyright (C) 2009-2020
   Free Software Foundation, Inc.

   Written by:
   Slava Zanko <slavazanko@gmail.com>, 2009.
   Andrew Borodin <aborodin@vmail.ru>, 2011, 2012.

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

#include <cstdlib>
#include <cstdio>
#include <iostream>

#include "lib/global.hpp"
#include "lib/tty/tty.hpp"
#include "lib/strutil.hpp"
#include "lib/vfs/vfs.hpp"
#include "lib/util.hpp"           /* x_basename() */

#ifdef ENABLE_VFS_SMB
#include "src/vfs/smbfs/smbfs.hpp"        /* smbfs_set_debugf()  */
#endif

#include "src/textconf.hpp"

#include "src/args.hpp"

bool Args::Parse (int *argc, char ***argv, const char *translation_domain, GError ** mcerror)
{
    bool ok = true;

    mc_return_val_if_error (mcerror, FALSE);

    const char*_system_codepage = str_detect_termencoding ();

#ifdef ENABLE_NLS
    if (!str_isutf8 (_system_codepage))
        bind_textdomain_codeset ("mc", "UTF-8");
#endif

    context = g_option_context_new (AddUsageInfo());

    g_option_context_set_ignore_unknown_options (context, FALSE);

    AddExtendedInfoToHelp();

    mainGroup = g_option_group_new ("main", _("Main options"), _("Main options"), NULL, NULL);

    g_option_group_add_entries (mainGroup, argument_main_table);
    g_option_context_set_main_group (context, mainGroup);
    g_option_group_set_translation_domain (mainGroup, translation_domain);

    terminalGroup = g_option_group_new ("terminal", _("Terminal options"),
                                         _("Terminal options"), NULL, NULL);

    g_option_group_add_entries (terminalGroup, argument_terminal_table);
    g_option_context_add_group (context, terminalGroup);
    g_option_group_set_translation_domain (terminalGroup, translation_domain);

    colorGroup = NewColorGroup();

    g_option_group_add_entries (colorGroup, argument_color_table);
    g_option_context_add_group (context, colorGroup);
    g_option_group_set_translation_domain (colorGroup, translation_domain);

    if (!g_option_context_parse (context, argc, argv, mcerror))
    {
        if (*mcerror == NULL)
            mc_propagate_error (mcerror, 0, "%s\n", _("Arguments parse error!"));
        else
        {
            std::string help_str = g_option_context_get_help (context, TRUE, NULL);

            if (str_isutf8 (_system_codepage))
                mc_replace_error (mcerror, (*mcerror)->code, "%s\n\n%s\n", (*mcerror)->message, help_str.c_str());
            else
            {
                std::string full_help_str =
                        ConvertHelpToSyscharset(_system_codepage, (*mcerror)->message, help_str.c_str());
                mc_replace_error (mcerror, (*mcerror)->code, "%s", full_help_str.c_str());
            }
        }

        ok = false;
    }

    g_option_context_free (context);
    CleanTempHelpStrings();

#ifdef ENABLE_NLS
    if (!str_isutf8 (_system_codepage))
        bind_textdomain_codeset ("mc", _system_codepage);
#endif

    return ok;
}

bool Args::ShowInfo()
{
    if (bShowVersion)
    {
        TextConf::show_version ();
        return false;
    }

    if (bShowDatadirs)
    {
        std::cout << mc_global.sysconfig_dir << " (" << mc_global.share_data_dir << ")" << std::endl;
        return false;
    }

    if (bShowDatadirsExtended)
    {
        TextConf::show_datadirs_extended ();
        return false;
    }

#ifdef ENABLE_CONFIGURE_ARGS
    if (bShowConfigureOpts)
    {
        TextConf::show_configure_options ();
        return false;
    }
#endif

    return true;
}

bool Args::MCSetupByArgs (int argc, char **argv, GError ** mcerror)
{
    mc_return_val_if_error (mcerror, FALSE);

    if (Args::bForceColors)
        mc_global.tty.disable_colors = FALSE;

#ifdef ENABLE_SUBSHELL
    if (bNouseSubshell)
        mc_global.tty.use_subshell = FALSE;
#endif /* ENABLE_SUBSHELL */

#ifdef ENABLE_VFS_SMB
    if (Args::debug_level != 0)
        smbfs_set_debug (Args::debug_level);
#endif /* ENABLE_VFS_SMB */

    if (Args::netfs_logfile)
    {
        vfs_path_t *vpath;
#ifdef ENABLE_VFS_FTP
        vpath = vfs_path_from_str ("ftp://");
        mc_setctl (vpath, VFS_SETCTL_LOGFILE, (void *) Args::netfs_logfile);
        vfs_path_free (vpath);
#endif /* ENABLE_VFS_FTP */
#ifdef ENABLE_VFS_SMB
        vpath = vfs_path_from_str ("smb://");
        mc_setctl (vpath, VFS_SETCTL_LOGFILE, (void *) Args::netfs_logfile);
        vfs_path_free (vpath);
#endif /* ENABLE_VFS_SMB */
    }

    char *tmp = (argc > 0) ? argv[1] : nullptr;

    switch (mc_global.GetRunMode())
    {
        case Global::RunMode::MC_RUN_EDITOR:
            Args::mc_run_param0 = ParseMceditArguments(argc - 1, &argv[1]);
            break;

        case Global::RunMode::MC_RUN_VIEWER:
            if (!tmp)
            {
                mc_propagate_error (mcerror, 0, "%s\n", _("No arguments given to the viewer."));
                return FALSE;
            }

            Args::mc_run_param0 = g_strdup (tmp);
            break;

#ifdef USE_DIFF_VIEW
        case Global::RunMode::MC_RUN_DIFFVIEWER:
            if (argc < 3)
            {
                mc_propagate_error (mcerror, 0, "%s\n",
                                    _("Two files are required to envoke the diffviewer."));
                return FALSE;
            }
            MC_FALLTHROUGH;
#endif /* USE_DIFF_VIEW */

        case Global::RunMode::MC_RUN_FULL:
        default:
            /* set the current dir and the other dir for filemanager,
               or two files for diff viewer */
            if (tmp)
            {
                Args::mc_run_param0 = g_strdup (tmp);
                tmp = (argc > 1) ? argv[2] : nullptr;
                if (tmp)
                    Args::mc_run_param1 = g_strdup (tmp);
            }
            break;
    }

    return TRUE;
}

bool Args::ParseMCArgumentE (const char* /* option_name */, const char* /* value */, void* /* data */, GError ** mcerror)
{
    mc_return_val_if_error (mcerror, false);

    mc_global.SetRunMode(Global::RunMode::MC_RUN_EDITOR);

    return true;
}

bool Args::ParseMCArgumentV (const char* /* option_name */, const char* /* value */, void* /* data */, GError ** mcerror)
{
    mc_return_val_if_error (mcerror, false);

    mc_global.SetRunMode(Global::RunMode::MC_RUN_VIEWER);

    return true;
}

void Args::CleanTempHelpStrings()
{
    MC_PTR_FREE (locColorsString);
    MC_PTR_FREE (locFooterString);
    MC_PTR_FREE (locHeaderString);
    MC_PTR_FREE (locUsageString);
}

GOptionGroup* Args::NewColorGroup()
{
/* *INDENT-OFF* */
    /* FIXME: to preserve translations, lines should be split. */
    locColorsString = g_strdup_printf ("%s\n%s",
            /* TRANSLATORS: don't translate keywords */
                                       _("--colors KEYWORD={FORE},{BACK},{ATTR}:KEYWORD2=...\n\n"
                                         "{FORE}, {BACK} and {ATTR} can be omitted, and the default will be used\n"
                                         "\n Keywords:\n"
                                         "   Global:       errors, disabled, reverse, gauge, header\n"
                                         "                 input, inputmark, inputunchanged, commandlinemark\n"
                                         "                 bbarhotkey, bbarbutton, statusbar\n"
                                         "   File display: normal, selected, marked, markselect\n"
                                         "   Dialog boxes: dnormal, dfocus, dhotnormal, dhotfocus, errdhotnormal,\n"
                                         "                 errdhotfocus\n"
                                         "   Menus:        menunormal, menuhot, menusel, menuhotsel, menuinactive\n"
                                         "   Popup menus:  pmenunormal, pmenusel, pmenutitle\n"
                                         "   Editor:       editnormal, editbold, editmarked, editwhitespace,\n"
                                         "                 editlinestate, editbg, editframe, editframeactive\n"
                                         "                 editframedrag\n"
                                         "   Viewer:       viewnormal,viewbold, viewunderline, viewselected\n"
                                         "   Help:         helpnormal, helpitalic, helpbold, helplink, helpslink\n"),
            /* TRANSLATORS: don't translate color names and attributes */
                                       _("Standard Colors:\n"
                                         "   black, gray, red, brightred, green, brightgreen, brown,\n"
                                         "   yellow, blue, brightblue, magenta, brightmagenta, cyan,\n"
                                         "   brightcyan, lightgray and white\n\n"
                                         "Extended colors, when 256 colors are available:\n"
                                         "   color16 to color255, or rgb000 to rgb555 and gray0 to gray23\n\n"
                                         "Attributes:\n"
                                         "   bold, italic, underline, reverse, blink; append more with '+'\n")
    );
/* *INDENT-ON* */

    return g_option_group_new ("color", locColorsString,
                               _("Color options"), NULL, NULL);

}

char* Args::AddUsageInfo()
{
    char* s;

    switch (mc_global.GetRunMode())
    {
        case Global::RunMode::MC_RUN_EDITOR:
            s = g_strdup_printf ("%s\n", _("[+lineno] file1[:lineno] [file2[:lineno]...]"));
            break;
        case Global::RunMode::MC_RUN_VIEWER:
            s = g_strdup_printf ("%s\n", _("file"));
            break;
#ifdef USE_DIFF_VIEW
        case Global::RunMode::MC_RUN_DIFFVIEWER:
            s = g_strdup_printf ("%s\n", _("file1 file2"));
            break;
#endif /* USE_DIFF_VIEW */
        case Global::RunMode::MC_RUN_FULL:
        default:
            s = g_strdup_printf ("%s\n", _("[this_dir] [other_panel_dir]"));
    }

    locUsageString = s;

    return locUsageString;
}

void Args::AddExtendedInfoToHelp()
{
    locFooterString = g_strdup_printf ("%s",
                                       _
                                       ("\n"
                                        "Please send any bug reports (including the output of 'mc -V')\n"
                                        "as tickets at www.midnight-commander.org\n"));
    locHeaderString = g_strdup_printf (_("GNU Midnight Commander %s\n"), VERSION);

    g_option_context_set_description (context, locFooterString);
    g_option_context_set_summary (context, locHeaderString);
}

char* Args::ConvertHelpToSyscharset (const char* charset, const char* error_message_str, const char* help_str)
{
    GString* buffer = g_string_new ("");
    GIConv conv = g_iconv_open (charset, "UTF-8");
    char* full_help_str = g_strdup_printf ("%s\n\n%s\n", error_message_str, help_str);

    str_convert (conv, full_help_str, buffer);

    g_free (full_help_str);
    g_iconv_close (conv);

    return g_string_free (buffer, FALSE);
}

GList* Args::ParseMceditArguments (int argc, char **argv)
{
    GList *flist = nullptr;
    long first_line_number = -1;

    for (int i = 0; i < argc; i++)
    {

        char *end, *p;
        Args *arg;

        char *tmp = argv[i];

        /*
         * First, try to get line number as +lineno.
         */
        if (*tmp == '+')
        {
            long lineno;
            char *error;

            lineno = strtol (tmp + 1, &error, 10);

            if (*error == '\0')
            {
                /* this is line number */
                first_line_number = lineno;
                continue;
            }
            /* this is file name */
        }

        /*
         * Check for filename:lineno, followed by an optional colon.
         * This format is used by many programs (especially compilers)
         * in error messages and warnings. It is supported so that
         * users can quickly copy and paste file locations.
         */
        end = tmp + strlen (tmp);
        p = end;

        if (p > tmp && p[-1] == ':')
            p--;
        while (p > tmp && g_ascii_isdigit ((gchar) p[-1]))
            p--;

        if (tmp < p && p < end && p[-1] == ':')
        {
            struct stat st;

            char *fname = g_strndup (tmp, p - 1 - tmp);
            vfs_path_t *tmp_vpath = vfs_path_from_str (tmp);
            vfs_path_t *fname_vpath = vfs_path_from_str (fname);

            /*
             * Check that the file before the colon actually exists.
             * If it doesn't exist, create new file.
             */
            if (mc_stat (tmp_vpath, &st) == -1 && mc_stat (fname_vpath, &st) != -1)
            {
                arg = new Args(fname_vpath, atoi (p));
                vfs_path_free (tmp_vpath);
            }
            else
            {
                arg = new Args(tmp_vpath, 0);
                vfs_path_free (fname_vpath);
            }

            g_free (fname);
        }
        else
            arg = new Args(vfs_path_from_str (tmp), 0);

        flist = g_list_prepend (flist, arg);
    }

    if (!flist)
        flist = g_list_prepend (flist, new Args(vfs_path_from_str (NULL), 0));
    else if (first_line_number != -1)
    {
        /* overwrite line number for first file */
        GList *l = g_list_last (flist);
        ((Args *) l->data)->SetLineNumber(first_line_number);
    }

    return flist;
}


Global::RunMode Args::SetupRunMode(const char* base)
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

void Args::Free(Args* arg)
{
    vfs_path_free (arg->file_vpath);
    g_free (arg);
}