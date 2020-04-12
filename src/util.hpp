#pragma once

#include "lib/global.hpp"
#include "src/filemanager/file.hpp"
#include "src/filemanager/filegui.hpp"

class Util
{
public:
    static bool Check_for_default(const vfs_path_t *default_file_vpath, const vfs_path_t *file_vpath)
    {
        if (!exist_file (vfs_path_as_str (file_vpath)))
        {
            if (!exist_file (vfs_path_as_str (default_file_vpath)))
                return false;

            file_op_context_t *ctx = file_op_context_new (OP_COPY);
            file_op_total_context_t *tctx = file_op_total_context_new ();
            file_op_context_create_ui (ctx, 0, static_cast<filegui_dialog_type_t>(FALSE));
            copy_file_file (tctx, ctx, vfs_path_as_str (default_file_vpath), vfs_path_as_str (file_vpath));
            file_op_total_context_destroy (tctx);
            file_op_context_destroy (ctx);
        }

        return true;
    }
};
