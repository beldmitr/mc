/*
   Util for external clipboard.

   Copyright (C) 2009-2020
   Free Software Foundation, Inc.

   Written by:
   Ilia Maslakov <il.smind@gmail.com>, 2010.
   Andrew Borodin <aborodin@vmail.ru>, 2014.

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

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include "lib/global.hpp"
#include "lib/fileloc.hpp"
#include "lib/mcconfig.hpp"
#include "lib/util.hpp"
#include "lib/event.hpp"

#include "lib/vfs/vfs.hpp"

#include "src/execute.hpp"

#include "clipboard.hpp"

gboolean Clipboard::clipboard_file_to_ext_clip(const char*, const char*, void*, void*)
{


    const char *d = getenv ("DISPLAY");


    if (d == nullptr || clipboard_store_path == nullptr || clipboard_store_path[0] == '\0')
        return TRUE;

    char *tmp = mc_config_get_full_path (EDIT_CLIP_FILE);
    char *cmd = g_strconcat (clipboard_store_path, " ", tmp, " 2>/dev/null", (char *) NULL);

    if (cmd != nullptr)
        my_system (EXECUTE_AS_SHELL, mc_global.shell->path, cmd);

    g_free (cmd);
    g_free (tmp);
    return TRUE;
}

gboolean Clipboard::clipboard_file_from_ext_clip(const char*, const char*, void*, void*)
{
    const char *d = getenv ("DISPLAY");

    if (d == nullptr || clipboard_paste_path == nullptr || clipboard_paste_path[0] == '\0')
        return TRUE;

    mc_pipe_t *p = mc_popen(clipboard_paste_path, nullptr);
    if (p == nullptr)
        return TRUE;            /* don't show error message */

    p->out.null_term = FALSE;
    p->err.null_term = TRUE;

    int file = -1;
    while (TRUE)
    {
        p->out.len = MC_PIPE_BUFSIZE;
        p->err.len = MC_PIPE_BUFSIZE;

        GError *error = nullptr;
        mc_pread(p, &error);

        if (error != nullptr)
        {
            /* don't show error message */
            g_error_free(error);
            break;
        }

        /* ignore stderr and get stdout */
        if (p->out.len == MC_PIPE_STREAM_EOF || p->out.len == MC_PIPE_ERROR_READ)
            break;

        if (p->out.len > 0)
        {
            if (file < 0)
            {
                vfs_path_t *fname_vpath = mc_config_get_full_vpath (EDIT_CLIP_FILE);
                file = mc_open(fname_vpath, clip_open_flags, clip_open_mode);
                vfs_path_free(fname_vpath);

                if (file < 0)
                    break;
            }

            mc_write(file, p->out.buf, p->out.len);
        }
    }

    if (file >= 0)
        mc_close(file);

    mc_pclose(p, nullptr);

    return TRUE;
}

gboolean Clipboard::clipboard_text_to_file(const char*, const char*, void*, void* data)
{
    const char *text = static_cast<const char*>(data);

    if (text == nullptr)
        return FALSE;

    vfs_path_t *fname_vpath = mc_config_get_full_vpath(EDIT_CLIP_FILE);
    int file = mc_open(fname_vpath, clip_open_flags, clip_open_mode);
    vfs_path_free(fname_vpath);

    if (file == -1)
        return TRUE;

    size_t str_len = strlen(text);
    mc_write(file, text, str_len);

    mc_close(file);
    return TRUE;
}

gboolean Clipboard::clipboard_text_from_file(const char*, const char*, void*, void* data)
{
    char buf[BUF_LARGE];
    gboolean first = TRUE;
    auto* event_data = static_cast<ev_clipboard_text_from_file_t*>(data);

    char* fname = mc_config_get_full_path(EDIT_CLIP_FILE);
    FILE* f = fopen(fname, "r");
    g_free(fname);

    if (f == nullptr)
    {
        event_data->ret = FALSE;
        return TRUE;
    }

    *(event_data->text) = nullptr;

    while (fgets(buf, sizeof(buf), f))
    {
        size_t len;

        len = strlen(buf);
        if (len > 0)
        {
            if (buf[len - 1] == '\n')
                buf[len - 1] = '\0';

            if (first)
            {
                first = FALSE;
                *(event_data->text) = g_strdup(buf);
            }
            else
            {
                /* remove \n on EOL */
                char *tmp;

                tmp = g_strconcat(*(event_data->text), " ", buf, (char *) nullptr);
                g_free(*(event_data->text));
                *(event_data->text) = tmp;
            }
        }
    }

    fclose (f);
    event_data->ret = (*(event_data->text) != nullptr);
    return TRUE;
}