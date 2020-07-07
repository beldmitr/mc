/*
   Internal file viewer for the Midnight Commander
   Interface functions

   Copyright (C) 1994-2020
   Free Software Foundation, Inc

   Written by:
   Miguel de Icaza, 1994, 1995, 1998
   Janne Kukonlehto, 1994, 1995
   Jakub Jelinek, 1995
   Joseph M. Hinkle, 1996
   Norbert Warmuth, 1997
   Pavel Machek, 1998
   Roland Illig <roland.illig@gmx.de>, 2004, 2005
   Slava Zanko <slavazanko@google.com>, 2009, 2013
   Andrew Borodin <aborodin@vmail.ru>, 2009, 2013
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

#include <errno.h>

#include "lib/global.hpp"
#include "lib/tty/tty.hpp"
#include "lib/vfs/vfs.hpp"
#include "lib/strutil.hpp"
#include "lib/util.hpp"           /* load_file_position() */
#include "lib/widget.hpp"

#include "src/filemanager/layout.hpp"
#include "src/filemanager/midnight.hpp"   /* the_menubar */
#include "src/keybind-defaults.hpp"       /* global_keymap_t */

#include "actions_cmd.hpp"
#include "move.hpp"
#include "display.hpp"
#include "inlines.hpp"
#include "lib.hpp"
#include "mcviewer.hpp"


McViewer::mcview_mode_flags_t McViewer::mcview_global_flags = {
    .wrap = TRUE,
    .hex = FALSE,
    .magic = TRUE,
    .nroff = FALSE
};

McViewer::mcview_mode_flags_t McViewer::mcview_altered_flags = {
    .wrap = FALSE,
    .hex = FALSE,
    .magic = FALSE,
    .nroff = FALSE
};

gboolean McViewer::mcview_remember_file_position = FALSE;

/* Maxlimit for skipping updates */
int McViewer::mcview_max_dirt_limit = 10;

/* Scrolling is done in pages or line increments */
gboolean McViewer::mcview_mouse_move_pages = TRUE;

/* end of file will be showen from mcview_show_eof */
char* McViewer::mcview_show_eof = nullptr;

WView* McViewer::mcview_new(int y, int x, int lines, int cols, gboolean is_panel)
{
    WView *view = g_new0(WView, 1);
    Widget *w = WIDGET(view);
    widget_init(w, y, x, lines, cols, ActionsCmd::mcview_callback, mcview_mouse_callback);
    w->options = static_cast<widget_options_t>(w->options | WOP_SELECTABLE | WOP_TOP_SELECT);
    w->keymap = KeyBindDefaults::viewer_map;

    mcview_clear_mode_flags(&view->mode_flags);
    view->hexedit_mode = FALSE;
    view->hex_keymap = KeyBindDefaults::viewer_hex_map;
    view->hexview_in_text = FALSE;
    view->locked = FALSE;

    view->dpy_frame_size = is_panel ? 1 : 0;
    view->converter = str_cnv_from_term;

    Lib::mcview_init(view);

    if (McViewer::mcview_global_flags.hex)
        Lib::mcview_toggle_hex_mode(view);
    if (McViewer::mcview_global_flags.nroff)
        Lib::mcview_toggle_nroff_mode(view);
    if (McViewer::mcview_global_flags.wrap)
        Lib::mcview_toggle_wrap_mode(view);
    if (McViewer::mcview_global_flags.magic)
        Lib::mcview_toggle_magic_mode(view);

    return view;
}

/** Real view only */
gboolean McViewer::mcview_viewer(const char* command, const vfs_path_t* file_vpath, int start_line, off_t search_start, off_t search_end)
{
    /* Create dialog and widgets, put them on the dialog */
    WDialog* view_dlg = dlg_create(FALSE, 0, 0, 1, 1, WPOS_FULLSCREEN, FALSE, nullptr, ActionsCmd::mcview_dialog_callback, nullptr, "[Internal File Viewer]", nullptr);
    Widget* vw = WIDGET(view_dlg);
    widget_want_tab(vw, TRUE);

    WGroup* g = GROUP(view_dlg);

    WView* lc_mcview = mcview_new(vw->y, vw->x, vw->lines - 1, vw->cols, FALSE);
    group_add_widget_autopos(g, lc_mcview, WPOS_KEEP_ALL, nullptr);

    Widget* b = WIDGET(buttonbar_new (TRUE));
    group_add_widget_autopos(g, b, b->pos_flags, nullptr);

    view_dlg->get_title = Lib::mcview_get_title;

    gboolean succeeded = mcview_load(lc_mcview, command, vfs_path_as_str (file_vpath), start_line, search_start, search_end);

    if (succeeded)
        dlg_run(view_dlg);
    else
        dlg_stop(view_dlg);

    if (widget_get_state(vw, WST_CLOSED))
        dlg_destroy(view_dlg);

    return succeeded;
}

gboolean McViewer::mcview_load(WView* view, const char* command, const char* file, int start_line, off_t search_start, off_t search_end)
{
    gboolean retval = FALSE;
    vfs_path_t* vpath = nullptr;

    g_assert(view->bytes_per_line != 0);

    view->filename_vpath = vfs_path_from_str(file);

    /* get working dir */
    if (file != nullptr && file[0] != '\0')
    {
        vfs_path_free(view->workdir_vpath);

        if (!g_path_is_absolute(file))
        {
            vfs_path_t* p = vfs_path_clone(vfs_get_raw_current_dir ());
            view->workdir_vpath = vfs_path_append_new(p, file, (char *)nullptr);
            vfs_path_free(p);
        }
        else
        {
            /* try extract path from filename */
            const char *fname = x_basename(file);
            char *dir = g_strndup (file, (size_t) (fname - file));
            view->workdir_vpath = vfs_path_from_str(dir);
            g_free(dir);
        }
    }

    if (!Inlines::mcview_is_in_panel(view))
        view->dpy_text_column = 0;

#ifdef HAVE_CHARSET
    Lib::mcview_set_codeset(view);
#endif

    if (command != nullptr && (view->mode_flags.magic || file == nullptr || file[0] == '\0'))
        retval = DataSource::mcview_load_command_output (view, command);
    else if (file != nullptr && file[0] != '\0')
    {
        int fd;
        char tmp[BUF_MEDIUM];
        struct stat st;

        /* Open the file */
        vpath = vfs_path_from_str (file);
        fd = mc_open (vpath, O_RDONLY | O_NONBLOCK);
        if (fd == -1)
        {
            g_snprintf (tmp, sizeof (tmp), _("Cannot open \"%s\"\n%s"),
                        file, unix_error_string (errno));
            DataSource::mcview_close_datasource (view);
            Lib::mcview_show_error (view, tmp);
            vfs_path_free (view->filename_vpath);
            view->filename_vpath = nullptr;
            vfs_path_free (view->workdir_vpath);
            view->workdir_vpath = nullptr;
            goto finish;
        }

        /* Make sure we are working with a regular file */
        if (mc_fstat (fd, &st) == -1)
        {
            mc_close (fd);
            g_snprintf (tmp, sizeof (tmp), _("Cannot stat \"%s\"\n%s"),
                        file, unix_error_string (errno));
            DataSource::mcview_close_datasource (view);
            Lib::mcview_show_error (view, tmp);
            vfs_path_free (view->filename_vpath);
            view->filename_vpath = nullptr;
            vfs_path_free (view->workdir_vpath);
            view->workdir_vpath = nullptr;
            goto finish;
        }

        if (!S_ISREG (st.st_mode))
        {
            mc_close (fd);
            DataSource::mcview_close_datasource (view);
            Lib::mcview_show_error (view, _("Cannot view: not a regular file"));
            vfs_path_free (view->filename_vpath);
            view->filename_vpath = nullptr;
            vfs_path_free (view->workdir_vpath);
            view->workdir_vpath = nullptr;
            goto finish;
        }

        if (st.st_size == 0 || mc_lseek (fd, 0, SEEK_SET) == -1)
        {
            /* Must be one of those nice files that grow (/proc) */
            DataSource::mcview_set_datasource_vfs_pipe (view, fd);
        }
        else
        {
            if (view->mode_flags.magic)
            {
                int type;

                type = get_compression_type (fd, file);

                if (type != COMPRESSION_NONE)
                {
                    char *tmp_filename;
                    vfs_path_t *vpath1;
                    int fd1;

                    tmp_filename = g_strconcat (file, decompress_extension (type), (char *) nullptr);
                    vpath1 = vfs_path_from_str (tmp_filename);
                    g_free (tmp_filename);
                    fd1 = mc_open (vpath1, O_RDONLY | O_NONBLOCK);
                    vfs_path_free (vpath1);

                    if (fd1 == -1)
                    {
                        g_snprintf (tmp, sizeof (tmp), _("Cannot open \"%s\" in parse mode\n%s"),
                                    file, unix_error_string (errno));
                        DataSource::mcview_close_datasource (view);
                        Lib::mcview_show_error (view, tmp);
                    }
                    else
                    {
                        mc_close (fd);
                        fd = fd1;
                        mc_fstat (fd, &st);
                    }
                }
            }

            DataSource::mcview_set_datasource_file (view, fd, &st);
        }
        retval = TRUE;
    }

    finish:
    view->command = g_strdup (command);
    view->dpy_start = 0;
    view->dpy_paragraph_skip_lines = 0;
    Ascii::mcview_state_machine_init (&view->dpy_state_top, 0);
    view->dpy_wrap_dirty = FALSE;
    view->force_max = -1;
    view->dpy_text_column = 0;

    Display::mcview_compute_areas (view);
    Display::mcview_update_bytes_per_line (view);

    if (McViewer::mcview_remember_file_position && view->filename_vpath != nullptr && start_line == 0)
    {
        long line;
        long col;
        off_t new_offset;

        load_file_position(view->filename_vpath, &line, &col, &new_offset, &view->saved_bookmarks);
        off_t max_offset = DataSource::mcview_get_filesize(view) - 1;
        if (max_offset < 0)
            new_offset = 0;
        else
            new_offset = MIN (new_offset, max_offset);

        if (!view->mode_flags.hex)
        {
            view->dpy_start = Lib::mcview_bol (view, new_offset, 0);
            view->dpy_wrap_dirty = TRUE;
        }
        else
        {
            view->dpy_start = new_offset - new_offset % view->bytes_per_line;
            view->hex_cursor = new_offset;
        }
    }
    else if (start_line > 0)
        Move::mcview_moveto (view, start_line - 1, 0);

    view->search_start = search_start;
    view->search_end = search_end;
    view->hexedit_lownibble = FALSE;
    view->hexview_in_text = FALSE;
    view->change_list = nullptr;
    vfs_path_free (vpath);
    return retval;
}

void McViewer::mcview_clear_mode_flags (mcview_mode_flags_t* flags)
{
    memset(flags, 0, sizeof(*flags));
}

void McViewer::mcview_mouse_callback (Widget* w, mouse_msg_t msg, mouse_event_t* event)
{
    auto* view = reinterpret_cast<WView*>(w);
    gboolean ok = TRUE;

    switch (msg)
    {
        case MSG_MOUSE_DOWN:
            if (Inlines::mcview_is_in_panel (view))
            {
                if (event->y == WIDGET (w->owner)->y)
                {
                    /* return MOU_UNHANDLED */
                    event->result.abort = TRUE;
                    /* don't draw viewer over menu */
                    ok = FALSE;
                    break;
                }

                if (!widget_get_state (w, WST_FOCUSED))
                {
                    /* Grab focus */
                    change_panel ();
                }
            }
            MC_FALLTHROUGH;

        case MSG_MOUSE_CLICK:
            if (!view->mode_flags.wrap)
            {
                /* Scrolling left and right */
                screen_dimen x;

                x = event->x + 1;   /* FIXME */

                if (x < view->data_area.width * 1 / 4)
                {
                    Move::mcview_move_left (view, 1);
                    event->result.repeat = msg == MSG_MOUSE_DOWN;
                }
                else if (x < view->data_area.width * 3 / 4)
                {
                    /* ignore the click */
                    ok = FALSE;
                }
                else
                {
                    Move::mcview_move_right (view, 1);
                    event->result.repeat = msg == MSG_MOUSE_DOWN;
                }
            }
            else
            {
                /* Scrolling up and down */
                screen_dimen y;

                y = event->y + 1;   /* FIXME */

                if (y < view->data_area.top + view->data_area.height * 1 / 3)
                {
                    if (McViewer::mcview_mouse_move_pages)
                        Move::mcview_move_up (view, view->data_area.height / 2);
                    else
                        Move::mcview_move_up (view, 1);

                    event->result.repeat = msg == MSG_MOUSE_DOWN;
                }
                else if (y < view->data_area.top + view->data_area.height * 2 / 3)
                {
                    /* ignore the click */
                    ok = FALSE;
                }
                else
                {
                    if (McViewer::mcview_mouse_move_pages)
                        Move::mcview_move_down (view, view->data_area.height / 2);
                    else
                        Move::mcview_move_down (view, 1);

                    event->result.repeat = msg == MSG_MOUSE_DOWN;
                }
            }
            break;

        case MSG_MOUSE_SCROLL_UP:
            Move::mcview_move_up (view, 2);
            break;

        case MSG_MOUSE_SCROLL_DOWN:
            Move::mcview_move_down (view, 2);
            break;

        default:
            ok = FALSE;
            break;
    }

    if (ok)
        Display::mcview_update (view);
}
