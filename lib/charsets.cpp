/*
   Text conversion from one charset to another.

   Copyright (C) 2001-2020
   Free Software Foundation, Inc.

   Written by:
   Walery Studennikov <despair@sama.ru>

   This file is part of the Midnight Commander.

   The Midnight Commander is free software: you can redistribute it
   and/or modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation, either version 3 of the License,
   or (at your option) any later version.

   The Midnight Commander is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/** \file charsets.c
 *  \brief Source: Text conversion from one charset to another
 */

#include "lib/charsets.hpp"

#include "lib/fileloc.hpp"
#include "lib/strutil.hpp"        /* utf-8 functions */
#include "lib/util.hpp"           /* whitespace() */


#define UNKNCHAR '\001'
#define OTHER_8BIT "Other_8_bit"

unsigned char CodepageDesc::conv_displ[256];
unsigned char CodepageDesc::conv_input[256];

GPtrArray* CodepageDesc::codepages = nullptr;

const char* CodepageDesc::cp_display = nullptr;
const char* CodepageDesc::cp_source = nullptr;

void CodepageDesc::load_codepages_list_from_file (GPtrArray ** list, const char *fname)
{
    char buf[BUF_MEDIUM];
    char *default_codepage = nullptr;

    FILE *f = fopen (fname, "r");   // TODO use fstream
    if (!f)
        return;

    while (fgets (buf, sizeof buf, f) != nullptr)
    {
        /* split string into id and cpname */
        char *p = buf;

        if (*p == '\n' || *p == '\0' || *p == '#')
            continue;

        size_t buflen = std::strlen(buf);

        if (buflen != 0 && buf[buflen - 1] == '\n')
            buf[buflen - 1] = '\0';
        while (*p != '\0' && !whitespace (*p))
            ++p;
        if (*p == '\0')
        {
            fclose (f);
            return;
        }
        *p++ = '\0';
        g_strstrip (p);
        if (*p == '\0')
        {
            fclose (f);
            return;
        }

        if (strcmp (buf, "default") == 0)
            default_codepage = g_strdup (p);
        else
        {
            const char *id = buf;

            if (*list == nullptr)
            {
                *list = g_ptr_array_sized_new (16);
                g_ptr_array_add (*list, new CodepageDesc(id, p));
            }
            else
            {
                unsigned int i;

                /* whether id is already present in list */
                /* if yes, overwrite description */
                for (i = 0; i < (*list)->len; i++)
                {
                    auto *desc = static_cast<CodepageDesc*>(g_ptr_array_index (*list, i));

                    if (strcmp (id, desc->GetId()) == 0)
                    {
                        /* found */
                        desc->SetName(p);
                        break;
                    }
                }

                /* not found */
                if (i == (*list)->len)
                    g_ptr_array_add (*list, new CodepageDesc(id, p));
            }
        }
    }

    if (default_codepage != nullptr)
    {
        mc_global.display_codepage = get_codepage_index (default_codepage);
        g_free (default_codepage);
    }

    fclose (f);
}

int CodepageDesc::convert_from_8bit_to_utf_c2(char input_char)
{
    int ch = '.';
    GIConv conv;
    const char *cp_from;

    cp_from = get_codepage_id (mc_global.source_codepage);

    conv = str_crt_conv_to (cp_from);
    if (conv != INVALID_CONV)
    {
        ch = convert_from_8bit_to_utf_c (input_char, conv);
        str_close_conv (conv);
    }

    return ch;
}

int CodepageDesc::convert_from_8bit_to_utf_c(char input_char, GIConv conv)
{
    unsigned char str[2];
    unsigned char buf_ch[UTF8_CHAR_LEN + 1];
    int ch;

    str[0] = (unsigned char) input_char;
    str[1] = '\0';

    switch (str_translate_char (conv, (char *) str, -1, (char *) buf_ch, sizeof (buf_ch)))
    {
        case ESTR_SUCCESS:
        {
            int res;

            res = g_utf8_get_char_validated ((char *) buf_ch, -1);
            ch = res >= 0 ? res : buf_ch[0];
            break;
        }
        case ESTR_PROBLEM:
        case ESTR_FAILURE:
        default:
            ch = '.';
            break;
    }

    return ch;
}

unsigned char CodepageDesc::convert_from_utf_to_current_c(int input_char, GIConv conv)
{
    unsigned char str[UTF8_CHAR_LEN + 1];
    unsigned char buf_ch[UTF8_CHAR_LEN + 1];
    unsigned char ch = '.';

    int res = g_unichar_to_utf8 (input_char, (char *) str);
    if (res == 0)
        return ch;

    str[res] = '\0';

    switch (str_translate_char (conv, (char *) str, -1, (char *) buf_ch, sizeof (buf_ch)))
    {
        case ESTR_SUCCESS:
            ch = buf_ch[0];
            break;
        case ESTR_PROBLEM:
        case ESTR_FAILURE:
            ch = '.';
            break;
        default:
            break;
    }

    return ch;
}

unsigned char CodepageDesc::convert_from_utf_to_current(const char *str)
{
    unsigned char buf_ch[UTF8_CHAR_LEN + 1];
    unsigned char ch = '.';

    if (!str)
        return '.';

    const char *cp_to = get_codepage_id (mc_global.source_codepage);
    GIConv conv = str_crt_conv_to (cp_to);

    if (conv != INVALID_CONV)
    {
        switch (str_translate_char (conv, str, -1, (char *) buf_ch, sizeof (buf_ch)))
        {
            case ESTR_SUCCESS:
                ch = buf_ch[0];
                break;
            case ESTR_PROBLEM:
            case ESTR_FAILURE:
                ch = '.';
                break;
            default:
                break;
        }
        str_close_conv (conv);
    }

    return ch;
}

GString* CodepageDesc::str_nconvert_to_input (const char *str, int len)
{
    GString *buff;
    GIConv conv;

    if (!str)
        return g_string_new ("");

    if (cp_display == cp_source)
        return g_string_new (str);

    conv = str_crt_conv_to (cp_source);

    buff = g_string_new ("");
    str_nconvert (conv, str, len, buff);
    str_close_conv (conv);
    return buff;
}

GString* CodepageDesc::str_convert_to_input (const char *str)
{
    return str_nconvert_to_input (str, -1);
}

GString* CodepageDesc::str_nconvert_to_display (const char *str, int len)
{
    if (!str)
        return g_string_new ("");

    if (CodepageDesc::cp_display == CodepageDesc::cp_source)
        return g_string_new (str);

    GIConv conv = str_crt_conv_from (CodepageDesc::cp_source);

    GString *buff = g_string_new ("");
    str_nconvert (conv, str, len, buff);
    str_close_conv (conv);
    return buff;
}

GString* CodepageDesc::str_convert_to_display (const char *str)
{
    return str_nconvert_to_display (str, -1);
}

gboolean CodepageDesc::is_supported_encoding (const char *encoding)
{
    gboolean result = FALSE;

    for (size_t t = 0; t < codepages->len; t++)
    {
        const char *id = (static_cast<CodepageDesc*>(g_ptr_array_index (codepages, t)))->GetId();
        result |= (g_ascii_strncasecmp (encoding, id, strlen (id)) == 0);
    }

    return result;
}

const char* CodepageDesc::get_codepage_id (int n)
{
    return (n < 0) ? OTHER_8BIT : (static_cast<CodepageDesc*>(g_ptr_array_index (codepages, n)))->GetId();
}

void CodepageDesc::free_codepages_list ()
{
    g_ptr_array_foreach (codepages, free_codepage_desc, NULL);
    g_ptr_array_free (codepages, TRUE);
    /* NULL-ize pointer to make unit tests happy */
    codepages = nullptr;
}

int CodepageDesc::get_codepage_index(const char *id)
{
    if (!codepages)
        return -1;

    if (strcmp (id, OTHER_8BIT) == 0)
        return -1;

    for (size_t i = 0; i < codepages->len; i++)
    {
        if (strcmp(id, (static_cast<CodepageDesc *>(g_ptr_array_index (codepages, i)))->GetId()) == 0)
            return i;
    }
    return -1;
}

void CodepageDesc::load_codepages_list()
{
    char *fname;

    /* 1: try load /usr/share/mc/mc.charsets */
    fname = g_build_filename (mc_global.share_data_dir, CHARSETS_LIST, (char *) NULL);
    load_codepages_list_from_file (&codepages, fname);
    g_free (fname);

    /* 2: try load /etc/mc/mc.charsets */
    fname = g_build_filename (mc_global.sysconfig_dir, CHARSETS_LIST, (char *) NULL);
    load_codepages_list_from_file (&codepages, fname);
    g_free (fname);

    if (!codepages)
    {
        /* files are not found, add default codepage */
        fprintf (stderr, "%s\n", _("Warning: cannot load codepages list"));

        codepages = g_ptr_array_new ();
        g_ptr_array_add (CodepageDesc::codepages, new CodepageDesc(DEFAULT_CHARSET, _("7-bit ASCII")));
    }
}

void CodepageDesc::convert_from_input (char *str)
{
    if (str != nullptr)
        for (; *str != '\0'; str++)
            *str = conv_input[(unsigned char) *str];
}

void CodepageDesc::convert_to_display (char *str)
{
    if (str != nullptr)
        for (; *str != '\0'; str++)
            *str = conv_displ[(unsigned char) *str];
}

char CodepageDesc::translate_character (GIConv cd, char c)
{
    gsize bytes_read, bytes_written = 0;
    const char *ibuf = &c;
    char ch = UNKNCHAR;
    int ibuflen = 1;

    gchar *tmp_buff = g_convert_with_iconv (ibuf, ibuflen, cd, &bytes_read, &bytes_written, NULL);
    if (tmp_buff != nullptr)
        ch = tmp_buff[0];
    g_free (tmp_buff);
    return ch;
}

char* CodepageDesc::init_translation_table (int cpsource, int cpdisplay)
{
    /* Fill inpit <-> display tables */

    if (cpsource < 0 || cpdisplay < 0 || cpsource == cpdisplay)
    {
        for (int i = 0; i <= 255; ++i)
        {
            conv_displ[i] = i;
            conv_input[i] = i;
        }
        cp_source = cp_display;
        return nullptr;
    }

    for (int i = 0; i <= 127; ++i)
    {
        conv_displ[i] = i;
        conv_input[i] = i;
    }
    cp_source = (static_cast<CodepageDesc*>(g_ptr_array_index (codepages, cpsource)))->GetId();
    cp_display = (static_cast<CodepageDesc*>(g_ptr_array_index (codepages, cpdisplay)))->GetId();

    /* display <- inpit table */

    GIConv cd = g_iconv_open (cp_display, cp_source);
    if (cd == INVALID_CONV)
        return g_strdup_printf (_("Cannot translate from %s to %s"), cp_source, cp_display);

    for (int i = 128; i <= 255; ++i)
        conv_displ[i] = translate_character (cd, i);

    g_iconv_close (cd);

    /* inpit <- display table */

    cd = g_iconv_open (cp_source, cp_display);
    if (cd == INVALID_CONV)
        return g_strdup_printf (_("Cannot translate from %s to %s"), cp_display, cp_source);

    for (int i = 128; i <= 255; ++i)
    {
        unsigned char ch;
        ch = translate_character (cd, i);
        conv_input[i] = (ch == UNKNCHAR) ? i : ch;
    }

    g_iconv_close (cd);

    return nullptr;
}
