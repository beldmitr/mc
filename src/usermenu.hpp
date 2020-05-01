/** \file usermenu.h
 *  \brief Header: user menu implementation
 */

#pragma once

#include "lib/global.hpp"
#include "src/filemanager/panel.hpp" /* WPanel */

#include "src/editor/edit.hpp"    /* WEdit */

class UserMenu
{
public:
    /*
     * If edit_widget is NULL then we are called from the mc menu,
     * otherwise we are called from the mcedit menu.
     */
    static gboolean Cmd(const WEdit * edit_widget, const char *menu_file, int selected_entry);

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
    static char* ExpandFormat(const WEdit * edit_widget, char c, gboolean do_quote);

    /*
     * Returns how many characters we should advance if %view was found
     */
    static int CheckFormatView(const char *p);

    /*
     *  Check if p has a "^var\{var-name\}"
     *  Returns the number of skipped characters (zero on not found)
     *  V will be set to the expanded variable name
     */
    static int CheckFormatVar (const char *p, char **v);

    static int CheckFormatCD(const char *p);
private:
    /** strip file's extension */
    static char* StripExt(char *ss);

    /**
     * Check for the "shell_patterns" directive.  If it's found and valid,
     * interpret it and move the pointer past the directive.  Return the
     * current pointer.
     */
    static char* CheckPatterns(char *p);

    /**
     * Copies a whitespace separated argument from p to arg. Returns the point after argument.
     */
    static char* ExtractArg(char *p, char *arg, int size);

    /*
     * Tests whether the selected file in the panel is of any of the types specified in argument.
     */
    static gboolean TestType(WPanel* panel, char *arg);

    /*
     * Calculates the truth value of the next condition starting from p. Returns the point after condition.
     */
    static char* TestCondition(const WEdit * edit_widget, char *p, gboolean * condition);

    /*
     * General purpose condition debug output handler
     */
    static void DebugOut(char *start, char *end, gboolean condition);

    /*
     * Calculates the truth value of one lineful of conditions. Returns the point just before the end of line.
     */
    static char* TestLine(const WEdit * edit_widget, char *p, gboolean * result);

    /** FIXME: recode this routine on version 3.0, it could be cleaner */
    static void ExecuteMenuCommand(const WEdit * edit_widget, const char *commands, gboolean show_prompt);

    /*
     *     Check owner of the menu file. Using menu file is allowed, if
     *     owner of the menu is root or the actual user. In either case
     *     file should not be group and word-writable.
     *
     *     Q. Should we apply this routine to system and home menu (and .ext files)?
     */
    static gboolean MenuFileOwn(char *path);

private:
    static inline gboolean debugFlag = FALSE;
    static inline gboolean debugError = FALSE;
    static inline char *menu = nullptr;

private:
    static const int MAX_ENTRIES = 16;
    static const int MAX_ENTRY_LEN = 60;
};

