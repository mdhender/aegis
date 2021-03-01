//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2000, 2002, 2004 Peter Miller;
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
// MANIFEST: functions to manipulate frees
//

#include <change.h>
#include <error.h> // for assert
#include <mem.h>
#include <project.h>
#include <str_list.h>
#include <symtab.h>
#include <trace.h>


void
change_free(change_ty *cp)
{
    trace(("change_free(cp = %08lX)\n{\n", (long)cp));
    assert(cp->reference_count >= 1);
    cp->reference_count--;
    if (cp->reference_count <= 0)
    {
	size_t          j;

	assert(cp->pp);
	project_free(cp->pp);
	if (cp->cstate_filename)
		str_free(cp->cstate_filename);
	if (cp->fstate_filename)
		str_free(cp->fstate_filename);
	if (cp->cstate_data)
		cstate_type.free(cp->cstate_data);
	if (cp->fstate_data)
		fstate_type.free(cp->fstate_data);
	if (cp->fstate_stp)
		symtab_free(cp->fstate_stp);
	if (cp->top_path_unresolved)
		str_free(cp->top_path_unresolved);
	if (cp->top_path_resolved)
		str_free(cp->top_path_resolved);
	if (cp->development_directory_unresolved)
		str_free(cp->development_directory_unresolved);
	if (cp->development_directory_resolved)
		str_free(cp->development_directory_resolved);
	if (cp->integration_directory_unresolved)
		str_free(cp->integration_directory_unresolved);
	if (cp->integration_directory_resolved)
		str_free(cp->integration_directory_resolved);
	if (cp->logfile)
		str_free(cp->logfile);
	if (cp->pconf_data)
		pconf_type.free(cp->pconf_data);
	for (j = 0; j < view_path_MAX; ++j)
	{
	    if (cp->file_list[j])
	    {
		string_list_delete(cp->file_list[j]);
		cp->file_list[j] = 0;
	    }
	}
	mem_free((char *)cp);
    }
    trace(("}\n"));
}
