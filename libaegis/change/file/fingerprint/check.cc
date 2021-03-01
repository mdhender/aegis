//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2003-2008 Peter Miller
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

#include <libaegis/change/file.h>
#include <common/error.h> // for assert


void
change_file_fingerprint_check(change::pointer cp, fstate_src_ty *src_data)
{
    string_ty       *path;
    int             same;
    cstate_ty       *cstate_data;

    //
    // Only useful in the 'being developed' and 'being integrated'
    // states.
    //
    path = change_file_path(cp, src_data->file_name);
    assert(path);
    if (!src_data->file_fp)
    {
	src_data->file_fp = (fingerprint_ty *)fingerprint_type.alloc();
    }
    assert(src_data->file_fp->youngest >= 0);
    assert(src_data->file_fp->oldest >= 0);
    change_become(cp);
    same = change_fingerprint_same(src_data->file_fp, path, 0);
    change_become_undo(cp);
    assert(src_data->file_fp->youngest > 0);
    assert(src_data->file_fp->oldest > 0);

    //
    // If the file is unchanged, do nothing more.
    //
    if (same)
	return;

    //
    // Nuke the change's difference time.
    //
    cstate_data = cp->cstate_get();
    if (cstate_data->state != cstate_state_being_integrated)
    {
	if (src_data->diff_file_fp)
	{
    	    fingerprint_type.free(src_data->diff_file_fp);
    	    src_data->diff_file_fp = 0;
	}
    }
    else
    {
#if 0
	if (src_data->idiff_file_fp)
	{
    	    fingerprint_type.free(src_data->idiff_file_fp);
    	    src_data->idiff_file_fp = 0;
	}
#endif
    }

    //
    // Nuke the change's UUID.
    //
    // If the file manifest of the change is altered (e.g. by aenf, aenfu,
    // aecp, aecpu, etc), or the contents of any file is changed, the
    // UUID is cleared.  This is because it is no longer the same change
    // as was received by aedist or aepatch, and the UUID is invalidated.
    //
    change_uuid_clear(cp);

    //
    // Nuke the file's test times.
    //
    if (src_data->architecture_times)
    {
	fstate_src_architecture_times_list_type.free
	(
    	    src_data->architecture_times
	);
	src_data->architecture_times = 0;
    }
}
