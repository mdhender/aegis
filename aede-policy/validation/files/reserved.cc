//
// aegis - project change supervisor
// Copyright (C) 2009, 2010, 2012 Peter Miller
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

#include <common/ac/assert.h>
#include <common/ac/ctype.h>
#include <common/ac/string.h>

#include <common/nstring/accumulator.h>
#include <libaegis/change/file.h>
#include <libaegis/input/crlf.h>
#include <libaegis/input/file_text.h>
#include <libaegis/os.h>
#include <libaegis/sub.h>

#include <aede-policy/validation/files/reserved.h>


validation_files_reserved::~validation_files_reserved()
{
}


validation_files_reserved::validation_files_reserved()
{
    exceptions.push_back("__attribute__");
    exceptions.push_back("__bool__");
    exceptions.push_back("__bool");
    exceptions.push_back("__DATE__");
    exceptions.push_back("__FILE__");
    exceptions.push_back("__func__");
    exceptions.push_back("__FUNC__");
    exceptions.push_back("__function__");
    exceptions.push_back("__FUNCTION__");
    exceptions.push_back("__inline__");
    exceptions.push_back("__inline");
    exceptions.push_back("__LINE__");
    exceptions.push_back("__PRETTY_FUNCTION__");
    exceptions.push_back("__TIME__");
    exceptions.push_back("__TIMESTAMP__");
}


validation::pointer
validation_files_reserved::create(void)
{
    return pointer(new validation_files_reserved());
}


bool
validation_files_reserved::check_branches()
    const
{
    return false;
}


bool
validation_files_reserved::check_downloaded()
    const
{
    return false;
}


bool
validation_files_reserved::check_foreign_copyright()
    const
{
    return false;
}


bool
validation_files_reserved::check_binaries()
    const
{
    return false;
}


bool
validation_files_reserved::is_a_reserved_word(const nstring &name)
    const
{
#if 0
    // These are just plain fugly.
    if (name.size() >= 1 && (name[0] == '_' || name[name.size() - 1] == '_')
        return true;
#endif

    //
    // The symbols defined by the standard (or in common use) are OK to
    // be used.  They aren't OK to be redefined, but that takes more
    // semantic analysis, and this is just a heuristic.
    //
    if (exceptions.member(name))
        return false;

    //
    // ANSI C++ Standard, Section 2.10, Paragraph 2:
    // "Identifiers containing a double underscore (__) or beginning
    // with an underscore and an upper-case letter are reserved for use
    // by C++ implementations and standard libraries and shall not be
    // used otherwise."
    //
    if (strstr(name.c_str(), "__"))
        return true;
    if (name.size() >= 2 && name[0] == '_' && isupper((unsigned char)name[1]))
        return true;

    //
    // ANSI C Standard, Section 2.10, Paragraph 1:
    // "In addition, some identifiers are reserved for use by C++
    // implementations and standard libraries and shall not be used
    // otherwise; no diagnostic is required."
    //

    //
    // Can't find anything to complain about.
    //
    return false;
}


static nstring
get_character(input ip)
{
    nstring_accumulator ac;
    int c = ip->getch();
    if (c < 0)
        return ac.mkstr();
    ac.push_back((char)c);
    if (c != '\\')
        return ac.mkstr();
    c = ip->getch();
    if (c < 0)
        return ac.mkstr();
    ac.push_back((char)c);
    switch (c)
    {
    default:
        return ac.mkstr();

    case '0': case '1': case '2': case '3':
    case '4': case '5': case '6': case '7':
        for (;;)
        {
            c = ip->getch();
            if (c < 0)
                return ac.mkstr();
            switch ((char)c)
            {
            case '0': case '1': case '2': case '3':
            case '4': case '5': case '6': case '7':
                continue;

            default:
                ip->ungetc((char)c);
                return ac.mkstr();
            }
        }

    case 'x':
    case 'X':
        for (;;)
        {
            c = ip->getch();
            if (c < 0)
                return ac.mkstr();
            switch ((char)c)
            {
            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9':
            case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
            case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
                continue;

            default:
                ip->ungetc((char)c);
                return ac.mkstr();
            }
        }
    }
}


bool
validation_files_reserved::check(change::pointer cp, fstate_src_ty *src)
{
    nstring path(cp->file_path(src));
    assert(!path.empty());
    if (path.empty())
        return true;

    //
    // figure out what sort of file we are looking at
    //
    nstring base = path.basename();
    nstring lc_base = base.downcase();
    bool isa_c_file = base.ends_with(".c");
    bool isa_cxx_file =
        base.ends_with(".C") ||
        lc_base.ends_with(".c++") ||
        lc_base.ends_with(".cc") ||
        lc_base.ends_with(".cpp") ||
        base.ends_with(".H") ||
        lc_base.ends_with(".h++") ||
        lc_base.ends_with(".hh") ||
        lc_base.ends_with(".hpp")
        ;
    bool isa_h_file = lc_base.ends_with(".h");
    if (!isa_c_file && !isa_cxx_file && !isa_h_file)
        return true;

    os_become_orig();
    bool ok = true;
    input ip = input_file_text_open(path);
    for (;;)
    {
        int ic = ip->getch();
        if (ic < 0)
            break;
        unsigned char c = ic;
        switch (c)
        {
        case ' ':
        case '\b':
        case '\f':
        case '\n':
        case '\r':
        case '\t':
        case '\v':
            continue;

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
        case '$': case '_':
            // Identifier (or pre-processor phase number)
            {
                nstring_accumulator ac;
                for (;;)
                {
                    ac.push_back(c);
                    ic = ip->getch();
                    if (ic < 0)
                        break;
                    c = ic;
                    switch (c)
                    {
                    case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
                    case 'G': case 'H': case 'I': case 'J': case 'K': case 'L':
                    case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R':
                    case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
                    case 'Y': case 'Z':
                    case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
                    case 'g': case 'h': case 'i': case 'j': case 'k': case 'l':
                    case 'm': case 'n': case 'o': case 'p': case 'q': case 'r':
                    case 's': case 't': case 'u': case 'v': case 'w': case 'x':
                    case 'y': case 'z':
                    case '0': case '1': case '2': case '3': case '4':
                    case '5': case '6': case '7': case '8': case '9':
                    case '$': case '_':
                        continue;

                    default:
                        ip->ungetc(c);
                        break;
                    }
                    break;
                }
                nstring name = ac.mkstr();
                if (is_a_reserved_word(name))
                {
                    string_ty *text =
                        str_from_c
                        (
                            "identifier \"$name\" is reserved by the "
                            "ANSI C Standard"
                        );
                    sub_context_ty sc;
                    sc.var_set_long("Name", name);
                    string_ty *msg = sc.substitute(cp, text);
                    str_free(text);
                    ip->error(msg->str_text);
                    str_free(msg);
                    ok = false;
                }
            }
            break;

        case '/':
            ic = ip->getch();
            if (ic < 0)
                break;
            c = ic;
            switch (c)
            {
            default:
                ip->ungetc(c);
                break;

            case '/':
                // This is a C++ comment
                for (;;)
                {
                    ic = ip->getch();
                    if (ic < 0)
                        break;
                    if (c == '\n')
                        break;
                }
                break;

            case '*':
                // This is a C comment
                for (;;)
                {
                    ic = ip->getch();
                    if (ic < 0)
                        break;
                    c = ic;
                    if (c != '*')
                        continue;
                    for (;;)
                    {
                        ic = ip->getch();
                        if (ic < 0)
                            break;
                        c = ic;
                        if (c != '*')
                            break;
                    }
                    if (c != '/')
                        continue;
                    break;
                }
                break;
            }
            break;

        case '\'':
            // Character constant
            for (;;)
            {
                nstring ss = get_character(ip);
                if (ss == "'")
                    break;
            }
            break;

        case '"':
            // String constant
            for (;;)
            {
                nstring ss = get_character(ip);
                if (ss == "\"")
                    break;
            }
            break;
        }
    }

    ip.close();
    os_become_undo();
    return ok;
}


// vim: set ts=8 sw=4 et :
