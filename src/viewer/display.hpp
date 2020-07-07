//
// Created by wbull on 5/24/20.
//

#pragma once

class WView;

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