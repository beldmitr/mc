/*
   Internal file viewer for the Midnight Commander
   Common finctions (used from some other mcviewer functions)

   Copyright (C) 1994-2020
   Free Software Foundation, Inc.

   Written by:
   Miguel de Icaza, 1994, 1995, 1998
   Janne Kukonlehto, 1994, 1995
   Jakub Jelinek, 1995
   Joseph M. Hinkle, 1996
   Norbert Warmuth, 1997
   Pavel Machek, 1998
   Roland Illig <roland.illig@gmx.de>, 2004, 2005
   Slava Zanko <slavazanko@google.com>, 2009, 2013
   Andrew Borodin <aborodin@vmail.ru>, 2009, 2013, 2014
   Ilia Maslakov <il.smind@gmail.com>, 2009

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

#include <string.h>             /* memset() */
#include <sys/types.h>

#include "lib/global.hpp"
#include "lib/vfs/vfs.hpp"
#include "lib/strutil.hpp"
#include "lib/util.hpp"           /* save_file_position() */
#include "lib/widget.hpp"
#ifdef HAVE_CHARSET
#include "lib/charsets.hpp"
#endif

#ifdef HAVE_CHARSET
#include "src/selcodepage.hpp"
#endif

#include "internal.hpp"

void Lib::mcview_toggle_magic_mode(WView* view)
{
    char *filename, *command;
    dir_list *dir;
    int *dir_idx;

    McViewer::mcview_altered_flags.magic = TRUE;
    view->mode_flags.magic = !view->mode_flags.magic;

    /* reinit view */
    filename = g_strdup (vfs_path_as_str (view->filename_vpath));
    command = g_strdup (view->command);
    dir = view->dir;
    dir_idx = view->dir_idx;
    view->dir = nullptr;
    view->dir_idx = nullptr;
    mcview_done (view);
    mcview_init (view);
    McViewer::mcview_load (view, command, filename, 0, 0, 0);
    view->dir = dir;
    view->dir_idx = dir_idx;
    g_free (filename);
    g_free (command);

    view->dpy_bbar_dirty = TRUE;
    view->dirty++;
}

void Lib::mcview_toggle_wrap_mode(WView* view)
{
    view->mode_flags.wrap = !view->mode_flags.wrap;
    view->dpy_wrap_dirty = TRUE;
    view->dpy_bbar_dirty = TRUE;
    view->dirty++;
}

void Lib::mcview_toggle_nroff_mode(WView* view)
{
    view->mode_flags.nroff = !view->mode_flags.nroff;
    McViewer::mcview_altered_flags.nroff = TRUE;
    view->dpy_wrap_dirty = TRUE;
    view->dpy_bbar_dirty = TRUE;
    view->dirty++;
}

void Lib::mcview_toggle_hex_mode(WView* view)
{
    view->mode_flags.hex = !view->mode_flags.hex;

    if (view->mode_flags.hex)
    {
        view->hex_cursor = view->dpy_start;
        view->dpy_start = Inlines::mcview_offset_rounddown (view->dpy_start, view->bytes_per_line);
        widget_want_cursor (WIDGET (view), TRUE);
    }
    else
    {
        view->dpy_start = mcview_bol (view, view->hex_cursor, 0);
        view->hex_cursor = view->dpy_start;
        widget_want_cursor (WIDGET (view), FALSE);
    }
    McViewer::mcview_altered_flags.hex = TRUE;
    view->dpy_paragraph_skip_lines = 0;
    view->dpy_wrap_dirty = TRUE;
    view->dpy_bbar_dirty = TRUE;
    view->dirty++;
}

void Lib::mcview_init(WView* view)
{
    view->filename_vpath = nullptr;
    view->workdir_vpath = nullptr;
    view->command = nullptr;
    view->search_nroff_seq = nullptr;

    mcview_set_datasource_none (view);

    view->growbuf_in_use = FALSE;
    /* leave the other growbuf fields uninitialized */

    view->hexedit_lownibble = FALSE;
    view->locked = FALSE;
    view->coord_cache = nullptr;

    view->dpy_start = 0;
    view->dpy_paragraph_skip_lines = 0;
    mcview_state_machine_init (&view->dpy_state_top, 0);
    view->dpy_wrap_dirty = FALSE;
    view->force_max = -1;
    view->dpy_text_column = 0;
    view->dpy_end = 0;
    view->hex_cursor = 0;
    view->cursor_col = 0;
    view->cursor_row = 0;
    view->change_list = nullptr;

    /* {status,ruler,data}_area are left uninitialized */

    view->dirty = 0;
    view->dpy_bbar_dirty = TRUE;
    view->bytes_per_line = 1;

    view->search_start = 0;
    view->search_end = 0;

    view->marker = 0;
    for (size_t i = 0; i < G_N_ELEMENTS (view->marks); i++)
        view->marks[i] = 0;

    view->update_steps = 0;
    view->update_activate = 0;

    view->saved_bookmarks = nullptr;
}

void Lib::mcview_done(WView* view)
{
    /* Save current file position */
    if (McViewer::mcview_remember_file_position && view->filename_vpath != nullptr)
    {
        save_file_position (view->filename_vpath, -1, 0,
                            view->mode_flags.hex ? view->hex_cursor : view->dpy_start,
                            view->saved_bookmarks);
        view->saved_bookmarks = nullptr;
    }

    /* Write back the global viewer mode */
    McViewer::mcview_global_flags = view->mode_flags;

    /* Free memory used by the viewer */
    /* view->widget needs no destructor */
    vfs_path_free (view->filename_vpath);
    view->filename_vpath = nullptr;
    vfs_path_free (view->workdir_vpath);
    view->workdir_vpath = NULL;
    MC_PTR_FREE (view->command);

    mcview_close_datasource (view);
    /* the growing buffer is freed with the datasource */

    coord_cache_free (view->coord_cache);
    view->coord_cache = nullptr;

    if (view->converter == INVALID_CONV)
        view->converter = str_cnv_from_term;

    if (view->converter != str_cnv_from_term)
    {
        str_close_conv (view->converter);
        view->converter = str_cnv_from_term;
    }

    mc_search_free (view->search);
    view->search = nullptr;
    MC_PTR_FREE (view->last_search_string);
    Nroff::mcview_nroff_seq_free (&view->search_nroff_seq);
    mcview_hexedit_free_change_list (view);

    if (mc_global.mc_run_mode == MC_RUN_VIEWER && view->dir != nullptr)
    {
        /* mcviewer is the owner of file list */
        dir_list_free_list (view->dir);
        g_free (view->dir);
        g_free (view->dir_idx);
    }

    view->dir = nullptr;
}

#ifdef HAVE_CHARSET
void Lib::mcview_select_encoding(WView* view)
{
    if (SelCodePage::do_select_codepage())
        mcview_set_codeset(view);
}

void Lib::mcview_set_codeset(WView* view)
{
    view->utf8 = TRUE;
    const char *cp_id = get_codepage_id(mc_global.source_codepage >= 0 ? mc_global.source_codepage : mc_global.display_codepage);
    if (cp_id != nullptr)
    {
        GIConv conv = str_crt_conv_from (cp_id);
        if (conv != INVALID_CONV)
        {
            if (view->converter != str_cnv_from_term)
                str_close_conv (view->converter);
            view->converter = conv;
        }
        view->utf8 = (gboolean) str_isutf8 (cp_id);
        view->dpy_wrap_dirty = TRUE;
    }
}
#endif /* HAVE_CHARSET */

void Lib::mcview_show_error(WView* view, const char* msg)
{
    if (Inlines::mcview_is_in_panel (view))
        mcview_set_datasource_string (view, msg);
    else
        message (D_ERROR, MSG_ERROR, "%s", msg);
}

off_t Lib::mcview_bol(WView* view, off_t current, off_t limit)
{
    off_t filesize = mcview_get_filesize (view);
    if (current <= 0)
        return 0;

    if (current > filesize)
        return filesize;

    int c;
    if (!Inlines::mcview_get_byte (view, current, &c))
        return current;

    if (c == '\n')
    {
        if (!Inlines::mcview_get_byte (view, current - 1, &c))
            return current;
        if (c == '\r')
            current--;
    }
    while (current > 0 && current > limit)
    {
        if (!Inlines::mcview_get_byte (view, current - 1, &c))
            break;
        if (c == '\r' || c == '\n')
            break;
        current--;
    }
    return current;
}

off_t Lib::mcview_eol(WView* view, off_t current)
{
    int c, prev_ch = 0;

    if (current < 0)
        return 0;

    while (TRUE)
    {
        if (!Inlines::mcview_get_byte (view, current, &c))
            break;
        if (c == '\n')
        {
            current++;
            break;
        }
        else if (prev_ch == '\r')
        {
            break;
        }
        current++;
        prev_ch = c;
    }
    return current;
}

char* Lib::mcview_get_title(const WDialog* h, size_t len)
{
    const auto* view = (const WView *) widget_find_by_type(CONST_WIDGET (h), mcview_callback); // FIXME DB c++ cast
    const char *modified = view->hexedit_mode && (view->change_list != nullptr) ? "(*) " : "    ";
    const char *view_filename = vfs_path_as_str (view->filename_vpath);

    len -= 4;

    const char *file_label;
    // TODO DB I don't really undestand 2 lines below, so you assign something to file_label and after immediately reassign it
    file_label = view_filename != nullptr ? view_filename : view->command != nullptr ? view->command : "";
    file_label = str_term_trim (file_label, len - str_term_width1 (_("View: ")));

    char *ret_str = g_strconcat (_("View: "), modified, file_label, (char *) nullptr);
    return ret_str;
}

int Lib::mcview_calc_percent(WView* view, off_t p)
{
    const screen_dimen right = view->status_area.left + view->status_area.width;
    const screen_dimen height = view->status_area.height;

    if (height < 1 || right < 4)
        return (-1);
    if (Inlines::mcview_may_still_grow (view))
        return (-1);

    off_t filesize = mcview_get_filesize (view);
    if (view->mode_flags.hex && filesize > 0)
    {
        /* p can't be beyond the last char, only over that. Compensate for this. */
        filesize--;
    }

    int percent;
    if (filesize == 0 || p >= filesize)
        percent = 100;
    else if (p > (INT_MAX / 100))
        percent = p / (filesize / 100);
    else
        percent = p * 100 / filesize;

    return percent;
}