//
// aegis - project change supervisor
// Copyright (C) 2012 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or (at
// your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <common/ac/ctype.h>
#include <common/ac/dirent.h>
#include <common/ac/stdio.h>
#include <common/ac/stdlib.h>
#include <common/ac/string.h>
#include <map>

#include <common/boolean.h>
#include <common/nstring/accumulator.h>
#include <common/nstring/list.h>
#include <common/trace.h>

#include <libaegis/input/file.h>
#include <libaegis/locale_name.h>
#include <libaegis/os.h>
#include <libaegis/xmltextread/by_node.h>
#include <libaegis/xml_node/iso_3166.h>
#include <libaegis/xml_node/iso_639_3.h>


static xml_node_iso_639_3 language;


static void
language_setup(void)
{
    if (language.need_setup())
    {
        trace(("%s\n{\n", __PRETTY_FUNCTION__));
        nstring path = "/usr/share/xml/iso-codes/iso_639_3.xml";
        os_become_orig();
        input ip = input_file_open(path);
        xml_text_reader_by_node reader(ip);
        reader.register_node_handler
        (
            "iso_639_3_entries/iso_639_3_entry",
            language
        );
        reader.parse();
        ip.close();
        os_become_undo();
        trace(("}\n"));
    }
}


bool
is_a_language_code(const nstring &text)
{
    language_setup();
    return language.is_a_valid_code(text);
}


static xml_node_iso_3166 country;


static void
country_setup(void)
{
    if (country.need_setup())
    {
        trace(("%s\n{\n", __PRETTY_FUNCTION__));
        nstring path = "/usr/share/xml/iso-codes/iso_3166.xml";
        os_become_orig();
        input ip = input_file_open(path);
        xml_text_reader_by_node reader(ip);
        reader.register_node_handler
        (
            "iso_3166_entries/iso_3166_entry",
            country
        );
        reader.parse();
        ip.close();
        os_become_undo();
        trace(("}\n"));
    }
}


bool
is_a_country_code(const nstring &text)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    country_setup();
    return country.is_a_valid_code(text);
}


static bool
is_a_valid_charset_name(const nstring &text)
{
    //
    // This is according to eglibc's "iconv/gconv_charset.h" source
    // file, but without the "//something" suffix support, since they
    // never appear in file names).
    //
    const char *cp = text.c_str();
    if (!*cp)
        return false;
    for (;;)
    {
        unsigned char c = *cp++;
        switch (c)
        {
        case '\0':
            return true;

        case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G':
        case 'H': case 'I': case 'J': case 'K': case 'L': case 'M': case 'N':
        case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U':
        case 'V': case 'W': case 'X': case 'Y': case 'Z':
        case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g':
        case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n':
        case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u':
        case 'v': case 'w': case 'x': case 'y': case 'z':
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
        case '_': case '-': case '.': case ',': case ':':
            break;

        default:
            return false;
        }
    }
}


static bool
is_probably_a_qualifier_name(const nstring &text)
{
    // the only actual example I can find is "@euro"
    // so extrapolate and allow simple words.
    const char *cp = text.c_str();
    if (!*cp)
        return false;
    for (;;)
    {
        unsigned char c = *cp++;
        switch (c)
        {
        case '\0':
            return true;

        case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G':
        case 'H': case 'I': case 'J': case 'K': case 'L': case 'M': case 'N':
        case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U':
        case 'V': case 'W': case 'X': case 'Y': case 'Z':
        case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g':
        case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n':
        case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u':
        case 'v': case 'w': case 'x': case 'y': case 'z':
            break;

        default:
            return false;
        }
    }
}


typedef std::map<nstring, bool> locales_t;
static locales_t locales;


bool
is_a_locale_name(const nstring &text)
{
    trace(("is_a_locale_name(text = %s)\n{\n", text.quote_c().c_str()));

    //
    // Look for the given locale name in the locales table.
    // It contains both hits and misses.
    //
    locales_t::const_iterator it = locales.find(text);
    if (it != locales.end())
        return it->second;

    //
    // Pull the text apart, looking for langauges and countries and
    // charsets and qualifiers.
    //
    //     {language} [ '_' {country} ][ '.' {charset} ][ '@' {qualifier} ]
    //
    // If only things were that simple.
    //
    nstring language_part;
    nstring country_part;
    nstring charset_part;
    nstring qualifier_part;
    {
        const char *cp = text.c_str();

        //
        // Work out how much of the first part of the string is the
        // language code.
        //
        // The implementation can't be clean and tidy, because some
        // language entries aren't simple 2-letter or 3-letter codes,
        // and they could even contain random punctuation characters.
        //
        {
            const char *sep = strpbrk(cp, "_.@");
            if (!sep)
                sep = cp + strlen(cp);
            for (;;)
            {
                nstring key(cp, sep - cp);
                if (is_a_language_code(key))
                {
                    language_part = key;
                    cp = sep;
                    break;
                }
                if (!*sep)
                    goto definite_negative;
                ++sep;
                const char *sep2 = strpbrk(sep, "_.@");
                sep = sep2 ? sep2 : sep + strlen(sep);
            }
        }

        //
        // Work out how much of the next part of the string is the
        // country code.
        //
        // The implementation can't be clean and tidy, because some
        // country entries aren't simple 2-letter or 3-letter codes, and
        // they could even contain random punctuation characters.
        //
        if (*cp == '_')
        {
            ++cp;
            const char *sep = strpbrk(cp, ".@");
            if (!sep)
                sep = cp + strlen(cp);
            for (;;)
            {
                nstring key(cp, sep - cp);
                if (is_a_country_code(key))
                {
                    country_part = "_" + key;
                    cp = sep;
                    break;
                }
                if (!*sep)
                    goto definite_negative;
                ++sep;
                const char *sep2 = strpbrk(sep, ".@");
                sep = sep2 ? sep2 : sep + strlen(sep);
            }
        }

        //
        // Work out how much of the next part of the string is the
        // charset name.
        //
        // Unlike language code and country code, there is no data file
        // we can use to validate charset names.
        //
        // If gnu gettext is to be believed (care of some rather
        // excessive inlining of data files) all charset names match
        // regex /[A-Za-z][-A-Za-z0-9]*/ except, of course, they don't.
        //
        if (*cp == '.')
        {
            ++cp;
            const char *sep = strchr(cp, '@');
            if (!sep)
                sep = cp + strlen(cp);

            nstring key(cp, sep - cp);
            if (!is_a_valid_charset_name(key))
                goto definite_negative;
            charset_part = "." + key;
            cp = sep;
        }

        //
        // Work out how much of the next part of the string is the
        // qualifier.
        //
        // Unlike language code and country code, there is no data file
        // we can use to validate a qualifier.
        //
        // If gnu gettext is to be believed (care of some rather
        // excessive inlining of data files) the only qualifier is
        // "@euro" or nothing.  As if!
        //
        if (*cp == '@')
        {
            ++cp;
            nstring key(cp);
            if (!is_probably_a_qualifier_name(key))
                goto definite_negative;
            qualifier_part = "@" + key;
            cp += key.size();
        }

        // there should be nothing left over
        if (*cp)
            goto definite_negative;
    }

    // cache the hit
    locales[text] = true;
    locales[language_part+country_part+charset_part+qualifier_part] = true;
    trace(("return true;\n"));
    trace(("}\n"));
    return true;

    definite_negative:
    // cache the miss
    trace(("cache miss %s\n", text.quote_c().c_str()));
    locales[text] = false;
    trace(("return false;\n"));
    trace(("}\n"));
    return false;
}


// vim: set ts=8 sw=4 et :
