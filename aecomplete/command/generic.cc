//
//	aegis - project change supervisor
//	Copyright (C) 2002, 2004-2006, 2008 Peter Miller
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

#include <libaegis/arglex2.h>
#include <aecomplete/command/generic.h>
#include <aecomplete/complete/filename.h>
#include <aecomplete/complete/nil.h>


complete_ty *
generic_argument_complete()
{
    switch (arglex_token)
    {
    default:
	// discard
	arglex();
	break;

    case arglex_token_option_incomplete:
	// Maybe "complete_option"?
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
	    // Maybe "complete_number"?
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
