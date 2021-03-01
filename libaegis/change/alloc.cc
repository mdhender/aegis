//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2000, 2002-2008 Peter Miller
//	Copyright (C) 2007, 2008 Walter Franzini
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

#include <libaegis/change.h>
#include <common/error.h> // for assert
#include <common/mem.h>
#include <libaegis/project.h>
#include <common/trace.h>


change::pointer
change_alloc(project_ty *pp, long number)
{
    change::pointer cp;
    size_t          n;

    trace(("change_alloc(pp = %08lX, number = %ld)\n{\n", (long)pp,
	number));
    assert(number >= 1 || number == MAGIC_ZERO);
    cp = (change::pointer)mem_alloc(sizeof(change));
    cp->reference_count = 1;
    cp->pp = project_copy(pp);
    cp->number = number;

    cp->cstate_data = 0;
    cp->cstate_filename = 0;
    cp->cstate_is_a_new_file = 0;
    cp->fstate_data = 0;
    cp->fstate_stp = 0;
    cp->fstate_uuid_stp = 0;
    cp->fstate_filename = 0;
    cp->pfstate_data = 0;
    cp->pfstate_stp = 0;
    cp->pfstate_uuid_stp = 0;
    cp->pfstate_filename = 0;
    cp->fstate_is_a_new_file = 0;

    cp->architecture_name = 0;
    cp->bogus = 0;
    cp->top_path_unresolved = 0;
    cp->top_path_resolved = 0;
    cp->development_directory_unresolved = 0;
    cp->development_directory_resolved = 0;
    cp->integration_directory_unresolved = 0;
    cp->integration_directory_resolved = 0;
    cp->lock_magic = 0;
    cp->logfile = 0;
    cp->pconf_data = 0;
    cp->project_specific_setenv_performed = false;
    for (n = 0; n < SIZEOF(cp->file_list); ++n)
            cp->file_list[n] = 0;
    trace(("return %08lX;\n", (long)cp));
    trace(("}\n"));
    return cp;
}
