/*
 *	aegis - project change supervisor
 *	Copyright (C) 1997, 2002 Peter Miller;
 *	All rights reserved.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * MANIFEST: functions to manipulate name tree nodes
 */

#include <aer/value/string.h>
#include <function/basename.h>
#include <shorthand/name.h>
#include <str.h>
#include <tree.h>
#include <tree/constant.h>
#include <tree/list.h>
#include <tree/match.h>
#include <tree/this.h>


tree_ty *
shorthand_name(string_ty *pattern)
{
    rpt_value_ty    *vp;
    tree_ty	    *tp1;
    tree_ty	    *tp2;
    tree_list_ty    *tlp;
    tree_ty	    *result;

    vp = rpt_value_string(pattern);
    tp1 = tree_constant_new(vp);
    rpt_value_free(vp);

    tlp = tree_list_new();
    tp2 = tree_this_new();
    tree_list_append(tlp, tp2);
    tree_delete(tp2);
    tp2 = function_basename(tlp);
    tree_list_delete(tlp);

    result = tree_match_new(tp1, tp2);
    tree_delete(tp1);
    tree_delete(tp2);

    return result;
}
