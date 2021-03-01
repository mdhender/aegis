//
//	aegis - project change supervisor
//	Copyright (C) 1997, 2002-2008 Peter Miller
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

#include <common/now.h>
#include <libaegis/aer/value/time.h>

#include <aefind/tree.h>
#include <aefind/tree/constant.h>
#include <aefind/tree/now.h>


tree::pointer
tree_now_new(void)
{
    static rpt_value::pointer vp;

    if (!vp)
	vp = rpt_value_time::create(now());
    return tree_constant::create(vp);
}
