//
//	aegis - project change supervisor
//	Copyright (C) 2006, 2008 Peter Miller
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

#include <common/mem.h>
#include <common/trace.h>
#include <libaegis/col/pretty.h>
#include <libaegis/col/unformatted.h>
#include <libaegis/option.h>
#include <libaegis/os.h>
#include <libaegis/output/file.h>
#include <libaegis/output/pager.h>
#include <libaegis/wide_output/to_narrow.h>


col::pointer
col::open(string_ty *filename)
{
    //
    // open a suitable output
    //
    trace(("col::open(filename = %08lX)\n{\n", (long)filename));
    os_become_must_not_be_active();
    output::pointer narrow_fp;
    if (filename && filename->str_length)
    {
	trace_string(filename->str_text);
	os_become_orig();
	narrow_fp = output_file::text_open(filename);
	os_become_undo();
    }
    else
	narrow_fp = output_pager::open();
    wide_output::pointer wide_fp = wide_output_to_narrow::open(narrow_fp);

    //
    // pick a formatting option
    //
    if (option_unformatted_get())
    {
        trace(("}\n"));
	return col_unformatted::create(wide_fp);
    }

    trace(("}\n"));
    return col_pretty::create(wide_fp);
}
