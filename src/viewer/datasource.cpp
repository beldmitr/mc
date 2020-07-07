/*
   Internal file viewer for the Midnight Commander
   Functions for datasources

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
   Slava Zanko <slavazanko@google.com>, 2009
   Andrew Borodin <aborodin@vmail.ru>, 2009
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

/*
   The data source provides the viewer with data from either a file, a
   string or the output of a command. The mcview_get_byte() function can be
   used to get the value of a byte at a specific offset. If the offset
   is out of range, -1 is returned. The function mcview_get_byte_indexed(a,b)
   returns the byte at the offset a+b, or -1 if a+b is out of range.

   The mcview_set_byte() function has the effect that later calls to
   mcview_get_byte() will return the specified byte for this offset. This
   function is designed only for use by the hexedit component after
   saving its changes. Inspect the source before you want to use it for
   other purposes.

   The mcview_get_filesize() function returns the current size of the
   data source. If the growing buffer is used, this size may increase
   later on. Use the mcview_may_still_grow() function when you want to
   know if the size can change later.
 */

#include "lib/global.hpp"
#include "lib/vfs/vfs.hpp"
#include "lib/util.hpp"

#include "WView.hpp"
#include "growbuf.hpp"
#include "inlines.hpp"
#include "display.hpp"
#include "lib.hpp"
#include "datasource.hpp"

void DataSource::mcview_set_datasource_none(WView* view)
{
    view->datasource = WView::DS_NONE;
}

off_t DataSource::mcview_get_filesize(WView* view)
{
    switch (view->datasource)
    {
        case WView::DS_STDIO_PIPE:
        case WView::DS_VFS_PIPE:
            return Growbuf::mcview_growbuf_filesize (view);
        case WView::DS_FILE:
            return view->ds_file_filesize;
        case WView::DS_STRING:
            return view->ds_string_len;
        default:
            return 0;
    }
}

void DataSource::mcview_update_filesize(WView* view)
{
    if (view->datasource == WView::DS_FILE)
    {
        struct stat st;
        if (mc_fstat (view->ds_file_fd, &st) != -1)
            view->ds_file_filesize = st.st_size;
    }
}

char* DataSource::mcview_get_ptr_file(WView* view, off_t byte_index)
{
    g_assert (view->datasource == WView::DS_FILE);

    mcview_file_load_data (view, byte_index);
    if (Inlines::mcview_already_loaded (view->ds_file_offset, byte_index, view->ds_file_datalen))
        return (char *) (view->ds_file_data + (byte_index - view->ds_file_offset));
    return nullptr;
}

char* DataSource::mcview_get_ptr_string(WView* view, off_t byte_index)
{
    g_assert (view->datasource == WView::DS_STRING);

    if (byte_index >= 0 && byte_index < (off_t) view->ds_string_len)
        return (char *) (view->ds_string_data + byte_index);
    return nullptr;
}

gboolean DataSource::mcview_get_utf(WView* view, off_t byte_index, int* ch, int* ch_len)
{
    char* str = nullptr;
    int res;
    char utf8buf[UTF8_CHAR_LEN + 1];

    switch (view->datasource)
    {
        case WView::DS_STDIO_PIPE:
        case WView::DS_VFS_PIPE:
            str = Growbuf::mcview_get_ptr_growing_buffer (view, byte_index);
            break;
        case WView::DS_FILE:
            str = mcview_get_ptr_file (view, byte_index);
            break;
        case WView::DS_STRING:
            str = mcview_get_ptr_string (view, byte_index);
            break;
        case WView::DS_NONE:
        default:
            break;
    }

    *ch = 0;

    if (str == nullptr)
        return FALSE;

    res = g_utf8_get_char_validated (str, -1);

    if (res < 0)
    {
        /* Retry with explicit bytes to make sure it's not a buffer boundary */
        int i;

        for (i = 0; i < UTF8_CHAR_LEN; i++)
        {
            if (Inlines::mcview_get_byte (view, byte_index + i, &res))
                utf8buf[i] = res;
            else
            {
                utf8buf[i] = '\0';
                break;
            }
        }
        utf8buf[UTF8_CHAR_LEN] = '\0';
        str = utf8buf;
        res = g_utf8_get_char_validated (str, -1);
    }

    if (res < 0)
    {
        /* Implicit conversion from signed char to signed int keeps negative values. */
        *ch = *str;
        *ch_len = 1;
    }
    else
    {
        *ch = res;
        /* Calculate UTF-8 char length */
        char *next_ch = g_utf8_next_char (str);
        *ch_len = next_ch - str;
    }

    return TRUE;
}

gboolean DataSource::mcview_get_byte_string(WView* view, off_t byte_index, int* retval)
{
    if (retval != nullptr)
        *retval = -1;

    char *p = mcview_get_ptr_string (view, byte_index);
    if (p == nullptr)
        return FALSE;

    if (retval != nullptr)
        *retval = (unsigned char) (*p);
    return TRUE;
}

gboolean DataSource::mcview_get_byte_none(WView* view, off_t /* byte_index */, int* retval)
{
    g_assert (view->datasource == WView::DS_NONE);

    if (retval != nullptr)
        *retval = -1;
    return FALSE;
}

void DataSource::mcview_set_byte(WView* view, off_t offset, byte /* b */)
{
    g_assert (offset < mcview_get_filesize (view));
    g_assert (view->datasource == WView::DS_FILE);

    view->ds_file_datalen = 0;  /* just force reloading */
}

void DataSource::mcview_file_load_data(WView* view, off_t byte_index)
{
    ssize_t res;
    size_t bytes_read;

    g_assert (view->datasource == WView::DS_FILE);

    if (Inlines::mcview_already_loaded (view->ds_file_offset, byte_index, view->ds_file_datalen))
        return;

    if (byte_index >= view->ds_file_filesize)
        return;

    off_t blockoffset = Inlines::mcview_offset_rounddown (byte_index, view->ds_file_datasize);
    if (mc_lseek (view->ds_file_fd, blockoffset, SEEK_SET) == -1)
        goto error; // FIXME DB goto

    bytes_read = 0;
    while (bytes_read < view->ds_file_datasize)
    {
        res =
            mc_read (view->ds_file_fd, view->ds_file_data + bytes_read,
                     view->ds_file_datasize - bytes_read);
        if (res == -1)
            goto error; // FIXME DB goto
        if (res == 0)
            break;
        bytes_read += (size_t) res;
    }
    view->ds_file_offset = blockoffset;
    if ((off_t) bytes_read > view->ds_file_filesize - view->ds_file_offset)
    {
        /* the file has grown in the meantime -- stick to the old size */
        view->ds_file_datalen = view->ds_file_filesize - view->ds_file_offset;
    }
    else
    {
        view->ds_file_datalen = bytes_read;
    }
    return;

    error:
    view->ds_file_datalen = 0;
}

void DataSource::mcview_close_datasource(WView* view)
{
    switch (view->datasource)
    {
        case WView::DS_NONE:
            break;
        case WView::DS_STDIO_PIPE:
            if (view->ds_stdio_pipe != nullptr)
            {
                Growbuf::mcview_growbuf_done (view);
                Display::mcview_display (view);
            }
            Growbuf::mcview_growbuf_free (view);
            break;
        case WView::DS_VFS_PIPE:
            if (view->ds_vfs_pipe != -1)
                Growbuf::mcview_growbuf_done (view);
            Growbuf::mcview_growbuf_free (view);
            break;
        case WView::DS_FILE:
            (void) mc_close (view->ds_file_fd);
            view->ds_file_fd = -1;
            MC_PTR_FREE (view->ds_file_data);
            break;
        case WView::DS_STRING:
            MC_PTR_FREE (view->ds_string_data);
        default:
            break;
    }
    view->datasource = WView::DS_NONE;
}

void DataSource::mcview_set_datasource_file(WView* view, int fd, const struct stat* st)
{
    view->datasource = WView::DS_FILE;
    view->ds_file_fd = fd;
    view->ds_file_filesize = st->st_size;
    view->ds_file_offset = 0;
    view->ds_file_data = static_cast<byte*>(g_malloc (4096));
    view->ds_file_datalen = 0;
    view->ds_file_datasize = 4096;
}

gboolean DataSource::mcview_load_command_output(WView* view, const char* command)
{
    mcview_close_datasource (view);

    GError *error = nullptr;
    mc_pipe_t *p = mc_popen (command, &error);
    if (p == nullptr)
    {
        Display::mcview_display (view);
        Lib::mcview_show_error (view, error->message);
        g_error_free (error);
        return FALSE;
    }

    /* Check if filter produced any output */
    mcview_set_datasource_stdio_pipe (view, p);
    if (!Inlines::mcview_get_byte (view, 0, nullptr))
    {
        mcview_close_datasource (view);
        Display::mcview_display (view);
        return FALSE;
    }

    return TRUE;
}

void DataSource::mcview_set_datasource_vfs_pipe(WView* view, int fd)
{
    g_assert (fd != -1);

    view->datasource = WView::DS_VFS_PIPE;
    view->ds_vfs_pipe = fd;

    Growbuf::mcview_growbuf_init (view);
}

void DataSource::mcview_set_datasource_string(WView* view, const char* s)
{
    view->datasource = WView::DS_STRING;
    view->ds_string_len = strlen(s);
    view->ds_string_data = (byte*) g_strndup(s, view->ds_string_len);
}

void DataSource::mcview_set_datasource_stdio_pipe(WView* view, mc_pipe_t* p)
{
    p->out.len = MC_PIPE_BUFSIZE;
    p->out.null_term = FALSE;
    p->err.len = MC_PIPE_BUFSIZE;
    p->err.null_term = TRUE;
    view->datasource = WView::DS_STDIO_PIPE;
    view->ds_stdio_pipe = p;
    view->pipe_first_err_msg = TRUE;

    Growbuf::mcview_growbuf_init (view);
}