//
// Created by wbull on 5/24/20.
//

#pragma once

class WView;

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
