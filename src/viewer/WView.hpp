//
// Created by wbull on 5/24/20.
//

#pragma once

#include "mcviewer.hpp"
#include "ascii.hpp"
#include "coord_cache.hpp"
#include "src/filemanager/dir.hpp"        /* dir_list */
#include "hex.hpp"

#include "search.hpp" // FIXME DB
#include "nroff.hpp"

class WView
{
public:
    struct area
    {
        screen_dimen top, left;
        screen_dimen height, width;
    };

    /* data sources of the view */
    enum view_ds
    {
        DS_NONE,                    /* No data available */
        DS_STDIO_PIPE,              /* Data comes from a pipe using popen/pclose */
        DS_VFS_PIPE,                /* Data comes from a piped-in VFS file */
        DS_FILE,                    /* Data comes from a VFS file */
        DS_STRING                   /* Data comes from a string in memory */
    };
public:
    Widget widget;

    vfs_path_t *filename_vpath; /* Name of the file */
    vfs_path_t *workdir_vpath;  /* Name of the working directory */
    char *command;              /* Command used to pipe data in */

    view_ds datasource;    /* Where the displayed data comes from */

    /* stdio pipe data source */
    mc_pipe_t *ds_stdio_pipe;   /* Output of a shell command */
    gboolean pipe_first_err_msg;        /* Show only 1st message from stderr */

    /* vfs pipe data source */
    int ds_vfs_pipe;            /* Non-seekable vfs file descriptor */

    /* vfs file data source */
    int ds_file_fd;             /* File with random access */
    off_t ds_file_filesize;     /* Size of the file */
    off_t ds_file_offset;       /* Offset of the currently loaded data */
    byte *ds_file_data;         /* Currently loaded data */
    size_t ds_file_datalen;     /* Number of valid bytes in file_data */
    size_t ds_file_datasize;    /* Number of allocated bytes in file_data */

    /* string data source */
    byte *ds_string_data;       /* The characters of the string */
    size_t ds_string_len;       /* The length of the string */

    /* Growing buffers information */
    gboolean growbuf_in_use;    /* Use the growing buffers? */
    GPtrArray *growbuf_blockptr;        /* Pointer to the block pointers */
    size_t growbuf_lastindex;   /* Number of bytes in the last page of the
                                   growing buffer */
    gboolean growbuf_finished;  /* TRUE when all data has been read. */

    McViewer::mcview_mode_flags_t mode_flags;

    /* Hex editor modes */
    gboolean hexedit_mode;      /* Hexview or Hexedit */
    const global_keymap_t *hex_keymap;
    gboolean hexview_in_text;   /* Is the hexview cursor in the text area? */
    int bytes_per_line;         /* Number of bytes per line in hex mode */
    off_t hex_cursor;           /* Hexview cursor position in file */
    gboolean hexedit_lownibble; /* Are we editing the last significant nibble? */
    gboolean locked;            /* We hold lock on current file */

#ifdef HAVE_CHARSET
    gboolean utf8;              /* It's multibyte file codeset */
#endif

    CoordCache::coord_cache_t *coord_cache; /* Cache for mapping offsets to cursor positions */

    /* Display information */
    screen_dimen dpy_frame_size;        /* Size of the frame surrounding the real viewer */
    off_t dpy_start;            /* Offset of the displayed data (start of the paragraph in non-hex mode) */
    off_t dpy_end;              /* Offset after the displayed data */
    off_t dpy_paragraph_skip_lines;     /* Extra lines to skip in wrap mode */
    Ascii::mcview_state_machine_t dpy_state_top;       /* Parser-formatter state at the topmost visible line in wrap mode */
    Ascii::mcview_state_machine_t dpy_state_bottom;    /* Parser-formatter state after the bottomvisible line in wrap mode */
    gboolean dpy_wrap_dirty;    /* dpy_state_top needs to be recomputed */
    off_t dpy_text_column;      /* Number of skipped columns in non-wrap
                                 * text mode */
    screen_dimen cursor_col;    /* Cursor column */
    screen_dimen cursor_row;    /* Cursor row */
    Hex::hexedit_change_node *change_list;    /* Linked list of changes */
    struct area status_area;    /* Where the status line is displayed */
    struct area ruler_area;     /* Where the ruler is displayed */
    struct area data_area;      /* Where the data is displayed */

    ssize_t force_max;          /* Force a max offset, or -1 */

    int dirty;                  /* Number of skipped updates */
    gboolean dpy_bbar_dirty;    /* Does the button bar need to be updated? */


    /* handle of search engine */
    mc_search_t *search;
    gchar *last_search_string;
    Nroff::mcview_nroff_t *search_nroff_seq;
    off_t search_start;         /* First character to start searching from */
    off_t search_end;           /* Length of found string or 0 if none was found */
    int search_numNeedSkipChar;

    /* Markers */
    int marker;                 /* mark to use */
    off_t marks[10];            /* 10 marks: 0..9 */

    off_t update_steps;         /* The number of bytes between percent increments */
    off_t update_activate;      /* Last point where we updated the status */

    /* converter for translation of text */
    GIConv converter;

    GArray *saved_bookmarks;

    dir_list *dir;              /* List of current directory files
                                 * to handle CK_FileNext and CK_FilePrev commands */
    int *dir_idx;               /* Index of current file in dir structure.
                                 * Pointer is used here as reference to WPanel::dir::count */
    vfs_path_t *ext_script;     /* Temporary script file created by regex_command_for() */
};
