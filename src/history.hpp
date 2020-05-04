/** \file src/history.h
 *  \brief Header: defines history section names
 */

#pragma once

/* history section names */
class History
{
public:
    static inline const char* MC_HISTORY_EDIT_SAVE_AS = "mc.edit.save-as";
    static inline const char* MC_HISTORY_EDIT_LOAD = "mc.edit.load";
    static inline const char* MC_HISTORY_EDIT_SAVE_BLOCK = "mc.edit.save-block";
    static inline const char* MC_HISTORY_EDIT_INSERT_FILE = "mc.edit.insert-file";
    static inline const char* MC_HISTORY_EDIT_GOTO_LINE = "mc.edit.goto-line";
    static inline const char* MC_HISTORY_EDIT_SORT = "mc.edit.sort";
    static inline const char* MC_HISTORY_EDIT_PASTE_EXTCMD = "mc.edit.paste-extcmd";
    static inline const char* MC_HISTORY_EDIT_REPEAT = "mc.edit.repeat-action";

    static inline const char* MC_HISTORY_FM_VIEW_FILE = "mc.fm.view-file";
    static inline const char* MC_HISTORY_FM_MKDIR = "mc.fm.mkdir";
    static inline const char* MC_HISTORY_FM_LINK = "mc.fm.link";
    static inline const char* MC_HISTORY_FM_EDIT_LINK = "mc.fm.edit-link";
    static inline const char* MC_HISTORY_FM_TREE_COPY = "mc.fm.tree-copy";
    static inline const char* MC_HISTORY_FM_TREE_MOVE = "mc.fm.tree-move";
    static inline const char* MC_HISTORY_FM_PANELIZE_ADD = "mc.fm.panelize.add";
    static inline const char* MC_HISTORY_FM_FILTERED_VIEW = "mc.fm.filtered-view";
    static inline const char* MC_HISTORY_FM_PANEL_FILTER = "mc.fm.panel-filter";
    static inline const char* MC_HISTORY_FM_MENU_EXEC_PARAM = "mc.fm.menu.exec.parameter";

    static inline const char* MC_HISTORY_ESC_TIMEOUT = "mc.esc.timeout";

    static inline const char* MC_HISTORY_VIEW_GOTO = "mc.view.goto";
    static inline const char* MC_HISTORY_VIEW_GOTO_LINE = "mc.view.goto-line";
    static inline const char* MC_HISTORY_VIEW_GOTO_ADDR = "mc.view.goto-addr";
    static inline const char* MC_HISTORY_VIEW_SEARCH_REGEX = "mc.view.search.regex";

    static inline const char* MC_HISTORY_FTPFS_ACCOUNT = "mc.vfs.ftp.account";

    static inline const char* MC_HISTORY_EXT_PARAMETER = "mc.ext.parameter";

    static inline const char* MC_HISTORY_HOTLIST_ADD = "mc.hotlist.add";

    static inline const char* MC_HISTORY_SHARED_SEARCH = "mc.shared.search";

    static inline const char* MC_HISTORY_YDIFF_GOTO_LINE = "mc.ydiff.goto-line";
};
