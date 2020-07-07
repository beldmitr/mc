//
// Created by wbull on 5/24/20.
//

#pragma once

#include "src/filemanager/dir.hpp"        /* dir_list */

class DataSource
{
public:
    static void mcview_set_datasource_none(WView* view);

    static off_t mcview_get_filesize(WView* view);

    static void mcview_update_filesize(WView* view);

    static char* mcview_get_ptr_file(WView* view, off_t byte_index);

    static char* mcview_get_ptr_string(WView* view, off_t byte_index);

    /* Invalid UTF-8 is reported as negative integers (one for each byte),
    * see ticket 3783. */
    static gboolean mcview_get_utf(WView* view, off_t byte_index, int* ch, int* ch_len);

    static gboolean mcview_get_byte_string(WView* view, off_t byte_index, int* retval);

    static gboolean mcview_get_byte_none(WView* view, off_t byte_index, int* retval);

    static void mcview_set_byte(WView* view, off_t offset, byte b);

    static void mcview_file_load_data(WView* view, off_t byte_index);

    static void mcview_close_datasource(WView* view);

    static void mcview_set_datasource_file(WView* view, int fd, const struct stat* st);

    static gboolean mcview_load_command_output(WView* view, const char* command);

    static void mcview_set_datasource_vfs_pipe(WView* view, int fd);

    static void mcview_set_datasource_string(WView* view, const char* s);
private:
    static void mcview_set_datasource_stdio_pipe(WView* view, mc_pipe_t* p);
};
