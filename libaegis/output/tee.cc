//
//	aegis - project change supervisor
//	Copyright (C) 2001, 2003-2005 Peter Miller;
//	All rights reserved.
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: functions to manipulate tees
//

#include <libaegis/output/tee.h>


output_tee_ty::~output_tee_ty()
{
    //
    // Make sure all buffered data has been passed to our write_inner
    // method.
    //
    flush();

    if (d1_close)
	delete d1;
    d1 = 0;
    if (d2_close)
	delete d2;
    d2 = 0;
}


output_tee_ty::output_tee_ty(output_ty *arg1, bool arg2, output_ty *arg3,
	bool arg4) :
    d1(arg1),
    d1_close(arg2),
    d2(arg3),
    d2_close(arg4)
{
}


void
output_tee_ty::write_inner(const void *data, size_t nbytes)
{
    d1->write(data, nbytes);
    d2->write(data, nbytes);
}


void
output_tee_ty::flush_inner()
{
    d1->flush();
    d2->flush();
}


string_ty *
output_tee_ty::filename()
    const
{
    return d1->filename();
}


long
output_tee_ty::ftell_inner()
    const
{
    long result = d1->ftell();
    if (result < 0)
	    result = d2->ftell();
    return result;
}


int
output_tee_ty::page_width()
    const
{
    return d1->page_width();
}


int
output_tee_ty::page_length()
    const
{
    return d1->page_length();
}


void
output_tee_ty::end_of_line_inner()
{
    d1->end_of_line();
    d2->end_of_line();
}


const char *
output_tee_ty::type_name()
    const
{
    return "tee";
}
