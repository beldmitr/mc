//
// Created by wbull on 5/24/20.
//

#pragma once

#include "dialogs.hpp"

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
