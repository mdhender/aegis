//
// aegis - project change supervisor
// Copyright (C) 1999, 2002-2006, 2008, 2010, 2012 Peter Miller
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
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <common/trace.h>
#include <libaegis/input/crlf.h>
#include <libaegis/sub.h>


input_crlf::~input_crlf()
{
    trace(("input_crlf::~input_crlf(this = %p)\n{\n", this));
    pullback_transfer(deeper);
    deeper.close();
    trace(("}\n"));
}


input_crlf::input_crlf(input &arg1, bool arg2) :
    deeper(arg1),
    pos(0),
    line_number(0),
    prev_was_newline(true),
    newlines_may_be_escaped(arg2)
{
    trace(("input_crlf::input_crlf(this = %p)\n{\n", this));
    trace(("}\n"));
}


input
input_crlf::create(input &arg1, bool arg2)
{
    return input(new input_crlf(arg1, arg2));
}


ssize_t
input_crlf::read_inner(void *data, size_t len)
{
    trace(("input_crlf::read_inner(this = %p)\n{\n", this));
    if (prev_was_newline)
    {
        ++line_number;
        prev_was_newline = false;

        //
        // The name cache includes the line number, and the line
        // number just changed, so nuke it.
        //
        name_cache.clear();
    }

    unsigned char *cp = (unsigned char *)data;
    unsigned char *end = cp + len;
    while (cp < end)
    {
        int c = deeper->getch();
        switch (c)
        {
        case '\r':
            c = deeper->getch();
            if (c == '\n')
                goto newline;
            if (c >= 0)
                deeper->ungetc(c);
#ifdef __mac_os_x__
            goto newline;
#else
            *cp++ = '\r';
            continue;
#endif

        case '\\':
            if (newlines_may_be_escaped)
            {
                c = deeper->getch();
                if (c == '\n')
                {
                    //
                    // Don't put the newline in the
                    // buffer, but DO stop here,
                    // so that the line numbers
                    // are right.
                    //
                    prev_was_newline = 1;
                    break;
                }
                if (c >= 0)
                    deeper->ungetc(c);
            }
            *cp++ = '\\';
            continue;

        case -1:
            break;

        case 0:
            //
            // For plain ASCII text, the conditions reads
            //
            //  if (!isprint((unsigned char)c) &&
            //          !isspace((unsigned char)c))
            //
            // However, for international text, just about
            // anything is acceptable.  But not NUL.
            //
            {
                sub_context_ty sc;
                sc.var_set_format("Name", "\\%o", c);
                string_ty *s = sc.subst_intl(i18n("illegal '$name' character"));
                fatal_error(s->str_text);
            }
            continue;

        default:
            //
            // The default should be enough, but these are
            // to force the use of a lookup table instead of
            // an if-then-else chain in the code generated for
            // the switch.
            //
        case '!': case '"': case '#': case '$': case '%': case '&':
        case '\'': case '(': case ')': case '*': case '+': case ',':
        case '-': case '.': case '/': case '0': case '1': case '2':
        case '3': case '4': case '5': case '6': case '7': case '8':
        case '9': case ':': case ';': case '<': case '=': case '>':
        case '?': case '@': case 'A': case 'B': case 'C': case 'D':
        case 'E': case 'F': case 'G': case 'H': case 'I': case 'J':
        case 'K': case 'L': case 'M': case 'N': case 'O': case 'P':
        case 'Q': case 'R': case 'S': case 'T': case 'U': case 'V':
        case 'W': case 'X': case 'Y': case 'Z': case '[': case ']':
        case '^': case '_': case '`': case 'a': case 'b': case 'c':
        case 'd': case 'e': case 'f': case 'g': case 'h': case 'i':
        case 'j': case 'k': case 'l': case 'm': case 'n': case 'o':
        case 'p': case 'q': case 'r': case 's': case 't': case 'u':
        case 'v': case 'w': case 'x': case 'y': case 'z': case '{':
        case '|': case '}': case '~':
            *cp++ = c;
            continue;

        case '\n':
            //
            // We are line buffered.  This is the best way to
            // allow us to report line numbers accurately.
            // It results is some inefficiencies, but it also
            // lets us get rid of a whole layer of function
            // calls in the lexer.
            //
            newline:
            *cp++ = '\n';
            prev_was_newline = 1;
            break;
        }
        break;
    }

    //
    // Figure what happened.
    //
    size_t nbytes = (cp - (unsigned char *)data);
    pos += nbytes;
    trace(("return %ld;\n", (long)nbytes));
    trace(("}\n"));
    return nbytes;
}


off_t
input_crlf::ftell_inner()
{
    trace(("input_crlf_ftell(this = %p) => " OFF_T_FMT "\n", this, pos));
    return pos;
}


nstring
input_crlf::name()
{
    trace(("input_crlf_name(this = %p)\n", this));
    if (!line_number)
        return deeper->name();
    if (!name_cache)
    {
        name_cache =
            nstring::format("%s: %ld", deeper->name().c_str(), line_number);
    }
    return name_cache;
}


off_t
input_crlf::length()
{
    trace(("input_crlf_length(this = %p) => -1\n", this));
    return -1;
}


void
input_crlf::keepalive()
{
    deeper->keepalive();
}


bool
input_crlf::is_remote()
    const
{
    return deeper->is_remote();
}


// vim: set ts=8 sw=4 et :
