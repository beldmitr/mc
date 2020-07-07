//
// Created by wbull on 5/24/20.
//

#pragma once

#include "lib/search.hpp"

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