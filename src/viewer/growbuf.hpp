//
// Created by wbull on 5/24/20.
//

#pragma once

class Growbuf
{
private:
    /* Block size for reading files in parts */
    static constexpr size_t VIEW_PAGE_SIZE = 8192;
public:
    static void mcview_growbuf_init(WView* view);

    static void mcview_growbuf_done(WView* view);

    static void mcview_growbuf_free(WView* view);

    static off_t mcview_growbuf_filesize(WView* view);

    /** Copies the output from the pipe to the growing buffer, until either
     * the end-of-pipe is reached or the interval [0..ofs) of the growing
     * buffer is completely filled.
     */
    static void mcview_growbuf_read_until(WView* view, off_t ofs);

    static gboolean mcview_get_byte_growing_buffer(WView* view, off_t byte_index, int* retval);

    static char* mcview_get_ptr_growing_buffer(WView* view, off_t byte_index);
};
