//
//	aegis - project change supervisor
//	Copyright (C) 2007, 2008 Peter Miller
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

#include <common/error.h> // for assert
#include <common/nstring.h>
#include <libaegis/change/file.h>
#include <libaegis/input/file_text.h>
#include <libaegis/os.h>
#include <libaegis/sub.h>

#include <aede-policy/validation/files/merge-rcs.h>


validation_files_merge_rcs::~validation_files_merge_rcs()
{
}


validation_files_merge_rcs::validation_files_merge_rcs()
{
}


bool
validation_files_merge_rcs::check_binaries()
    const
{
    return false;
}


bool
validation_files_merge_rcs::check(change::pointer cp, fstate_src_ty *src)
{
    nstring bad_juju = "<<<<<<<";

    nstring path(change_file_path(cp, src));
    assert(!path.empty());
    if (path.empty())
	return true;

    os_become_orig();
    input ip = input_file_text_open(path);
    bool ok = true;
    for (;;)
    {
        nstring line;
        if (!ip->one_line(line))
            break;
        if (line.starts_with(bad_juju))
        {
            sub_context_ty sc;
            sc.var_set_string("File_Name", ip->name());
            change_error(cp, &sc, i18n("$filename: merge conflicts"));
	    ok = false;
            break;
        }
    }
    ip.close();
    os_become_undo();
    return ok;
}
