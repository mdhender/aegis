//
//	aegis - project change supervisor
//	Copyright (C) 2006-2008 Peter Miller
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
#include <libaegis/fstate.h>
#include <libaegis/input/file.h>
#include <libaegis/os.h>
#include <libaegis/sub.h>

#include <aede-policy/validation/files/text.h>


validation_files_text::~validation_files_text()
{
}


validation_files_text::validation_files_text()
{
}


bool
validation_files_text::check(change::pointer cp, fstate_src_ty *src)
{
    //
    // Note: we return true if the file is acceptable to the policy
    // (i.e. has no NUL characters).  We return false if the file is
    // unacceptable (is binary).
    //
    // We aren't trusting libmagic for this one, just in case it only
    // looks at the beginning of the file (many implementations do).
    //
    nstring path(change_file_path(cp, src));
    assert(!path.empty());
    if (path.empty())
	return true;

    os_become_orig();
    bool result = true;
    input ip = input_file_open(path);
    for (;;)
    {
	int c = ip->getch();
	if (c < 0)
	    break;
	if (c == 0)
	{
	    result = false;
	    break;
	}
    }
    ip.close();
    os_become_undo();
    if (!result)
    {
	sub_context_ty sc;
	sc.var_set_string("File_Name", src->file_name);
	change_error(cp, &sc, i18n("$filename: is binary"));
    }
    return result;
}
