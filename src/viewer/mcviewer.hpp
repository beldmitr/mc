/** \file mcviewer.h
 *  \brief Header: internal file viewer
 */

#pragma once

#include "lib/widget.hpp"

class WView;

class McViewer
{
public:
    struct mcview_mode_flags_t
    {
        gboolean wrap;              /* Wrap text lines to fit them on the screen */
        gboolean hex;               /* Plainview or hexview */
        gboolean magic;             /* Preprocess the file using external programs */
        gboolean nroff;             /* Nroff-style highlighting */
    };

public:
    static mcview_mode_flags_t mcview_global_flags;
    static mcview_mode_flags_t mcview_altered_flags;

    static gboolean mcview_remember_file_position;
    static int mcview_max_dirt_limit;

    static gboolean mcview_mouse_move_pages;
    static char* mcview_show_eof;

public:
    /* Creates a new WView object with the given properties. Caveat: the
     * origin is in y-x order, while the extent is in x-y order. */
    static WView* mcview_new(int y, int x, int lines, int cols, gboolean is_panel);

    /* Shows {file} or the output of {command} in the internal viewer,
     * starting in line {start_line}. */
    static gboolean mcview_viewer(const char* command, const vfs_path_t* file_vpath, int start_line, off_t search_start, off_t search_end);

    static gboolean mcview_load(WView* view, const char* command, const char* file, int start_line, off_t search_start, off_t search_end);
    static void mcview_clear_mode_flags(mcview_mode_flags_t* flags);
private:
    static void mcview_mouse_callback(Widget* w, mouse_msg_t msg, mouse_event_t* event);
};
