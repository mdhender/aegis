//
// aegis - project change supervisor
// Copyright (C) 1999, 2003-2006, 2008, 2011, 2012 Peter Miller
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

#include <common/mem.h>
#include <libaegis/output/to_wide.h>
#include <libaegis/output/wrap.h>
#include <libaegis/wide_output/expand.h>
#include <libaegis/wide_output/to_narrow.h>
#include <libaegis/wide_output/wrap.h>


output::pointer
output_wrap_open(const output::pointer &deeper, int width)
{
    wide_output::pointer fp1 = wide_output_to_narrow::open(deeper);
    if (width <= 0)
        width = fp1->page_width();
    wide_output::pointer fp2 = wide_output_wrap::open(fp1, width);
    wide_output::pointer fp3 = wide_output_expand::open(fp2);
    output::pointer fp4 = output_to_wide::open(fp3);
    return fp4;
}


// vim: set ts=8 sw=4 et :
