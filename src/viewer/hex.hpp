//
// Created by wbull on 5/24/20.
//

#pragma once

class Hex
{
private:
    enum mark_t
    {
        MARK_NORMAL,
        MARK_SELECTED,
        MARK_CURSOR,
        MARK_CHANGED
    };

public:
    /* A node for building a change list on change_list */
    struct hexedit_change_node
    {
        struct hexedit_change_node *next;
        off_t offset;
        byte value;
    };
private:
    static const inline char hex_char[] = "0123456789ABCDEF";   // FIXME DB constexpr
public:
    static void mcview_display_hex(WView* view);

    static gboolean mcview_hexedit_save_changes(WView* view);

    static void mcview_toggle_hexedit_mode(WView* view);

    static void mcview_hexedit_free_change_list(WView* view);

    static void mcview_enqueue_change (struct hexedit_change_node **head, struct hexedit_change_node *node);

private:
    /** Determine the state of the current byte.
     *
     * @param view viewer object
     * @param from offset
     * @param curr current node
     */
    static mark_t mcview_hex_calculate_boldflag(WView* view, off_t from, struct hexedit_change_node* curr, gboolean force_changed);
};