#pragma once

#include "WView.hpp"

#define OFF_T_BITWIDTH  ((unsigned int) (sizeof (off_t) * CHAR_BIT - 1))
#define OFFSETTYPE_MAX (((off_t) 1 << (OFF_T_BITWIDTH - 1)) - 1)

#include "datasource.hpp"
#include "growbuf.hpp"

class Inlines
{
private:
    // FIXME DB: Decide what is better MACRO or constexpr
//    static constexpr unsigned int OFF_T_BITWIDTH = ((unsigned int) (sizeof (off_t) * CHAR_BIT - 1));
//    static constexpr unsigned int OFFSETTYPE_MAX = ((static_cast<off_t>(1) << (OFF_T_BITWIDTH - 1)) - 1);
public:
    /* difference or zero */
    template <typename T>
    static T diff_or_zero(T a, T b)
    {
        return (a >= b) ? a - b : 0;
    }

    static off_t mcview_offset_rounddown(off_t a, off_t b)
    {
        g_assert (b != 0);  // FIXME DB: use cassert instead
        return a - a % b;
    }

    /* {{{ Simple Primitive Functions for WView }}} */
    static gboolean mcview_is_in_panel(WView* view)
    {
        return (view->dpy_frame_size != 0);
    }

    static gboolean mcview_may_still_grow(WView* view)
    {
        return (view->growbuf_in_use && !view->growbuf_finished);
    }

    /* returns TRUE if the idx lies in the half-open interval
     * [offset; offset + size), FALSE otherwise.
     */
    static gboolean mcview_already_loaded(off_t offset, off_t idx, size_t size)
    {
        return (offset <= idx && idx - offset < (off_t) size);
    }

    static gboolean mcview_get_byte_file(WView* view, off_t byte_index, int* retval)
    {
        g_assert (view->datasource == WView::DS_FILE);

        DataSource::mcview_file_load_data(view, byte_index);
        if (mcview_already_loaded(view->ds_file_offset, byte_index, view->ds_file_datalen))
        {
            if (retval)
                *retval = view->ds_file_data[byte_index - view->ds_file_offset];
            return TRUE;
        }
        if (retval)
            *retval = -1;
        return FALSE;
    }

    static gboolean mcview_get_byte(WView* view, off_t offset, int* retval)
    {
        switch (view->datasource)
        {
            case WView::DS_STDIO_PIPE:
            case WView::DS_VFS_PIPE:return Growbuf::mcview_get_byte_growing_buffer(view, offset, retval);
            case WView::DS_FILE:return mcview_get_byte_file(view, offset, retval);
            case WView::DS_STRING:return DataSource::mcview_get_byte_string(view, offset, retval);
            case WView::DS_NONE:return DataSource::mcview_get_byte_none(view, offset, retval);
            default:return FALSE;
        }
    }

    static gboolean mcview_get_byte_indexed(WView* view, off_t base, off_t ofs, int* retval)
    {
        if (base <= OFFSETTYPE_MAX - ofs)
        {
            return mcview_get_byte(view, base + ofs, retval);
        }
        if (retval)
            *retval = -1;
        return FALSE;
    }

    static int mcview_count_backspaces(WView *view, off_t offset)
    {
        int backspaces = 0;
        int c;
        while (offset >= 2 * backspaces && mcview_get_byte(view, offset - 2 * backspaces, &c) && c == '\b')
            backspaces++;

        return backspaces;
    }

    static gboolean mcview_is_nroff_sequence(WView* view, off_t offset)
    {
        int c0, c1, c2;

        /* The following commands are ordered to speed up the calculation. */

        if (!mcview_get_byte_indexed(view, offset, 1, &c1) || c1 != '\b')
            return FALSE;

        if (!mcview_get_byte_indexed(view, offset, 0, &c0) || !g_ascii_isprint (c0))
            return FALSE;

        if (!mcview_get_byte_indexed(view, offset, 2, &c2) || !g_ascii_isprint (c2))
            return FALSE;

        return (c0 == c2 || c0 == '_' || (c0 == '+' && c2 == 'o'));
    }

    static void mcview_growbuf_read_all_data(WView* view)
    {
        Growbuf::mcview_growbuf_read_until(view, OFFSETTYPE_MAX);
    }
};