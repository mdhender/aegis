//
//	aegis - project change supervisor
//	Copyright (C) 2006 Peter Miller
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
// MANIFEST: implementation of the col::open class method
//

#include <common/trace.h>
#include <libaegis/col/pretty.h>
#include <libaegis/col/unformatted.h>
#include <libaegis/option.h>
#include <libaegis/os.h>
#include <libaegis/output/file.h>
#include <libaegis/output/pager.h>
#include <libaegis/wide_output/to_narrow.h>


col *
col::open(string_ty *filename)
{
    //
    // open a suitable output
    //
    trace(("col::open(filename = %08lX)\n{\n", (long)filename));
    os_become_must_not_be_active();
    output_ty *narrow_fp = 0;
    if (filename && filename->str_length)
    {
	trace_string(filename->str_text);
	os_become_orig();
	narrow_fp = output_file_text_open(filename);
	os_become_undo();
    }
    else
	narrow_fp = output_pager_open();
    wide_output_ty *wide_fp = wide_output_to_narrow_open(narrow_fp, 1);

    //
    // pick a formatting option
    //
    col *result = 0;
    if (option_unformatted_get())
	result = new col_unformatted(wide_fp, true);
    else
	result = new col_pretty(wide_fp, true);

    //
    // all done
    //
    trace(("return %08lX;\n", (long)result));
    trace(("}\n"));
    return result;
}
