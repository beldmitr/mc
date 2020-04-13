/** \file charsets.h
 *  \brief Header: Text conversion from one charset to another
 */

#pragma once

#include <string>

#include "lib/global.hpp"

// TODO Get rid of statics !!!!!

class CodepageDesc
{
private:
    std::string id;
    std::string name;
public:
    CodepageDesc(const std::string& id, const std::string& name)
        :   id(id),
            name(name)
    {
    }

public:
    [[nodiscard]] const char* GetId() const
    {
        return id.c_str();
    }

    [[nodiscard]] const char* GetName() const
    {
        return name.c_str();
    }

    void SetId(const std::string& id)
    {
        this->id = id;
    }

    void SetName(const std::string& name)
    {
        this->name = name;
    }

public:
    /* Convert single characters */
    static inline int convert_to_display_c (int c)
    {
        if (c < 0 || c >= 256)
            return c;
        return (int) conv_displ[c];
    }

    /* Convert single characters */
    static inline int convert_from_input_c (int c)
    {
        if (c < 0 || c >= 256)
            return c;
        return (int) conv_input[c];
    }

public:
    static char* init_translation_table (int cpsource, int cpdisplay);

    static void free_codepages_list ();

    static GString* str_convert_to_input (const char *str);

    static GString* str_convert_to_display (const char *str);

    static const char* get_codepage_id (int n);

    static int get_codepage_index(const char *id);

    static void load_codepages_list ();

    /** Check if specified encoding can be used in mc.
     * @param encoding name of encoding
     * @return TRUE if encoding is supported by mc, FALSE otherwise
     */
    static gboolean is_supported_encoding (const char *encoding);

    /*
     * Converter from utf to selected codepage
     * param str, utf char
     * return char in needle codepage (by global int mc_global.source_codepage)
     */
    static unsigned char convert_from_utf_to_current (const char *str);

    /*
     * Converter from utf to selected codepage
     * param input_char, gunichar
     * return char in needle codepage (by global int mc_global.source_codepage)
     */
    static unsigned char convert_from_utf_to_current_c (int input_char, GIConv conv);

    /*
     * Converter from selected codepage 8-bit
     * param char input_char, GIConv converter
     * return int utf char
     */
    static int convert_from_8bit_to_utf_c(char input_char, GIConv conv);

    /*
     * Converter from display codepage 8-bit to utf-8
     * param char input_char, GIConv converter
     * return int utf char
     */
    static int convert_from_8bit_to_utf_c2 (char input_char);

private:
    static void free_codepage_desc (gpointer data, gpointer user_data)
    {
    }

    static void convert_to_display (char *str);

    static void convert_from_input (char *str);


    static GString* str_nconvert_to_display (const char *str, int len);

    static char translate_character (GIConv cd, char c);

    static void load_codepages_list_from_file (GPtrArray ** list, const char *fname);

    static GString* str_nconvert_to_input (const char *str, int len);

public:
    static unsigned char conv_displ[256];
    static unsigned char conv_input[256];
    static GPtrArray *codepages;
    static const char *cp_display;
    static const char *cp_source;
};
