/** \file help.h
 *  \brief Header: hypertext file browser
 *
 *  Implements the hypertext file viewer.
 *  The hypertext file is a file that may have one or more nodes.  Each
 *  node ends with a ^D character and starts with a bracket, then the
 *  name of the node and then a closing bracket. Right after the closing
 *  bracket a newline is placed. This newline is not to be displayed by
 *  the help viewer and must be skipped - its sole purpose is to faciliate
 *  the work of the people managing the help file template (xnc.hlp) .
 *
 *  Links in the hypertext file are specified like this: the text that
 *  will be highlighted should have a leading ^A, then it comes the
 *  text, then a ^B indicating that highlighting is done, then the name
 *  of the node you want to link to and then a ^C.
 *
 *  The file must contain a ^D at the beginning and at the end of the
 *  file or the program will not be able to detect the end of file.
 *
 *  Lazyness/widgeting attack: This file does use the dialog manager
 *  and uses mainly the dialog to achieve the help work.  there is only
 *  one specialized widget and it's only used to forward the mouse messages
 *  to the appropriate routine.
 *
 *  This file is included by help.c and man2hlp.c
 */

#pragma once

#include "lib/widget.hpp"

class Help
{
private:
    /* Markers used in the help files */
    static constexpr char CHAR_LINK_START = '\01';      /* Ctrl-A */
    static constexpr char CHAR_LINK_POINTER = '\02';    /* Ctrl-B */
    static constexpr char CHAR_LINK_END = '\03';        /* Ctrl-C */
    static constexpr char CHAR_NODE_END = '\04';        /* Ctrl-D */
    static constexpr char CHAR_ALTERNATE = '\05';       /* Ctrl-E */
    static constexpr char CHAR_NORMAL = '\06';          /* Ctrl-F */
    static constexpr char CHAR_VERSION = '\07';         /* Ctrl-G */
    static constexpr char CHAR_FONT_BOLD = '\010';      /* Ctrl-H */
    static constexpr char CHAR_FONT_NORMAL = '\013';    /* Ctrl-K */
    static constexpr char CHAR_FONT_ITALIC = '\024';    /* Ctrl-T */
private:
    static constexpr int MAXLINKNAME = 80;
    static constexpr int HISTORY_SIZE = 20;


    static constexpr const char* STRING_LINK_START = "\01";
    static constexpr const char* STRING_LINK_POINTER = "\02";
    static constexpr const char* STRING_LINK_END = "\03";
    static constexpr const char* STRING_NODE_END = "\04";

public:
    /* event callback */
    static gboolean help_interactive_display(const char* event_group_name, const char* event_name, void* init_data, void* data);

private:
    /** returns the position where text was found in the start buffer
     * or 0 if not found
     */
    static const char* search_string (const char* start, const char* text);

    /** Searches text in the buffer pointed by start.  Search ends
     * if the CHAR_NODE_END is found in the text.
     * @return NULL on failure
     */
    static const char* search_string_node (const char* start, const char* text);

    /** Searches the_char in the buffer pointer by start and searches
     * it can search forward (direction = 1) or backward (direction = -1)
     */
    static const char* search_char_node (const char* start, char the_char, int direction);

    /** Returns the new current pointer when moved lines lines */
    static const char* move_forward2 (const char* c, int lines);

    static const char* move_backward2 (const char* c, int lines);

    static void move_forward (int i);

    static void move_backward (int i);

    static void move_to_top();

    static void move_to_bottom();

    static const char* help_follow_link (const char* start, const char* lc_selected_item);

    static const char* select_next_link (const char* current_link);

    static const char* select_prev_link (const char* current_link);

    static void start_link_area (int x, int y, const char* link_name);

    static void end_link_area (int x, int y);

    static void clear_link_areas();

    static void help_print_word(WDialog* h, GString* word, int* col, int* line, gboolean add_space);

    static void help_show(WDialog* h, const char* paint_start);

    /** show help */
    static void help_help(WDialog* h);

    static void help_index(WDialog * h);

    static void help_back (WDialog* h);

    static void help_next_link(gboolean move_down);

    static void help_prev_link(gboolean move_up);

    static void help_next_node();

    static void help_prev_node();

    static void help_select_link();

    static cb_ret_t help_execute_cmd(long command);

    static cb_ret_t help_handle_key(WDialog* h, int key);

    static cb_ret_t help_bg_callback(Widget* w, Widget* sender, widget_msg_t msg, int parm, void* data);

    static cb_ret_t help_resize (WDialog* h);

    static cb_ret_t help_callback (Widget* w, Widget* sender, widget_msg_t msg, int parm, void* data);

    static void interactive_display_finish();

    /** translate help file into terminal encoding */
    static void translate_file(char* filedata);

    static cb_ret_t md_callback (Widget* w, Widget* sender, widget_msg_t msg, int parm, void* data);

    static void help_mouse_callback (Widget* w, mouse_msg_t msg, mouse_event_t* event);

    static Widget* mousedispatch_new (int y, int x, int yl, int xl);
private:
    /* Link areas for the mouse */
    struct Link_Area
    {
        int x1, y1, x2, y2;
        const char *link_name;
    };

    struct history_t
    {
        const char* page;           /* Pointer to the selected page */
        const char* link;           /* Pointer to the selected link */
    };
private:
    static char* fdata;             /* Pointer to the loaded data file */
    static int help_lines;          /* Lines in help viewer */
    static int history_ptr;         /* For the history queue */
    static const char* main_node;   /* The main node */
    static const char* last_shown;  /* Last byte shown in a screen */
    static gboolean end_of_node;    /* Flag: the last character of the node shown? */
    static const char* currentpoint;
    static const char* selected_item;

    /* The widget variables */
    static WDialog* whelp;

    static history_t history[HISTORY_SIZE];

    static GSList* link_area;
    static gboolean inside_link_area;
};