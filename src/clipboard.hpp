/** \file  clipboard.h
 *  \brief Header: Util for external clipboard
 */

#pragma once

class Clipboard
{
public:
    /* path to X clipboard utility */
    static inline char *clipboard_store_path = nullptr;
    static inline char *clipboard_paste_path = nullptr;
public:
    /* event callback */
    static gboolean clipboard_file_to_ext_clip (const char* event_group_name, const char* event_name, void* init_data, void* data);

    /* event callback */
    static gboolean clipboard_file_from_ext_clip (const char* event_group_name, const char* event_name, void* init_data, void* data);

    /* event callback */
    static gboolean clipboard_text_to_file (const char* event_group_name, const char* event_name, void* init_data, void* data);

    /* event callback */
    static gboolean clipboard_text_from_file (const char* event_group_name, const char* event_name, void* init_data, void* data);
private:
    static const int clip_open_flags = O_CREAT | O_WRONLY | O_TRUNC | O_BINARY;
    static const mode_t clip_open_mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
};