//
// aegis - project change supervisor
// Copyright (C) 2007, 2008 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or (at
// your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <libaegis/arglex2.h>
#include <libaegis/change/identifier.h>
#include <libaegis/help.h>


void
change_identifier::command_line_parse_rest(void (*usage)(void))
{
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    generic_argument(usage);
	    break;

	case arglex_token_baseline:
	case arglex_token_branch:
	case arglex_token_change:
	case arglex_token_delta:
	case arglex_token_delta_date:
	case arglex_token_development_directory:
	case arglex_token_grandparent:
	case arglex_token_number:
	case arglex_token_project:
	case arglex_token_string:
	case arglex_token_trunk:
            command_line_parse(usage);
	    break;
	}
    }
    command_line_check(usage);
}
