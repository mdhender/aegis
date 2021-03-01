//
//	aegis - project change supervisor
//	Copyright (C) 1996, 2002-2008 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//

#ifndef AEGIS_AER_POS_H
#define AEGIS_AER_POS_H

#include <common/ac/shared_ptr.h>
#include <common/nstring.h>

struct sub_context_ty; // existence

class sub_context_ty; // forward

/**
  * The rpt_position class is used to represent a source file location,
  * by name and line number.
  */
class rpt_position
{
public:
    typedef aegis_shared_ptr<rpt_position> pointer;

    virtual ~rpt_position();

private:
    rpt_position(const nstring &file_name, long linum1, long linum2);

public:
    static rpt_position::pointer create(const nstring &file_name);

    static rpt_position::pointer create(const nstring &file_name, long linum);

    static rpt_position::pointer create(const nstring &file_name, long linum1,
        long linum2);

    static rpt_position::pointer join(const rpt_position::pointer &lhs,
        const rpt_position::pointer &rhs);

    void print_error(sub_context_ty &sc, const char *text) const;

    nstring get_file_name() const { return file_name; }

    long get_line_number() const { return line_number[0]; }

    nstring representation() const;

private:
    nstring file_name;
    long line_number[2];
};

#endif // AEGIS_AER_POS_H
