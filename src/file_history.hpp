#pragma once

#include "lib/widget.hpp"

class FileHistory
{
private:
    static constexpr const char* TMP_SUFFIX = ".tmp";

private:
    struct file_history_data_t
    {
        char* file_name;
        char* file_pos;
    };

public:
    /**
     * Show file history and return the selected file
     *
     * @param w widget used for positioning of history window
     * @param action to do with file (edit, view, etc)
     *
     * @return name of selected file, A newly allocated string.
     */
    static char* show_file_history(const Widget* w, int* action);

private:
    static GList* file_history_list_read();

    static void file_history_list_write(const GList* file_list);

    static void file_history_create_item(history_descriptor_t* hd, void* data);

    static void* file_history_release_item(history_descriptor_t* hd, WLEntry* le);

    static void file_history_free_item(void* data);
};
