/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999, 2002, 2003 Peter Miller;
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
 * MANIFEST: functions to manipulate find_fuzzys
 */

#include <change/file.h>
#include <error.h> /* for assert */
#include <symtab.h>
#include <trace.h>


fstate_src_ty *
change_file_find_fuzzy(change_ty *cp, string_ty *file_name)
{
    string_ty       *best_file_name;
    fstate_src_ty   *best;

    trace(("change_file_find_fuzzy(cp = %08lX, fn = \"%s\")\n{\n",
	(long)cp, file_name->str_text));
    change_fstate_get(cp);
    assert(cp->fstate_stp);
    best_file_name = symtab_query_fuzzy(cp->fstate_stp, file_name);
    if (!best_file_name)
	best = 0;
    else
	best = symtab_query(cp->fstate_stp, best_file_name);
    trace(("return %08lX;\n", (long)best));
    trace(("}\n"));
    return best;
}
