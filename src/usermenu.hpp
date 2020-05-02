/** \file usermenu.h
 *  \brief Header: user menu implementation
 */

#pragma once

#include "lib/global.hpp"

#include "src/editor/edit.hpp"    /* WEdit */
#include "src/filemanager/panel.hpp"    /* WPanel */

class UserMenu
{
public:
    /* Formats defined:
       %%  The % character
       %f  The current file in the active panel (if non-local vfs, file will be copied locally
       and %f will be full path to it) or the opened file in the internal editor.
       %p  Likewise.
       %d  The current working directory
       %s  "Selected files"; the tagged files if any, otherwise the current file
       %t  Tagged files
       %u  Tagged files (and they are untagged on return from expand_format)
       %view Runs the commands and pipes standard output to the view command.
       If %view is immediately followed by '{', recognize keywords
       ascii, hex, nroff and unform

       If the format letter is in uppercase, it refers to the other panel.

       With a number followed the % character you can turn quoting on (default)
       and off. For example:
       %f    quote expanded macro
       %1f   ditto
       %0f   don't quote expanded macro

       expand_format returns a memory block that must be free()d.
     */

    /**
     * If edit_widget is NULL then we are called from the mc menu,
     * otherwise we are called from the mcedit menu.
     */
    static gboolean user_menu_cmd (const WEdit * edit_widget, const char *menu_file, int selected_entry);

    static char* expand_format (const WEdit * edit_widget, char c, gboolean do_quote);

    /* Returns how many characters we should advance if %view was found */
    static int check_format_view (const char *p);

    /* Check if p has a "^var\{var-name\}" */
    /* Returns the number of skipped characters (zero on not found) */
    /* V will be set to the expanded variable name */
    static int check_format_var (const char *p, char **v);

    static int check_format_cd (const char *p);
private:
    /**
     **     Check owner of the menu file. Using menu file is allowed, if
     **     owner of the menu is root or the actual user. In either case
     **     file should not be group and word-writable.
     **
     **     Q. Should we apply this routine to system and home menu (and .ext files)?
     */
    static gboolean menu_file_own (char *path);

    /** FIXME: recode this routine on version 3.0, it could be cleaner */
    static void execute_menu_command (const WEdit * edit_widget, const char *commands, gboolean show_prompt);

    /** Calculates the truth value of one lineful of conditions. Returns
   the point just before the end of line. */
    static char* test_line (const WEdit * edit_widget, char *p, gboolean * result);

    /** General purpose condition debug output handler */
    static void debug_out (char *start, char *end, gboolean condition);

    /** Calculates the truth value of the next condition starting from
   p. Returns the point after condition. */
    static char* test_condition (const WEdit * edit_widget, char *p, gboolean * condition);

    /* Tests whether the selected file in the panel is of any of the types
   specified in argument. */
    static gboolean test_type (WPanel * panel, char *arg);

    /** Copies a whitespace separated argument from p to arg. Returns the
   point after argument. */
    static char* extract_arg (char *p, char *arg, int size);

    /**
     * Check for the "shell_patterns" directive.  If it's found and valid,
     * interpret it and move the pointer past the directive.  Return the
     * current pointer.
     */
    static char* check_patterns (char *p);

    /** strip file's extension */
    static char* strip_ext (char *ss);

private:
    static inline gboolean debug_flag = FALSE;
    static inline gboolean debug_error = FALSE;
    static inline char *menu = NULL;
};

