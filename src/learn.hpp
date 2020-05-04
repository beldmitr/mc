/** \file learn.h
 *  \brief Header: learn keys module
 */

#pragma once

class Learn
{
private:
    static const int UX = 4;
    static const int UY = 2;

    static const int ROWS = 13;
    static const int COLSHIFT = 23;

private:
    struct learnkey_t
    {
        Widget *button;
        Widget *label;
        gboolean ok;
        char *sequence;
    };

private:
    static WDialog* learn_dlg;
    static const char *learn_title;

    static learnkey_t *learnkeys;
    static int learn_total;
    static int learnok;
    static gboolean learnchanged;

public:
    static void learn_keys();

private:
    static void learn_save();

    static void learn_done();

    static void init_learn();

    static cb_ret_t learn_callback(Widget * w, Widget * sender, widget_msg_t msg, int parm, void *data);

    static gboolean learn_check_key(int c);

    static gboolean learn_move(gboolean right);

    static int learn_button(WButton * button, int action);
};




