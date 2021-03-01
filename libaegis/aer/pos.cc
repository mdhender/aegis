//
// aegis - project change supervisor
// Copyright (C) 1996, 1999, 2003-2008, 2012 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published
// by the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <common/ac/assert.h>
#include <common/ac/string.h>

#include <libaegis/aer/pos.h>
#include <libaegis/sub.h>


rpt_position::~rpt_position()
{
}


rpt_position::rpt_position(const nstring &a1, long a2, long a3) :
    file_name(a1)
{
    line_number[0] = a2;
    line_number[1] = a3;
}


rpt_position::pointer
rpt_position::create(const nstring &a_file_name)
{
    return create(a_file_name, 0);
}


rpt_position::pointer
rpt_position::create(const nstring &a1, long a2, long a3)
{
    return pointer(new rpt_position(a1, a2, a3));
}


rpt_position::pointer
rpt_position::create(const nstring &a_file_name, long a_line_number)
{
    const char *s = a_file_name.c_str();
    const char *colon = strstr(s, ": ");
    if (colon)
    {
        assert(a_line_number == 0);
        nstring fn = nstring(s, colon - s);
        int ln = atoi(colon + 2);
        return create(fn, ln, ln);
    }

    return create(a_file_name, a_line_number, a_line_number);
}


nstring
rpt_position::representation()
    const
{
    return nstring::format("%s: %ld", file_name.c_str(), line_number[0]);
}


rpt_position::pointer
rpt_position::join(const rpt_position::pointer &p1,
    const rpt_position::pointer &p2)
{
    assert(p1);
    assert(p2);
    if (p1->get_file_name() != p2->get_file_name())
        return p1;
    long minlin = p1->line_number[0];
    long maxlin = p1->line_number[1];
    if (minlin > p2->line_number[0])
        minlin = p2->line_number[0];
    if (maxlin < p2->line_number[1])
        maxlin = p2->line_number[1];
    if (p1->line_number[0] == minlin && p1->line_number[1] == maxlin)
        return p1;
    if (p2->line_number[0] == minlin && p2->line_number[1] == maxlin)
        return p2;

    return create(p1->get_file_name(), minlin, maxlin);
}


void
rpt_position::print_error(sub_context_ty &sc, const char *fmt)
    const
{
    string_ty *s = sc.subst_intl(fmt);

    // re-use substitution context
    sc.var_set_string("MeSsaGe", s);
    str_free(s);

    if (line_number[0])
    {
        sc.var_set_format
        (
            "File_Name",
            "%s: %ld",
            file_name.c_str(),
            line_number[0]
        );
    }
    else
        sc.var_set_string("File_Name", file_name);
    sc.error_intl(i18n("$filename: $message"));
}


// vim: set ts=8 sw=4 et :
