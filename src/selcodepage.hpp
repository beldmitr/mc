
/** \file selcodepage.h
 *  \brief Header: user %interface for charset %selection
 */

#pragma once

#include "lib/global.hpp"


class SelCodePage
{
public:
    /* Return value:
 *   -2 (SELECT_CHARSET_CANCEL)       : Cancel
 *   -1 (SELECT_CHARSET_OTHER_8BIT)   : "Other 8 bit"    if seldisplay == TRUE
 *   -1 (SELECT_CHARSET_NO_TRANSLATE) : "No translation" if seldisplay == FALSE
 *   >= 0                             : charset number
 */
    static int select_charset(int center_y, int center_x, int current_charset, bool seldisplay);

    /** Set codepage */
    static bool do_set_codepage(int codepage);

    /** Show menu selecting codepage */
    static bool do_select_codepage();

private:
    static unsigned char get_hotkey(int n);

private:
    const static int ENTRY_LEN = 30;
public:
    /* some results of select_charset() */
    const static int SELECT_CHARSET_CANCEL = -2;
    /* select_charset() returns this value if dialog has been canceled */
    const static int SELECT_CHARSET_OTHER_8BIT = -1;
    /* select_charset() returns this value if seldisplay == TRUE
     * and the last item has been selected. Last item is "Other 8 bits" */
    const static int SELECT_CHARSET_NO_TRANSLATE = -1;
    /* select_charset() returns this value if seldisplay == FALSE
     * and the 1st item has been selected. 1st item is "No translation" */
    /* In other cases select_charset() returns non-negative value
     * which is number of codepage in codepage list */
};