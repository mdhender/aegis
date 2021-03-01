/*
 *	aegis - project change supervisor
 *	Copyright (C) 1997 Peter Miller;
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
 * MANIFEST: functions to manipulate print nodes
 */

#include <function/print.h>
#include <shorthand/print.h>
#include <tree.h>
#include <tree/list.h>
#include <tree/this.h>


tree_ty *
shorthand_print()
{
	tree_ty		*result;
	tree_ty		*tp2;
	tree_list_ty	*tlp;

	tp2 = tree_this_new();
	tlp = tree_list_new();
	tree_list_append(tlp, tp2);
	tree_delete(tp2);
	result = function_print(tlp);
	tree_list_delete(tlp);
	return result;
}
