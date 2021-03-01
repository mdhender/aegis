//
//	aegis - project change supervisor
//	Copyright (C) 2001-2005 Peter Miller;
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
// MANIFEST: functions to manipulate reads
//

#include <ac/string.h>

#include <common.h>
#include <error.h> // assert
#include <input.h>
#include <patch.h>
#include <patch/context.h>
#include <patch/format/context.h>
#include <patch/format/diff.h>
#include <patch/format/uni.h>
#include <patch/list.h>
#include <trace.h>

static patch_format_ty *format[] =
{
	&patch_format_context,
	&patch_format_uni,
	&patch_format_diff,
};

patch_list_ty *
patch_read(input_ty *input, int required)
{
    patch_list_ty   *result;
    patch_context_ty *context;

    trace(("patch_read(input = %08lX)\n{\n", (long)input));
    context = patch_context_new(input);
    result = patch_list_new();
    string_list_ty garbage;
    for (;;)
    {
	patch_ty	*pp;
	size_t		j;
	patch_format_ty *fp;

	//
	// Look for a patch header;
	// it could be in one of several formats.
	//
	trace(("%s\n", context->get_file_name().c_str()));
	pp = 0;
	fp = 0;
	for (j = 0; j < SIZEOF(format); ++j)
	{
	    fp = format[j];
	    pp = fp->header(context);
	    if (pp)
	       	break;
	    fp = 0;
	}
	assert(!pp == !fp);

	//
	// If there was no patch in there, we need to deal with
	// the garbage.
	//
	if (!pp)
	{
	    string_ty	*s;

	    s = patch_context_getline(context, 0);
	    if (!s)
		break;

	    //
	    // Stash the first line in the buffer,
	    // and discard it from the context.
	    //
	    garbage.push_back(s);
	    patch_context_discard(context, 1);
	    trace(("toss one line\n"));
	    continue;
	}
	trace(("found header, type %s\n", fp->name));
	assert(pp->name.nstrings);

	//
	// The first lot of "garbage" is kept as the change
	// description.
	//
	if (result->length == 0)
	{
	    result->description = garbage.unsplit("\n");
	}
	garbage.clear();

	//
	// Found a patch header, read the rest of it.
	//
	patch_list_append(result, pp);
	assert(fp);
	for (;;)
	{
	    patch_hunk_ty	*php;

	    php = fp->hunk(context);
	    if (!php)
	       	break;
	    trace(("got hunk\n"));
	    patch_append(pp, php);
	}

	//
	// Figure what this patch is doing.
	//
	pp->action = file_action_modify;
	if (pp->actions.length == 1)
	{
	    patch_hunk_ty	*php;

	    php = pp->actions.item[0];
	    if
	    (
		php->after.length == 0
	    &&
		php->after.start_line_number == 0
	    )
		pp->action = file_action_remove;
	    else if
	    (
		php->before.length == 0
	    &&
		php->before.start_line_number == 0
	    )
		pp->action = file_action_create;
	}

	//
	// Figure what sort of file this is.
	//
	// (How do we know it's a test?  It is entirely possible
	// that test names have been changed from the default.)
	//
	pp->usage = file_usage_source;
	if
	(
	    pp->name.nstrings
	&&
	    strstr(pp->name.string[0]->str_text, "test/")
	&&
	    strstr(pp->name.string[0]->str_text, ".sh")
	)
	    pp->usage = file_usage_test;
    }
    if (required && result->length == 0)
	input->fatal_error("no patch found");

    //
    // clean up and go home
    //
    patch_context_delete(context);
    trace(("return %08lX\n", (long)result));
    trace(("}\n"));
    return result;
}
