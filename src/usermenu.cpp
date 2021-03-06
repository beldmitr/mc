/*
   User Menu implementation

   Copyright (C) 1994-2020
   Free Software Foundation, Inc.

   Written by:
   Slava Zanko <slavazanko@gmail.com>, 2013
   Andrew Borodin <aborodin@vmail.ru>, 2013

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

/** \file usermenu.c
 *  \brief Source: user menu implementation
 */

#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "lib/global.hpp"
#include "lib/fileloc.hpp"
#include "lib/search.hpp"
#include "lib/vfs/vfs.hpp"
#include "lib/strutil.hpp"
#include "lib/util.hpp"
#include "lib/widget.hpp"

#include "src/editor/edit.hpp"      /* WEdit, BLOCK_FILE */
#include "src/viewer/mcviewer.hpp"  /* for default_* externs */

#include "src/execute.hpp"
#include "src/setup.hpp"
#include "src/history.hpp"

#include "src/filemanager/midnight.hpp"
#include "src/filemanager/layout.hpp"

#include "usermenu.hpp"

gboolean UserMenu::Cmd(const WEdit * edit_widget, const char *menu_file, int selected_entry)
{
    char *p;
    char *data, **entries;
    int max_cols, menu_lines, menu_limit;
    int col, i;
    gboolean accept_entry = TRUE;
    int selected;
    gboolean old_patterns;
    gboolean res = FALSE;
    gboolean interactive = TRUE;

    if (!vfs_current_is_local ())
    {
        message (D_ERROR, MSG_ERROR, "%s", _("Cannot execute commands on non-local filesystems"));
        return FALSE;
    }
    if (menu_file != nullptr)
        menu = g_strdup(menu_file);
    else
        menu = g_strdup (edit_widget != nullptr ? EDIT_LOCAL_MENU : MC_LOCAL_MENU);
    if (!exist_file(menu) || !MenuFileOwn(menu))
    {
        if (menu_file != nullptr)
        {
            message (D_ERROR, MSG_ERROR, _("Cannot open file %s\n%s"), menu,
                     unix_error_string (errno));
            MC_PTR_FREE (menu);
            return FALSE;
        }

        g_free (menu);
        if (edit_widget != nullptr)
            menu = mc_config_get_full_path (EDIT_HOME_MENU);
        else
            menu = mc_config_get_full_path (MC_USERMENU_FILE);

        if (!exist_file (menu))
        {
            g_free (menu);
            menu =
                    mc_build_filename (mc_config_get_home_dir (),
                                       edit_widget != nullptr ? EDIT_GLOBAL_MENU : MC_GLOBAL_MENU,
                                       (char *) NULL);
            if (!exist_file (menu))
            {
                g_free (menu);
                menu =
                        mc_build_filename (mc_global.sysconfig_dir.c_str(),
                                           edit_widget != nullptr ? EDIT_GLOBAL_MENU : MC_GLOBAL_MENU,
                                           (char *) NULL);
                if (!exist_file (menu))
                {
                    g_free (menu);
                    menu =
                            mc_build_filename (mc_global.share_data_dir.c_str(),
                                               edit_widget != NULL ? EDIT_GLOBAL_MENU : MC_GLOBAL_MENU,
                                               (char *) NULL);
                }
            }
        }
    }

    if (!g_file_get_contents (menu, &data, NULL, NULL))
    {
        message (D_ERROR, MSG_ERROR, _("Cannot open file %s\n%s"), menu, unix_error_string (errno));
        MC_PTR_FREE (menu);
        return FALSE;
    }

    max_cols = 0;
    selected = 0;
    menu_limit = 0;
    entries = nullptr;

    /* Parse the menu file */
    old_patterns = easy_patterns;
    p = CheckPatterns(data);
    for (menu_lines = col = 0; *p != '\0'; str_next_char (&p))
    {
        if (menu_lines >= menu_limit)
        {
            char **new_entries;

            menu_limit += MAX_ENTRIES;
            new_entries = static_cast<char **>(g_try_realloc(entries, sizeof(new_entries[0]) * menu_limit));
            if (new_entries == nullptr)
                break;

            entries = new_entries;
            new_entries += menu_limit;
            while (--new_entries >= &entries[menu_lines])
                *new_entries = nullptr;
        }

        if (col == 0 && entries[menu_lines] == nullptr)
            switch (*p)
            {
                case '#':
                    /* show prompt if first line of external script is #interactive */
                    if (selected_entry >= 0 && strncmp (p, "#silent", 7) == 0)
                        interactive = FALSE;
                    /* A commented menu entry */
                    accept_entry = TRUE;
                    break;

                case '+':
                    if (*(p + 1) == '=')
                    {
                        /* Combined adding and default */
                        p = TestLine(edit_widget, p + 1, &accept_entry);
                        if (selected == 0 && accept_entry)
                            selected = menu_lines;
                    }
                    else
                    {
                        /* A condition for adding the entry */
                        p = TestLine(edit_widget, p, &accept_entry);
                    }
                    break;

                case '=':
                    if (*(p + 1) == '+')
                    {
                        /* Combined adding and default */
                        p = TestLine(edit_widget, p + 1, &accept_entry);
                        if (selected == 0 && accept_entry)
                            selected = menu_lines;
                    }
                    else
                    {
                        /* A condition for making the entry default */
                        i = 1;
                        p = TestLine(edit_widget, p, &i);
                        if (selected == 0 && i != 0)
                            selected = menu_lines;
                    }
                    break;

                default:
                    if (!whitespace (*p) && str_isprint (p))
                    {
                        /* A menu entry title line */
                        if (accept_entry)
                            entries[menu_lines] = p;
                        else
                            accept_entry = TRUE;
                    }
                    break;
            }

        if (*p == '\n')
        {
            if (entries[menu_lines] != nullptr)
            {
                menu_lines++;
                accept_entry = TRUE;
            }
            max_cols = MAX (max_cols, col);
            col = 0;
        }
        else
        {
            if (*p == '\t')
                *p = ' ';
            col++;
        }
    }

    if (menu_lines == 0)
    {
        message (D_ERROR, MSG_ERROR, _("No suitable entries found in %s"), menu);
        res = FALSE;
    }
    else
    {
        if (selected_entry >= 0)
            selected = selected_entry;
        else
        {
            Listbox *listbox;

            max_cols = MIN (MAX (max_cols, col), MAX_ENTRY_LEN);

            /* Create listbox */
            listbox = create_listbox_window (menu_lines, max_cols + 2, _("User menu"),
                                             "[Edit Menu File]");
            /* insert all the items found */
            for (i = 0; i < menu_lines; i++)
            {
                p = entries[i];
                LISTBOX_APPEND_TEXT (listbox, (unsigned char) p[0],
                                     extract_line (p, p + MAX_ENTRY_LEN), p, FALSE);
            }
            /* Select the default entry */
            listbox_select_entry (listbox->list, selected);

            selected = run_listbox (listbox);
        }
        if (selected >= 0)
        {
            ExecuteMenuCommand(edit_widget, entries[selected], interactive);
            res = TRUE;
        }

        do_refresh ();
    }

    easy_patterns = old_patterns;
    MC_PTR_FREE (menu);
    g_free (entries);
    g_free (data);
    return res;
}

char* UserMenu::ExpandFormat(const WEdit * edit_widget, char c, gboolean do_quote)
{
    WPanel *panel = nullptr;
    char *(*quote_func) (const char *, gboolean);
    const char *fname = nullptr;
    char *result;
    char c_lc;

#ifndef USE_INTERNAL_EDIT
    (void) edit_widget;
#endif

    if (c == '%')
        return g_strdup ("%");

    switch (mc_global.GetRunMode())
    {
        case Global::RunMode::MC_RUN_FULL:
#ifdef USE_INTERNAL_EDIT
            if (edit_widget != nullptr)
                fname = edit_get_file_name (edit_widget);
            else
#endif
            {
                if (g_ascii_islower ((gchar) c))
                    panel = current_panel;
                else
                {
                    if (get_other_type () != view_listing)
                        return g_strdup ("");
                    panel = other_panel;
                }

                fname = panel->dir.list[panel->selected].fname;
            }
            break;

#ifdef USE_INTERNAL_EDIT
        case Global::RunMode::MC_RUN_EDITOR:
            fname = edit_get_file_name (edit_widget);
            break;
#endif

        default:
            /* other modes don't use formats */
            return g_strdup ("");
    }

    if (do_quote)
        quote_func = name_quote;
    else
        quote_func = fake_name_quote;

    c_lc = g_ascii_tolower ((gchar) c);

    switch (c_lc)
    {
        case 'f':
        case 'p':
            result = quote_func (fname, FALSE);
            goto ret;
        case 'x':
            result = quote_func (extension (fname), FALSE);
            goto ret;
        case 'd':
        {
            const char *cwd;
            char *qstr;

            if (panel != nullptr)
                cwd = vfs_path_as_str (panel->cwd_vpath);
            else
                cwd = vfs_get_current_dir ();

            qstr = quote_func (cwd, FALSE);

            result = qstr;
            goto ret;
        }
        case 'c':
#ifdef USE_INTERNAL_EDIT
            if (edit_widget != nullptr)
            {
                result = g_strdup_printf ("%u", (unsigned int) edit_get_cursor_offset (edit_widget));
                goto ret;
            }
#endif
            break;
        case 'i':                  /* indent equal number cursor position in line */
#ifdef USE_INTERNAL_EDIT
            if (edit_widget != nullptr)
            {
                result = g_strnfill (edit_get_curs_col (edit_widget), ' ');
                goto ret;
            }
#endif
            break;
        case 'y':                  /* syntax type */
#ifdef USE_INTERNAL_EDIT
            if (edit_widget != nullptr)
            {
                const char *syntax_type;

                syntax_type = edit_get_syntax_type (edit_widget);
                if (syntax_type != nullptr)
                {
                    result = g_strdup (syntax_type);
                    goto ret;
                }
            }
#endif
            break;
        case 'k':                  /* block file name */
        case 'b':                  /* block file name / strip extension */
#ifdef USE_INTERNAL_EDIT
            if (edit_widget != nullptr)
            {
                char *file;

                file = mc_config_get_full_path (EDIT_BLOCK_FILE);
                result = quote_func (file, FALSE);
                g_free (file);
                goto ret;
            }
#endif
            if (c_lc == 'b')
            {
                result = StripExt(quote_func (fname, FALSE));
                goto ret;
            }
            break;
        case 'n':                  /* strip extension in editor */
#ifdef USE_INTERNAL_EDIT
            if (edit_widget != nullptr)
            {
                result = StripExt(quote_func (fname, FALSE));
                goto ret;
            }
#endif
            break;
        case 'm':                  /* menu file name */
            if (menu != nullptr)
            {
                result = quote_func (menu, FALSE);
                goto ret;
            }
            break;
        case 's':
            if (panel == nullptr || panel->marked == 0)
            {
                result = quote_func (fname, FALSE);
                goto ret;
            }

            MC_FALLTHROUGH;

        case 't':
        case 'u':
        {
            GString *block;
            int i;

            if (panel == nullptr)
            {
                result = g_strdup ("");
                goto ret;
            }

            block = g_string_sized_new (16);

            for (i = 0; i < panel->dir.len; i++)
                if (panel->dir.list[i].f.marked)
                {
                    char *tmp;

                    tmp = quote_func (panel->dir.list[i].fname, FALSE);
                    g_string_append (block, tmp);
                    g_string_append_c (block, ' ');
                    g_free (tmp);

                    if (c_lc == 'u')
                        do_file_mark (panel, i, 0);
                }
            result = g_string_free (block, FALSE);
            goto ret;
        }                       /* sub case block */
        default:
            break;
    }                           /* switch */

    result = g_strdup ("% ");
    result[1] = c;
    ret:
    return result;
}

int UserMenu::CheckFormatView(const char *p)
{
    const char *q = p;

    if (strncmp (p, "view", 4) == 0)
    {
        q += 4;
        if (*q == '{')
        {
            for (q++; *q != '\0' && *q != '}'; q++)
            {
                if (strncmp (q, DEFAULT_CHARSET, 5) == 0)
                {
                    mcview_global_flags.hex = FALSE;
                    q += 4;
                }
                else if (strncmp (q, "hex", 3) == 0)
                {
                    mcview_global_flags.hex = TRUE;
                    q += 2;
                }
                else if (strncmp (q, "nroff", 5) == 0)
                {
                    mcview_global_flags.nroff = TRUE;
                    q += 4;
                }
                else if (strncmp (q, "unform", 6) == 0)
                {
                    mcview_global_flags.nroff = FALSE;
                    q += 5;
                }
            }
            if (*q == '}')
                q++;
        }
        return q - p;
    }
    return 0;
}

int UserMenu::CheckFormatVar(const char *p, char **v)
{
    *v = nullptr;

    if (strncmp (p, "var{", 4) == 0)
    {
        const char *q = p;
        const char *dots = nullptr;
        const char *value;
        char *var_name;

        for (q += 4; *q != '\0' && *q != '}'; q++)
        {
            if (*q == ':')
                dots = q + 1;
        }
        if (*q == '\0')
            return 0;

        if (dots == nullptr || dots == q + 5)
        {
            message (D_ERROR,
                     _("Format error on file Extensions File"),
                     !dots ? _("The %%var macro has no default")
                           : _("The %%var macro has no variable"));
            return 0;
        }

        /* Copy the variable name */
        var_name = g_strndup (p + 4, dots - 2 - (p + 3));
        value = getenv (var_name);
        g_free (var_name);

        if (value != nullptr)
            *v = g_strdup (value);
        else
            *v = g_strndup (dots, q - dots);

        return q - p;
    }
    return 0;
}

int UserMenu::CheckFormatCD(const char *p)
{
    return (strncmp (p, "cd", 2)) != 0 ? 0 : 3;
}

char* UserMenu::StripExt(char *ss)
{
    char *s = ss;
    char *e = nullptr;

    while (*s != '\0')
    {
        if (*s == '.')
            e = s;
        if (IS_PATH_SEP (*s) && e != nullptr)
            e = nullptr;           /* '.' in *directory* name */
        s++;
    }
    if (e != nullptr)
        *e = '\0';
    return ss;
}

char* UserMenu::CheckPatterns(char *p)
{
    static const char def_name[] = "shell_patterns=";
    char *p0 = p;

    if (strncmp (p, def_name, sizeof (def_name) - 1) != 0)
        return p0;

    p += sizeof (def_name) - 1;
    if (*p == '1')
        easy_patterns = TRUE;
    else if (*p == '0')
        easy_patterns = FALSE;
    else
        return p0;

    /* Skip spaces */
    p++;
    while (whiteness (*p))
        p++;
    return p;
}

char* UserMenu::ExtractArg(char *p, char *arg, int size)
{
    while (*p != '\0' && whiteness (*p))
        p++;

    /* support quote space .mnu */
    while (*p != '\0' && (*p != ' ' || *(p - 1) == '\\') && *p != '\t' && *p != '\n')
    {
        char *np;

        np = str_get_next_char (p);
        if (np - p >= size)
            break;
        memcpy (arg, p, np - p);
        arg += np - p;
        size -= np - p;
        p = np;
    }
    *arg = '\0';
    if (*p == '\0' || *p == '\n')
        str_prev_char (&p);
    return p;
}

gboolean UserMenu::TestType(WPanel * panel, char *arg)
{
    int result = 0;             /* False by default */
    mode_t st_mode = panel->dir.list[panel->selected].st.st_mode;

    for (; *arg != '\0'; arg++)
    {
        switch (*arg)
        {
            case 'n':              /* Not a directory */
                result |= !S_ISDIR (st_mode);
                break;
            case 'r':              /* Regular file */
                result |= S_ISREG (st_mode);
                break;
            case 'd':              /* Directory */
                result |= S_ISDIR (st_mode);
                break;
            case 'l':              /* Link */
                result |= S_ISLNK (st_mode);
                break;
            case 'c':              /* Character special */
                result |= S_ISCHR (st_mode);
                break;
            case 'b':              /* Block special */
                result |= S_ISBLK (st_mode);
                break;
            case 'f':              /* Fifo (named pipe) */
                result |= S_ISFIFO (st_mode);
                break;
            case 's':              /* Socket */
                result |= S_ISSOCK (st_mode);
                break;
            case 'x':              /* Executable */
                result |= (st_mode & 0111) != 0 ? 1 : 0;
                break;
            case 't':
                result |= panel->marked != 0 ? 1 : 0;
                break;
            default:
                debugError = TRUE;
                break;
        }
    }

    return (result != 0);
}

char* UserMenu::TestCondition(const WEdit * edit_widget, char *p, gboolean * condition)
{
    char arg[256];
    const mc_search_type_t search_type = easy_patterns ? MC_SEARCH_T_GLOB : MC_SEARCH_T_REGEX;

    /* Handle one condition */
    for (; *p != '\n' && *p != '&' && *p != '|'; p++)
    {
        WPanel *panel = nullptr;

        /* support quote space .mnu */
        if ((*p == ' ' && *(p - 1) != '\\') || *p == '\t')
            continue;
        if (*p >= 'a')
            panel = current_panel;
        else if (get_other_type () == view_listing)
            panel = other_panel;

        *p |= 0x20;

        switch (*p++)
        {
            case '!':
                p = TestCondition(edit_widget, p, condition);
                *condition = !*condition;
                str_prev_char (&p);
                break;
            case 'f':              /* file name pattern */
                p = ExtractArg(p, arg, sizeof (arg));
#ifdef USE_INTERNAL_EDIT
                if (edit_widget != nullptr)
                {
                    const char *edit_filename;

                    edit_filename = edit_get_file_name (edit_widget);
                    *condition = mc_search (arg, DEFAULT_CHARSET, edit_filename, search_type);
                }
                else
#endif
                    *condition = panel != nullptr &&
                                 mc_search (arg, DEFAULT_CHARSET, panel->dir.list[panel->selected].fname,
                                            search_type);
                break;
            case 'y':              /* syntax pattern */
#ifdef USE_INTERNAL_EDIT
                if (edit_widget != nullptr)
                {
                    const char *syntax_type;

                    syntax_type = edit_get_syntax_type (edit_widget);
                    if (syntax_type != nullptr)
                    {
                        p = ExtractArg(p, arg, sizeof (arg));
                        *condition = mc_search (arg, DEFAULT_CHARSET, syntax_type, MC_SEARCH_T_NORMAL);
                    }
                }
#endif
                break;
            case 'd':
                p = ExtractArg(p, arg, sizeof (arg));
                *condition = panel != nullptr
                             && mc_search (arg, DEFAULT_CHARSET, vfs_path_as_str (panel->cwd_vpath),
                                           search_type);
                break;
            case 't':
                p = ExtractArg(p, arg, sizeof (arg));
                *condition = panel != nullptr && TestType(panel, arg);
                break;
            case 'x':              /* executable */
            {
                struct stat status;

                p = ExtractArg(p, arg, sizeof (arg));
                *condition = stat (arg, &status) == 0 && is_exe (status.st_mode);
                break;
            }
            default:
                debugError = TRUE;
                break;
        }                       /* switch */
    }                           /* while */
    return p;
}

void UserMenu::DebugOut(char *start, char *end, gboolean condition)
{
    static char *msg = nullptr;

    if (start == nullptr && end == nullptr)
    {
        /* Show output */
        if (debugFlag && msg != nullptr)
        {
            size_t len = strlen (msg);
            if (len != 0)
                msg[len - 1] = '\0';
            message (D_NORMAL, _("Debug"), "%s", msg);

        }
        debugFlag = FALSE;
        MC_PTR_FREE (msg);
    }
    else
    {
        const char *type;
        char *p;

        /* Save debug info for later output */
        if (!debugFlag)
            return;
        /* Save the result of the condition */
        if (debugError)
        {
            type = _("ERROR:");
            debugError = FALSE;
        }
        else if (condition)
            type = _("True:");
        else
            type = _("False:");
        /* This is for debugging, don't need to be super efficient.  */
        if (end == nullptr)
            p = g_strdup_printf ("%s %s %c \n", msg ? msg : "", type, *start);
        else
            p = g_strdup_printf ("%s %s %.*s \n", msg ? msg : "", type, (int) (end - start), start);
        g_free (msg);
        msg = p;
    }
}

char* UserMenu::TestLine(const WEdit * edit_widget, char *p, gboolean * result)
{
    char Operator;

    /* Repeat till end of line */
    while (*p != '\0' && *p != '\n')
    {
        char *debug_start, *debug_end;
        gboolean condition = TRUE;

        /* support quote space .mnu */
        while ((*p == ' ' && *(p - 1) != '\\') || *p == '\t')
            p++;
        if (*p == '\0' || *p == '\n')
            break;
        Operator = *p++;
        if (*p == '?')
        {
            debugFlag = TRUE;
            p++;
        }
        /* support quote space .mnu */
        while ((*p == ' ' && *(p - 1) != '\\') || *p == '\t')
            p++;
        if (*p == '\0' || *p == '\n')
            break;

        debug_start = p;
        p = TestCondition(edit_widget, p, &condition);
        debug_end = p;
        /* Add one debug statement */
        DebugOut(debug_start, debug_end, condition);

        switch (Operator)
        {
            case '+':
            case '=':
                /* Assignment */
                *result = condition;
                break;
            case '&':              /* Logical and */
                *result = *result && condition;
                break;
            case '|':              /* Logical or */
                *result = *result || condition;
                break;
            default:
                debugError = TRUE;
                break;
        }                       /* switch */
        /* Add one debug statement */
        DebugOut(&Operator, NULL, *result);

    }                           /* while (*p != '\n') */
    /* Report debug message */
    DebugOut(NULL, NULL, TRUE);

    if (*p == '\0' || *p == '\n')
        str_prev_char (&p);
    return p;
}

void UserMenu::ExecuteMenuCommand(const WEdit * edit_widget, const char *commands, gboolean show_prompt)
{
    FILE *cmd_file;
    int cmd_file_fd;
    gboolean expand_prefix_found = FALSE;
    char *parameter = nullptr;
    gboolean do_quote = FALSE;
    char lc_prompt[80];
    int col;
    vfs_path_t *file_name_vpath;
    gboolean run_view = FALSE;

    /* Skip menu entry title line */
    commands = strchr (commands, '\n');
    if (commands == nullptr)
        return;

    cmd_file_fd = mc_mkstemps (&file_name_vpath, "mcusr", SCRIPT_SUFFIX);

    if (cmd_file_fd == -1)
    {
        message (D_ERROR, MSG_ERROR, _("Cannot create temporary command file\n%s"),
                 unix_error_string (errno));
        vfs_path_free (file_name_vpath);
        return;
    }

    cmd_file = fdopen (cmd_file_fd, "w");
    fputs ("#! /bin/sh\n", cmd_file);
    commands++;

    for (col = 0; *commands != '\0'; commands++)
    {
        if (col == 0)
        {
            if (!whitespace (*commands))
                break;
            while (whitespace (*commands))
                commands++;
            if (*commands == '\0')
                break;
        }
        col++;
        if (*commands == '\n')
            col = 0;
        if (parameter != nullptr)
        {
            if (*commands == '}')
            {
                *parameter = '\0';
                parameter =
                        input_dialog (_("Parameter"), lc_prompt, MC_HISTORY_FM_MENU_EXEC_PARAM, "",
                                      static_cast<input_complete_t>(INPUT_COMPLETE_FILENAMES | INPUT_COMPLETE_CD |
                                                                    INPUT_COMPLETE_HOSTNAMES | INPUT_COMPLETE_VARIABLES |
                                                                    INPUT_COMPLETE_USERNAMES));
                if (parameter == nullptr || *parameter == '\0')
                {
                    /* User canceled */
                    fclose (cmd_file);
                    mc_unlink (file_name_vpath);
                    vfs_path_free (file_name_vpath);
                    return;
                }
                if (do_quote)
                {
                    char *tmp;

                    tmp = name_quote (parameter, FALSE);
                    fputs (tmp, cmd_file);
                    g_free (tmp);
                }
                else
                    fputs (parameter, cmd_file);

                MC_PTR_FREE (parameter);
            }
            else if (parameter < lc_prompt + sizeof (lc_prompt) - 1)
                *parameter++ = *commands;
        }
        else if (expand_prefix_found)
        {
            expand_prefix_found = FALSE;
            if (g_ascii_isdigit ((gchar) * commands))
            {
                do_quote = (atoi (commands) != 0);
                while (g_ascii_isdigit ((gchar) * commands))
                    commands++;
            }
            if (*commands == '{')
                parameter = lc_prompt;
            else
            {
                char *text;

                text = ExpandFormat(edit_widget, *commands, do_quote);
                fputs (text, cmd_file);
                g_free (text);
            }
        }
        else if (*commands == '%')
        {
            int i;

            i = CheckFormatView(commands + 1);
            if (i != 0)
            {
                commands += i;
                run_view = TRUE;
            }
            else
            {
                do_quote = TRUE;        /* Default: Quote expanded macro */
                expand_prefix_found = TRUE;
            }
        }
        else
            fputc (*commands, cmd_file);
    }

    fclose (cmd_file);
    mc_chmod (file_name_vpath, S_IRWXU);

    if (run_view)
    {
        mcview_viewer (vfs_path_as_str (file_name_vpath), NULL, 0, 0, 0);
        dialog_switch_process_pending ();
    }
    else
    {
        /* execute the command indirectly to allow execution even
         * on no-exec filesystems. */
        char *cmd;

        cmd = g_strconcat ("/bin/sh ", vfs_path_as_str (file_name_vpath), (char *) NULL);

        if (show_prompt)
            shell_execute (cmd, EXECUTE_HIDE);
        else if (system (cmd) == -1)
            message (D_ERROR, MSG_ERROR, "%s", _("Error calling program"));

        g_free (cmd);
    }
    mc_unlink (file_name_vpath);
    vfs_path_free (file_name_vpath);
}

gboolean UserMenu::MenuFileOwn(char *path)
{
    struct stat st;

    if (stat (path, &st) == 0 && (st.st_uid == 0 || (st.st_uid == geteuid ()) != 0)
        && ((st.st_mode & (S_IWGRP | S_IWOTH)) == 0))
        return TRUE;

    if (verbose)
        message (D_NORMAL, _("Warning -- ignoring file"),
                 _("File %s is not owned by root or you or is world writable.\n"
                   "Using it may compromise your security"), path);

    return FALSE;
}
