//
// Created by wbull on 5/24/20.
//

#pragma once

class Nroff
{
public:
    enum nroff_type_t
    {
        NROFF_TYPE_NONE = 0,
        NROFF_TYPE_BOLD = 1,
        NROFF_TYPE_UNDERLINE = 2
    };

    struct mcview_nroff_t
    {
        WView *view;
        off_t index;
        int char_length;
        int current_char;
        nroff_type_t type;
        nroff_type_t prev_type;
    };

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
