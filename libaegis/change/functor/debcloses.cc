//
// aegis - project change supervisor
// Copyright (C) 2009, 2010 Peter Miller
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
#include <common/ac/stdlib.h>

#include <libaegis/change/functor/debcloses.h>


change_functor_debcloses::~change_functor_debcloses()
{
}


change_functor_debcloses::change_functor_debcloses() :
    change_functor(false, true),
    closes("(closes|debian):?[:space:]*", true),
    bug("bug[:space:]*", true),
    hash("#[:space:]*")
{
    //
    // According to
    // http://www.debian.org/doc/debian-policy/ch-source.html#s-dpkgchangelog
    // the regex is Perl...
    //
    //       /closes:\s*(?:bug)?\#?\s?\d+(?:,\s*(?:bug)?\#?\s?\d+)*/i
    //
    // So we search for /closes/ and the do the rest "by hand"
    //
}



void
change_functor_debcloses::grope(const nstring &caption, const nstring &text)
{
    const char *cp = text.c_str();
    for (;;)
    {
        if (!*cp)
            break;
        size_t ibegin = 0;
        size_t iend = 0;
        if (!closes.matches(cp, ibegin, iend))
            break;
        cp += iend;
        for (;;)
        {
            if (!*cp)
                break;
            if (*cp == ',' || isspace((unsigned char)*cp))
            {
                ++cp;
                continue;
            }
            if (bug.matches(cp, ibegin, iend) && ibegin == 0)
                cp += iend;
            if (hash.matches(cp, ibegin, iend) && ibegin == 0)
                cp += iend;
            // should be looking at a number
            char *ep = 0;
            long n = strtol(cp, &ep, 10);
            if (cp == ep)
                break;
            cp = ep;

            // Normalize the result.
            nstring s = nstring::format("%s: Closes: #%ld", caption.c_str(), n);
            result.push_back_unique(s);
        }
    }
}


void
change_functor_debcloses::operator()(change::pointer cp)
{
    grope(cp->version_get(), cp->description_get());
    grope(cp->version_get(), cp->brief_description_get());

    if (cp->attributes_get_boolean("aemakegen:debian:accepted"))
        result.clear();
}


nstring_list
change_functor_debcloses::get_result(void)
    const
{
    nstring_list answer(result);
    if (!answer.empty())
    {
        answer.push_back
        (
            "This is a summary, see earlier changelog entries "
            "for details of individual bug fixes."
        );
    }
    return answer;
}
