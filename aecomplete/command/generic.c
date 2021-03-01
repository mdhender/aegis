/*
 *	aegis - project change supervisor
 *	Copyright (C) 2002 Peter Miller;
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
 * MANIFEST: functions to manipulate generics
 */

#include <arglex2.h>
#include <command/generic.h>
#include <complete/filename.h>
#include <complete/nil.h>


complete_ty *
generic_argument_complete()
{
    switch (arglex_token)
    {
    default:
	/* discard */
	arglex();
	break;

    case arglex_token_option_incomplete:
	/* Maybe "complete_option"? */
	return complete_nil();

    case arglex_token_file:
    case arglex_token_output:
	switch (arglex())
	{
	case arglex_token_string:
	    arglex();
	    break;

	case arglex_token_string_incomplete:
	case arglex_token_number_incomplete:
	    return complete_filename(0);

	default:
	    break;
	}
	break;

    case arglex_token_library:
    case arglex_token_directory:
	switch (arglex())
	{
	case arglex_token_string:
	    arglex();
	    break;

	case arglex_token_string_incomplete:
	case arglex_token_number_incomplete:
	    return complete_filename(1);

	default:
	    break;
	}
	break;

    case arglex_token_page_length:
    case arglex_token_page_width:
    case arglex_token_tab_width:
	switch (arglex())
	{
	case arglex_token_number:
	    arglex();
	    break;

	case arglex_token_string_incomplete:
	case arglex_token_number_incomplete:
	    /* Maybe "complete_number"? */
	    return complete_nil();

	default:
	    break;
	}
	break;

    case arglex_token_trace:
	for (;;)
	{
	    switch (arglex())
	    {
	    default:
		break;

	    case arglex_token_string:
		arglex();
		continue;

	    case arglex_token_string_incomplete:
	    case arglex_token_number_incomplete:
		return complete_filename(0);
	    }
	    break;
	}
	break;
    }
    return 0;
}
