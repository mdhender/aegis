//
//	aegis - project change supervisor
//	Copyright (C) 2001-2006 Peter Miller;
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
// MANIFEST: functions to manipulate applys
//

#include <libaegis/input/file_text.h>
#include <libaegis/output/file.h>
#include <libaegis/patch.h>
#include <common/trace.h>


int
patch_apply(patch_ty *pp, string_ty *ifn, string_ty *ofn)
{
    output_ty       *ofp;
    size_t          j;
    size_t          k;
    int             curline;
    int             ok = 1;

    trace(("patch_apply(pp = %08lX, ifn = \"%s\", ofn = \"%s\")\n{\n",
	(long)pp, (ifn ? ifn->str_text : ""), (ofn ? ofn->str_text : "")));
    ofp = output_file_text_open(ofn);
    if (!ifn)
    {
	//
	// Write all the lines from all the hunks
	// to the output file.
	//
	// (There will only be one hunk if the patch asks for
	// a modification to a file which doesn't exist.)
	//
	for (k = 0; k < pp->actions.length; ++k)
	{
	    patch_hunk_ty   *php;
	    patch_line_list_ty *pllp;
	    size_t          m;

	    php = pp->actions.item[0];
	    pllp = &php->after;
	    for (m = 0; m < pllp->length; ++m)
	    {
		ofp->fputs(pllp->item[m].value);
		ofp->fputc('\n');
	    }
	}
    }
    else
    {
	//
	// Read  the file into an array.
	//
	string_list_ty buffer;
	input ifp = input_file_text_open(ifn);
	for (;;)
	{
	    nstring s;
	    if (!ifp->one_line(s))
		break;
	    buffer.push_back(s.get_ref());
	}
	ifp.close();

	//
	// Now work over the hunk list, looking for where they go.
	//
	for (j = 0; j < pp->actions.length; ++j)
	{
	    int             found;
	    patch_hunk_ty   *php;
	    size_t          min_line;
	    int		    running_offset;

	    php = pp->actions.item[j];
	    trace(("looking for %d,%d\n",
		php->before.start_line_number,
		php->before.start_line_number + php->before.length - 1));
	    found = 0;
	    min_line = 0;
	    running_offset = 0;
	    for (k = 1; k <= 2 * buffer.nstrings; ++k)
	    {
		size_t          m;
		int             idx;
		int             offset;

		//
		// 0, -1, 1, -2, 2, etc...
		//
		offset = k >> 1;
		if (k & 1)
		    offset = -offset;
		offset += running_offset;

		//
		// If it doesn't fit in the buffer,
		// it can't be there.
		//
		idx = php->before.start_line_number - 1 + offset;
		if (idx < (int)min_line)
		    continue;
		if (idx + php->before.length > buffer.nstrings)
		    continue;

		//
		// See if the lines match.
		//
		for (m = 0; m < php->before.length; ++m)
		{
		    if
		    (
			!str_equal
			(
			    buffer.string[idx + m],
			    php->before.item[m].value
			)
		    )
			break;
		}
		if (m >= php->before.length)
		{
		    trace(("found, offset=%d\n", offset));
		    php->before.start_line_number += offset;
		    found = 1;
		    min_line =
			php->before.start_line_number - 1 + php->before.length;
		    running_offset = offset;
		    break;
		}
	    }

	    //
	    // Remember if we didn't find a place to apply
	    // the patch.  We'll return it later.
	    //
	    if (!found)
	    {
		php->before.start_line_number += running_offset;
		ok = 0;
	    }
	}

	//
	// Go over each hunk, emitting lines as we go.
	//
	curline = 1;
	for (j = 0; j < pp->actions.length; ++j)
	{
	    patch_hunk_ty   *php;

	    //
	    // First, any prelude.
	    //
	    php = pp->actions.item[j];
	    while
	    (
		curline < php->before.start_line_number
	    &&
		curline <= (int)buffer.nstrings
	    )
	    {
		ofp->fputs(buffer.string[curline - 1]);
		ofp->fputc('\n');
		++curline;
	    }
	    curline += php->before.length;

	    //
	    // Toss the "before" and emit the "after".
	    // We checked that it was there already.
	    //
	    for (k = 0; k < php->after.length; ++k)
	    {
		ofp->fputs(php->after.item[k].value);
		ofp->fputc('\n');
	    }
	}

	//
	// Emit anything left over.
	//
	while (curline <= (int)buffer.nstrings)
	{
	    ofp->fputs(buffer.string[curline - 1]);
	    ofp->fputc('\n');
	    ++curline;
	}
    }
    delete ofp;
    trace(("return %d\n", ok));
    trace(("}\n"));
    return ok;
}
