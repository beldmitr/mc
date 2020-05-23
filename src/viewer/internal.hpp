#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>

#include "lib/global.hpp"

#include "lib/search.hpp"
#include "lib/widget.hpp"
#include "lib/vfs/vfs.hpp"        /* vfs_path_t */

#include "src/keybind-defaults.hpp"       /* global_keymap_t */
#include "src/filemanager/dir.hpp"        /* dir_list */

#include "mcviewer.hpp"

/*** typedefs(not structures) and defined constants **********************************************/

typedef unsigned char byte;

/* A width or height on the screen */
typedef unsigned int screen_dimen;

/*** enums ***************************************************************************************/

/* data sources of the view */
enum view_ds
{
    DS_NONE,                    /* No data available */
    DS_STDIO_PIPE,              /* Data comes from a pipe using popen/pclose */
    DS_VFS_PIPE,                /* Data comes from a piped-in VFS file */
    DS_FILE,                    /* Data comes from a VFS file */
    DS_STRING                   /* Data comes from a string in memory */
};



typedef enum
{
    NROFF_TYPE_NONE = 0,
    NROFF_TYPE_BOLD = 1,
    NROFF_TYPE_UNDERLINE = 2
} nroff_type_t;

/*** structures declarations (and typedefs of structures)*****************************************/

/* A node for building a change list on change_list */
struct hexedit_change_node
{
    struct hexedit_change_node *next;
    off_t offset;
    byte value;
};

struct area
{
    screen_dimen top, left;
    screen_dimen height, width;
};

/* A cache entry for mapping offsets into line/column pairs and vice versa.
 * cc_offset, cc_line, and cc_column are the 0-based values of the offset,
 * line and column of that cache entry. cc_nroff_column is the column
 * corresponding to cc_offset in nroff mode.
 */
typedef struct
{
    off_t cc_offset;
    off_t cc_line;
    off_t cc_column;
    off_t cc_nroff_column;
} coord_cache_entry_t;

typedef struct
{
    size_t size;
    size_t capacity;
    coord_cache_entry_t **cache;
} coord_cache_t;

/* TODO: find a better name. This is not actually a "state machine",
 * but a "state machine's state", but that sounds silly.
 * Could be parser_state, formatter_state... */
typedef struct
{
    off_t offset;               /* The file offset at which this is the state. */
    off_t unwrapped_column;     /* Columns if the paragraph wasn't wrapped, */
    /* used for positioning TABs in wrapped lines */
    gboolean nroff_underscore_is_underlined;    /* whether _\b_ is underlined rather than bold */
    gboolean print_lonely_combining;    /* whether lonely combining marks are printed on a dotted circle */
} mcview_state_machine_t;

struct mcview_nroff_struct;

struct WView
{
    Widget widget;

    vfs_path_t *filename_vpath; /* Name of the file */
    vfs_path_t *workdir_vpath;  /* Name of the working directory */
    char *command;              /* Command used to pipe data in */

    enum view_ds datasource;    /* Where the displayed data comes from */

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

    coord_cache_t *coord_cache; /* Cache for mapping offsets to cursor positions */

    /* Display information */
    screen_dimen dpy_frame_size;        /* Size of the frame surrounding the real viewer */
    off_t dpy_start;            /* Offset of the displayed data (start of the paragraph in non-hex mode) */
    off_t dpy_end;              /* Offset after the displayed data */
    off_t dpy_paragraph_skip_lines;     /* Extra lines to skip in wrap mode */
    mcview_state_machine_t dpy_state_top;       /* Parser-formatter state at the topmost visible line in wrap mode */
    mcview_state_machine_t dpy_state_bottom;    /* Parser-formatter state after the bottomvisible line in wrap mode */
    gboolean dpy_wrap_dirty;    /* dpy_state_top needs to be recomputed */
    off_t dpy_text_column;      /* Number of skipped columns in non-wrap
                                 * text mode */
    screen_dimen cursor_col;    /* Cursor column */
    screen_dimen cursor_row;    /* Cursor row */
    struct hexedit_change_node *change_list;    /* Linked list of changes */
    struct area status_area;    /* Where the status line is displayed */
    struct area ruler_area;     /* Where the ruler is displayed */
    struct area data_area;      /* Where the data is displayed */

    ssize_t force_max;          /* Force a max offset, or -1 */

    int dirty;                  /* Number of skipped updates */
    gboolean dpy_bbar_dirty;    /* Does the button bar need to be updated? */


    /* handle of search engine */
    mc_search_t *search;
    gchar *last_search_string;
    struct mcview_nroff_struct *search_nroff_seq;
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

typedef struct mcview_nroff_struct
{
    WView *view;
    off_t index;
    int char_length;
    int current_char;
    nroff_type_t type;
    nroff_type_t prev_type;
} mcview_nroff_t;



/* actions_cmd.c:  */
class ActionsCmd
{
public:
    static cb_ret_t mcview_callback(Widget* w, Widget* sender, widget_msg_t msg, int parm, void* data);

    static cb_ret_t mcview_dialog_callback(Widget* w, Widget* sender, widget_msg_t msg, int parm, void* data);

private:
    static gboolean mcview_ok_to_quit(WView* view);

    static void mcview_resize (WView* view);

    /** Both views */
    static cb_ret_t mcview_handle_key(WView* view, int key);

    static long mcview_lookup_key(WView* view, int key);

    static cb_ret_t mcview_execute_cmd(WView* view, long command);

    static void mcview_load_file_from_history(WView* view);

    static void mcview_load_next_prev(WView* view, int direction);

    static void mcview_scan_for_file(WView* view, int direction);

    static void mcview_load_next_prev_init(WView* view);

    static cb_ret_t mcview_handle_editkey(WView* view, int key);

    // FIXME DB: pass MView* as a param
    static void mcview_hook(void* v);

    static void mcview_continue_search_cmd(WView* view);

    /* Both views */
    static void mcview_search(WView* view, gboolean start_search);

    static void mcview_remove_ext_script(WView* view);
};


/* ascii.c: */
class Ascii
{
public:
    /**
     * In both wrap and unwrap modes, dpy_start points to the beginning of the paragraph.
     *
     * In unwrap mode, start displaying from this position, probably applying an additional horizontal
     * scroll.
     *
     * In wrap mode, an additional dpy_paragraph_skip_lines lines are skipped from the top of this
     * paragraph. dpy_state_top contains the position and parser-formatter state corresponding to the
     * top left corner so we can just start rendering from here. Unless dpy_wrap_dirty is set in which
     * case dpy_state_top is invalid and we need to recompute first.
     */
    static void mcview_display_text(WView* view);

    static void mcview_state_machine_init(mcview_state_machine_t* state, off_t offset);

    /**
     * Move down.
     *
     * It's very simple. Just invisibly format the next "lines" lines, carefully carrying the formatter
     * state in wrap mode. But before each step we need to check if we've already hit the end of the
     * file, in that case we can no longer move. This is done by walking from dpy_state_bottom.
     *
     * Note that this relies on mcview_display_text() setting dpy_state_bottom to its correct value
     * upon rendering the screen contents. So don't call this function from other functions (e.g. at
     * the bottom of mcview_ascii_move_up()) which invalidate this value.
     */
    static void mcview_ascii_move_down(WView* view, off_t lines);

    /**
     * Move up.
     *
     * Unwrap mode: Piece of cake. Wrap mode: If we'd walk back more than the current line offset
     * within the paragraph, we need to jump back to the previous paragraph and compute its height to
     * see if we start from that paragraph, and repeat this if necessary. Once we're within the desired
     * paragraph, we still need to format it from its beginning to know the state.
     *
     * See the top of this file for comments about MAX_BACKWARDS_WALK_IN_PARAGRAPH.
     *
     * force_max is a nice protection against the rare extreme case that the file underneath us
     * changes, we don't want to endlessly consume a file of maybe full of zeros upon moving upwards.
     */
    static void mcview_ascii_move_up(WView* view, off_t lines);

    static void mcview_ascii_moveto_bol(WView* view);

    static void mcview_ascii_moveto_eol(WView* view);

private:
    /**
     * Recompute dpy_state_top from dpy_start and dpy_paragraph_skip_lines. Clamp
     * dpy_paragraph_skip_lines if necessary.
     *
     * This method should be called in wrap mode after changing one of the parsing or formatting
     * properties (e.g. window width, encoding, nroff), or when switching to wrap mode from unwrap or
     * hex.
     *
     * If we stayed within the same paragraph then try to keep the vertical offset within that
     * paragraph as well. It might happen though that the paragraph became shorter than our desired
     * vertical position, in that case move to its last row.
     */
    static void mcview_wrap_fixup(WView* view);

    /**
     * Parse, format and possibly display one paragraph (perhaps not from the beginning).
     *
     * Formatting starts at the given "state" (which encodes the file offset and parser and formatter's
     * internal state). In unwrap mode, this should point to the beginning of the paragraph with the
     * default state, the additional horizontal scrolling is added here. In wrap mode, this may point
     * to the beginning of the line within a paragraph (to display the partial paragraph at the top),
     * with the proper state at that point.
     *
     * Displaying the next paragraph should start at "state"'s new value, or if we displayed the bottom
     * line then state->offset tells the file offset to be shown in the top bar.
     *
     * If "row" is negative, don't display the first abs(row) lines and display the rest from the top.
     * This was a nice idea but it's now unused :)
     *
     * If "row" is too large, don't display the paragraph at all but still return the number of lines.
     * This is used when moving upwards.
     *
     * @param view ...
     * @param state the parser-formatter state machine's state, updated
     * @param row print starting at this row
     * @return the number of rows the paragraphs is wrapped to, that is, 0 if we were already at EOF,
     *   otherwise 1 in unwrap mode, >= 1 in wrap mode. We stop when reaching the bottom of the
     *   viewport, it's not counted how many more lines the paragraph would occupy
     */
    static int mcview_display_paragraph(WView* view, mcview_state_machine_t* state, int row);

    /**
     * Parse, format and possibly display one visual line of text.
     *
     * Formatting starts at the given "state" (which encodes the file offset and parser and formatter's
     * internal state). In unwrap mode, this should point to the beginning of the paragraph with the
     * default state, the additional horizontal scrolling is added here. In wrap mode, this should
     * point to the beginning of the line, with the proper state at that point.
     *
     * In wrap mode, if a line ends in a newline, it is consumed, even if it's exactly at the right
     * edge. In unwrap mode, the whole remaining line, including the newline is consumed. Displaying
     * the next line should start at "state"'s new value, or if we displayed the bottom line then
     * state->offset tells the file offset to be shown in the top bar.
     *
     * If "row" is offscreen, don't actually display the line but still update "state" and return the
     * proper value. This is used by mcview_wrap_move_down to advance in the file.
     *
     * @param view ...
     * @param state the parser-formatter state machine's state, updated
     * @param row print to this row
     * @param paragraph_ended store TRUE if paragraph ended by newline or EOF, FALSE if wraps to next
     *   line
     * @param linewidth store the width of the line here
     * @return the number of rows, that is, 0 if we were already at EOF, otherwise 1
     */
    static int mcview_display_line(WView* view, mcview_state_machine_t* state, int row, gboolean* paragraph_ended, off_t* linewidth);

    /**
     * Get one base character, along with its combining or spacing mark characters.
     *
     * (A spacing mark is a character that extends the base character's width 1 into a combined
     * character of width 2, yet these two character cells should not be separated. E.g. Devanagari
     * <U+0939><U+094B>.)
     *
     * This method exists mainly for two reasons. One is to be able to tell if we fit on the current
     * line or need to wrap to the next one. The other is that both slang and ncurses seem to require
     * that the character and its combining marks are printed in a single call (or is it just a
     * limitation of mc's wrapper to them?).
     *
     * For convenience, this method takes care of converting CR or CR+LF into LF.
     * TODO this should probably happen later, when displaying the file?
     *
     * Normally: stores cs and color, updates state, returns >= 1 (entries in cs).
     * At EOF: state is unchanged, cs and color are undefined, returns 0.
     *
     * @param view ...
     * @param state the parser-formatter state machine's state, updated
     * @param cs store the characters here
     * @param clen the room available in cs (that is, at most clen-1 combining marks are allowed), must
     *   be at least 2
     * @param color if non-NULL, store the color here, taken from the first codepoint's color
     * @return the number of entries placed in cs, or 0 on EOF
     */
    static int mcview_next_combining_char_sequence(WView* view, mcview_state_machine_t* state, int* cs, int clen, int* color);

    /**
     * This function parses the next nroff character and gives it to you along with its desired color,
     * so you never have to care about nroff again.
     *
     * The nroff mode does the backspace trick for every single character (Unicode codepoint). At least
     * that's what the GNU groff 1.22 package produces, and that's what less 458 expects. For
     * double-wide characters (CJK), still only a single backspace is emitted. For combining accents
     * and such, the print-backspace-print step is repeated for the base character and then for each
     * accent separately.
     *
     * So, the right place for this layer is after the bytes are interpreted in UTF-8, but before
     * joining a base character with its combining accents.
     *
     * Normally: stores c and color, updates state, returns TRUE.
     * At EOF: state is unchanged, c and color are undefined, returns FALSE.
     *
     * color can be null if the caller doesn't care.
     */
    static gboolean mcview_get_next_maybe_nroff_char(WView* view, mcview_state_machine_t* state, int* c, int* color);

    /**
     * Just for convenience, a common interface in front of mcview_get_utf and mcview_get_byte, so that
     * the caller doesn't have to care about utf8 vs 8-bit modes.
     *
     * Normally: stores c, updates state, returns TRUE.
     * At EOF: state is unchanged, c is undefined, returns FALSE.
     *
     * Just as with mcview_get_utf(), invalid UTF-8 is reported using negative integers.
     *
     * Also, temporary hack: handle force_max here.
     * TODO: move it to lower layers (datasource.c)?
     */
    static gboolean mcview_get_next_char(WView* view, mcview_state_machine_t* state, int* c);

    static int mcview_char_display(const WView* view, int c, char* s);

    static gboolean mcview_isprint(const WView* view, int c);

    /* actually is_non_spacing_mark_or_enclosing_mark */
    static gboolean mcview_is_non_spacing_mark(const WView* view, int c);

    static gboolean mcview_ismark(const WView* view, int c);

    /* TODO: These methods shouldn't be necessary, see ticket 3257 */
    static int mcview_wcwidth(const WView* view, int c);

private:
    /*
     * Wrap mode: This is for safety so that jumping to the end of file (which already includes
     * scrolling back by a page) and then walking backwards is reasonably fast, even if the file is
     * extremely large and consists of maybe full zeros or something like that. If there's no newline
     * found within this limit, just start displaying from there and see what happens. We might get
     * some displaying parameteres (most importantly the columns) incorrect, but at least will show the
     * file without spinning the CPU for ages. When scrolling back to that point, the user might see a
     * garbled first line (even starting with an invalid partial UTF-8), but then walking back by yet
     * another line should fix it.
     *
     * Unwrap mode: This is not used, we wouldn't be able to do anything reasonable without walking
     * back a whole paragraph (well, view->data_area.height paragraphs actually).
     */
    static constexpr int MAX_BACKWARDS_WALK_IN_PARAGRAPH = (100 * 1000);

    /* I think anything other than space (e.g. arrows) just introduce visual clutter without actually
    * adding value. */
    static constexpr char PARTIAL_CJK_AT_LEFT_MARGIN = ' ';
    static constexpr char PARTIAL_CJK_AT_RIGHT_MARGIN = ' ';

    /* The Unicode standard recommends that lonely combining characters are printed over a dotted
    * circle. If the terminal is not UTF-8, this will be replaced by a dot anyway. */
    static constexpr int BASE_CHARACTER_FOR_LONELY_COMBINING = 0x25CC;      /* dotted circle */
    static constexpr int MAX_COMBINING_CHARS = 4;   /* both slang and ncurses support exactly 4 */
};


/* coord_cache.c: */
class CoordCache
{
public:
    enum ccache_type
    {
        CCACHE_OFFSET,
        CCACHE_LINECOL
    };
private:
    enum nroff_state_t
    {
        NROFF_START,
        NROFF_BACKSPACE,
        NROFF_CONTINUATION
    };
private:
    static constexpr int VIEW_COORD_CACHE_GRANUL = 1024;
    static constexpr int CACHE_CAPACITY_DELTA = 64;

    // FIXME DB use `using` or std::function
    typedef gboolean (*cmp_func_t) (const coord_cache_entry_t * a, const coord_cache_entry_t * b);
public:
    static coord_cache_t* coord_cache_new();

    static void coord_cache_free(coord_cache_t * cache);

#ifdef MC_ENABLE_DEBUGGING_CODE
    static void mcview_ccache_dump (WView* view);
#endif

    /** Look up the missing components of ''coord'', which are given by
     * ''lookup_what''. The function returns the smallest value that
     * matches the existing components of ''coord''.
     */
    static void mcview_ccache_lookup(WView* view, coord_cache_entry_t* coord, enum ccache_type lookup_what);

private:
    /** Find and return the index of the last cache entry that is
    * smaller than ''coord'', according to the criterion ''sort_by''. */
    static size_t mcview_ccache_find(WView* view, const coord_cache_entry_t * coord, cmp_func_t cmp_func);

    static gboolean mcview_coord_cache_entry_less_nroff(const coord_cache_entry_t* a, const coord_cache_entry_t* b);

    static gboolean mcview_coord_cache_entry_less_plain(const coord_cache_entry_t* a, const coord_cache_entry_t* b);

    static gboolean mcview_coord_cache_entry_less_offset(const coord_cache_entry_t* a, const coord_cache_entry_t* b);

    /* insert new cache entry into the cache */
    static void mcview_ccache_add_entry(coord_cache_t* cache, size_t pos, const coord_cache_entry_t* entry);
};


/* datasource.c: */
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


/* dialogs.c: */
class Dialogs
{
private:
    struct mcview_search_options_t
    {
        mc_search_type_t type;
        gboolean case_sens;
        gboolean backwards;
        gboolean whole_words;
        gboolean all_codepages;
    };

public:
    static inline mcview_search_options_t mcview_search_options = {
        .type = MC_SEARCH_T_NORMAL,
        .case_sens = FALSE,
        .backwards = FALSE,
        .whole_words = FALSE,
        .all_codepages = FALSE
    };
public:
    static gboolean mcview_dialog_search(WView* view);

    static gboolean mcview_dialog_goto(WView* view, off_t* offset);
};


/* display.c: */
class Display
{
private:
    /* If set, show a ruler */
    enum ruler_type
    {
        RULER_NONE,
        RULER_TOP,
        RULER_BOTTOM
    };

    static inline ruler_type ruler = RULER_NONE;

    /* The length of the line displays the file size */
    static constexpr int BUF_TRUNC_LEN = 5;
public:
    static void mcview_update(WView* view);

    /** Displays as much data from view->dpy_start as fits on the screen */
    static void mcview_display(WView* view);

    static void mcview_compute_areas(WView* view);

    static void mcview_update_bytes_per_line(WView* view);

    static void mcview_display_toggle_ruler(WView* view);

    static void mcview_display_clean(WView* view);

    static void mcview_display_ruler(WView* view);

private:
    static void mcview_display_status(WView* view);

    static void mcview_display_percent(WView* view, off_t p);

    /** Define labels and handlers for functional keys */
    static void mcview_set_buttonbar(WView* view);
};


/* growbuf.c: */
class Growbuf
{
private:
    /* Block size for reading files in parts */
    static constexpr size_t VIEW_PAGE_SIZE = 8192;
public:
    static void mcview_growbuf_init(WView* view);

    static void mcview_growbuf_done(WView* view);

    static void mcview_growbuf_free(WView* view);

    static off_t mcview_growbuf_filesize(WView* view);

    /** Copies the output from the pipe to the growing buffer, until either
     * the end-of-pipe is reached or the interval [0..ofs) of the growing
     * buffer is completely filled.
     */
    static void mcview_growbuf_read_until(WView* view, off_t ofs);

    static gboolean mcview_get_byte_growing_buffer(WView* view, off_t byte_index, int* retval);

    static char* mcview_get_ptr_growing_buffer(WView* view, off_t byte_index);
};


/* hex.c: */
class Hex
{
private:
    enum mark_t
    {
        MARK_NORMAL,
        MARK_SELECTED,
        MARK_CURSOR,
        MARK_CHANGED
    };
private:
    static const inline char hex_char[] = "0123456789ABCDEF";   // FIXME DB constexpr
public:
    static void mcview_display_hex(WView* view);

    static gboolean mcview_hexedit_save_changes(WView* view);

    static void mcview_toggle_hexedit_mode(WView* view);

    static void mcview_hexedit_free_change_list(WView* view);

    static void mcview_enqueue_change (struct hexedit_change_node **head, struct hexedit_change_node *node);

private:
    /** Determine the state of the current byte.
     *
     * @param view viewer object
     * @param from offset
     * @param curr current node
     */
    static mark_t mcview_hex_calculate_boldflag(WView* view, off_t from, struct hexedit_change_node* curr, gboolean force_changed);
};


/* lib.c: */
class Lib
{
public:
    static void mcview_toggle_magic_mode(WView* view);

    static void mcview_toggle_wrap_mode(WView* view);

    static void mcview_toggle_nroff_mode(WView* view);

    static void mcview_toggle_hex_mode(WView* view);

    static void mcview_init(WView* view);

    static void mcview_done(WView* view);

#ifdef HAVE_CHARSET
    static void mcview_select_encoding(WView* view);

    static void mcview_set_codeset(WView* view);
#endif /* HAVE_CHARSET */

    static void mcview_show_error(WView* view, const char* msg);

    /** returns index of the first char in the line
     * it is constant for all line characters
     */
    static off_t mcview_bol(WView* view, off_t current, off_t limit);

    /** returns index of last char on line + width EOL
     * mcview_eol of the current line == mcview_bol next line
     */
    static off_t mcview_eol(WView* view, off_t current);

    static char* mcview_get_title(const WDialog* h, size_t len);

    static int mcview_calc_percent(WView* view, off_t p);
};


/* move.c */
class Move
{
public:
    static void mcview_move_up(WView* view, off_t lines);

    static void mcview_move_down(WView* view, off_t lines);

    static void mcview_move_left(WView* view, off_t columns);

    static void mcview_move_right(WView* view, off_t columns);

    static void mcview_moveto_top(WView* view);

    static void mcview_moveto_bottom(WView* view);

    static void mcview_moveto_bol(WView* view);

    static void mcview_moveto_eol(WView* view);

    static void mcview_moveto_offset(WView* view, off_t offset);

    static void mcview_moveto(WView* view, off_t line, off_t col);

    static void mcview_coord_to_offset(WView* view, off_t* ret_offset, off_t line, off_t column);

    static void mcview_offset_to_coord(WView* view, off_t* ret_line, off_t* ret_column, off_t offset);

    static void mcview_place_cursor(WView* view);

    /** we have set view->search_start and view->search_end and must set
     * view->dpy_text_column and view->dpy_start
     * try to display maximum of match */
    static void mcview_moveto_match (WView* view);

private:
    static void mcview_scroll_to_cursor(WView* view);

    static void mcview_movement_fixups(WView* view, gboolean reset_search);
};



/* nroff.c: */
class Nroff
{
public:
    static int mcview__get_nroff_real_len (WView* view, off_t start, off_t length);

    static mcview_nroff_t* mcview_nroff_seq_new_num(WView* view, off_t lc_index);

    static mcview_nroff_t* mcview_nroff_seq_new(WView* view);

    static void mcview_nroff_seq_free(mcview_nroff_t** nroff);

    static nroff_type_t mcview_nroff_seq_info(mcview_nroff_t* nroff);

    static int mcview_nroff_seq_next(mcview_nroff_t* nroff);

    static int mcview_nroff_seq_prev(mcview_nroff_t* nroff);

private:
    static gboolean mcview_nroff_get_char(mcview_nroff_t* nroff, int* ret_val, off_t nroff_index);
};


/* search.c: */
class Search
{
private:
    struct mcview_search_status_msg_t
    {
        simple_status_msg_t status_msg;     /* base class */

        gboolean first;
        WView* view;
        off_t offset;
    };
private:
    static inline int search_cb_char_curr_index = -1;
    static inline char search_cb_char_buffer[6];
public:
    static mc_search_cbret_t mcview_search_cmd_callback(const void* user_data, std::size_t char_offset, int* current_char);

    static mc_search_cbret_t mcview_search_update_cmd_callback(const void* user_data, std::size_t char_offset);

    static void mcview_do_search(WView* view, off_t want_search_start);

private:
    static void mcview_search_show_result(WView* view, std::size_t match_len);

    static gboolean mcview_find(mcview_search_status_msg_t* ssm, off_t search_start, off_t search_end, std::size_t* len);

    static void mcview_search_update_steps(WView* view);

    static int mcview_search_status_update_cb(status_msg_t* sm);
};


/*** inline functions ****************************************************************************/

// FIXME DB
#include "inlines.hpp"

