//
//	aegis - project change supervisor
//	Copyright (C) 1997, 2002, 2004, 2005 Peter Miller;
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
// MANIFEST: functions to manipulate path tree nodes
//

#include <libaegis/aer/value/string.h>
#include <aefind/shorthand/path.h>
#include <common/str.h>
#include <aefind/tree.h>
#include <aefind/tree/constant.h>
#include <aefind/tree/match.h>
#include <aefind/tree/this.h>


tree_ty *
shorthand_path(string_ty *pattern)
{
    rpt_value_ty    *vp;
    tree_ty	    *tp1;
    tree_ty	    *tp2;
    tree_ty	    *result;

    vp = rpt_value_string(pattern);
    tp1 = tree_constant_new(vp);
    rpt_value_free(vp);

    tp2 = tree_this_new();

    result = tree_match_new(tp1, tp2);
    tree_delete(tp1);
    tree_delete(tp2);

    return result;
}
