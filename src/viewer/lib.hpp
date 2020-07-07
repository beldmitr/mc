//
// Created by wbull on 5/24/20.
//

#pragma once

class WDialog;
class WView;

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