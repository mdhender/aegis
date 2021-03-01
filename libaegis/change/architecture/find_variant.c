/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999 Peter Miller;
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
 * MANIFEST: functions to manipulate find_variants
 */

#include <change/architecture/find_variant.h>
#include <error.h> /* for assert */
#include <trace.h>
#include <uname.h>


cstate_architecture_times
change_find_architecture_variant(cp)
	change_ty	*cp;
{
	string_ty	*an;
	cstate_architecture_times tp;

	/*
	 * find the name of the architecture variant
	 *	one of the patterns, not the actual value in architecture
	 */
	trace(("find_architecture_variant(cp = %8.8lX)\n{\n"/*}*/, (long)cp));
	assert(cp->reference_count >= 1);
	an = change_architecture_name(cp, 1);

	/*
	 * find this variant in the times list
	 */
	tp = change_architecture_times_find(cp, an);

	/*
	 * adjust the node
	 */
	str_free(tp->node);
	tp->node = str_copy(uname_node_get());
	trace(("return %8.8lX;\n", (long)tp));
	trace((/*{*/"}\n"));
	return tp;
}
