#pragma once

#include "lib/global.hpp"
#include "lib/keybind.hpp"        /* global_keymap_t */
#include "lib/mcconfig.hpp"       /* mc_config_t */

class KeyBindDefaults
{
private:
    /* default keymaps in ini (key=value) format */
    struct global_keymap_ini_t
    {
        const char* key;
        const char* value;
    };

public:
    static mc_config_t* create_default_keymap();

private:
    static void create_default_keymap_section(mc_config_t* keymap, const char* section, const global_keymap_ini_t* k);

public:
    static GArray* main_keymap;
    static GArray* main_x_keymap;
    static GArray* panel_keymap;
    static GArray* dialog_keymap;
    static GArray* menu_keymap;
    static GArray* input_keymap;
    static GArray* listbox_keymap;
    static GArray* tree_keymap;
    static GArray* help_keymap;
#ifdef USE_INTERNAL_EDIT
    static GArray* editor_keymap;
    static GArray* editor_x_keymap;
#endif
    static GArray* viewer_keymap;
    static GArray* viewer_hex_keymap;
#ifdef USE_DIFF_VIEW
    static GArray* diff_keymap;
#endif

    static const global_keymap_t* main_map;
    static const global_keymap_t* main_x_map;
    static const global_keymap_t* panel_map;
    static const global_keymap_t* tree_map;
    static const global_keymap_t* help_map;

#ifdef USE_INTERNAL_EDIT
    static const global_keymap_t* editor_map;
    static const global_keymap_t* editor_x_map;
#endif
    static const global_keymap_t* viewer_map;
    static const global_keymap_t* viewer_hex_map;
#ifdef USE_DIFF_VIEW
    static const global_keymap_t* diff_map;
#endif

private:
    /* midnight */
    static const global_keymap_ini_t default_main_keymap[];

    static const global_keymap_ini_t default_main_x_keymap[];

    /* panel */
    static const global_keymap_ini_t default_panel_keymap[];

    /* dialog */
    static const global_keymap_ini_t default_dialog_keymap[];

    /* menubar */
    static const global_keymap_ini_t default_menu_keymap[];

    /* input line */
    static const global_keymap_ini_t default_input_keymap[];

    /* listbox */
    static const global_keymap_ini_t default_listbox_keymap[];

    /* tree */
    static const global_keymap_ini_t default_tree_keymap[];

    /* help */
    static const global_keymap_ini_t default_help_keymap[];

#ifdef USE_INTERNAL_EDIT
    static const global_keymap_ini_t default_editor_keymap[];

    /* emacs keyboard layout emulation */
    static const global_keymap_ini_t default_editor_x_keymap[];
#endif /* USE_INTERNAL_EDIT */

    /* viewer */
    static const global_keymap_ini_t default_viewer_keymap[];

    /* hex viewer */
    static const global_keymap_ini_t default_viewer_hex_keymap[];

#ifdef  USE_DIFF_VIEW
    /* diff viewer */
    static const global_keymap_ini_t default_diff_keymap[];
#endif
};
