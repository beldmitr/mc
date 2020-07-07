//
// Created by wbull on 5/24/20.
//

#pragma once

class WView;

class ActionsCmd
{
public:
    static cb_ret_t mcview_callback(Widget* w, Widget* sender, widget_msg_t msg, int parm, void* data);

    static cb_ret_t mcview_dialog_callback(Widget* w, Widget* sender, widget_msg_t msg, int parm, void* data);

private:
    static gboolean mcview_ok_to_quit(WView* view);

    static void mcview_resize (WView* view);

    /** Both views */
    static cb_ret_t mcview_handle_key(WView* view, int key);

    static long mcview_lookup_key(WView* view, int key);

    static cb_ret_t mcview_execute_cmd(WView* view, long command);

    static void mcview_load_file_from_history(WView* view);

    static void mcview_load_next_prev(WView* view, int direction);

    static void mcview_scan_for_file(WView* view, int direction);

    static void mcview_load_next_prev_init(WView* view);

    static cb_ret_t mcview_handle_editkey(WView* view, int key);

    // FIXME DB: pass MView* as a param
    static void mcview_hook(void* v);

    static void mcview_continue_search_cmd(WView* view);

    /* Both views */
    static void mcview_search(WView* view, gboolean start_search);

    static void mcview_remove_ext_script(WView* view);
};