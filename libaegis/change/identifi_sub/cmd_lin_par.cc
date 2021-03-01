//
//	aegis - project change supervisor
//	Copyright (C) 2004-2006, 2008 Peter Miller
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
#include <libaegis/change/identifi_sub.h>
#include <common/error.h>
#include <common/gettime.h>
#include <libaegis/help.h>
#include <libaegis/project/identifi_sub.h>
#include <libaegis/sub.h>
#include <common/trace.h>


void
change_identifier_subset::command_line_parse(void (*usage)(void))
{
    trace(("change_identifier_subset::command_line_parse()\n{\n"));
    switch (arglex_token)
    {
    default:
	fatal_raw
	(
	    "%s: %d: option %s not handled in switch (bug)",
	    __FILE__,
	    __LINE__,
	    arglex_token_name(arglex_token)
	);
	// NOTREACHED

    case arglex_token_baseline:
	if (baseline)
	    duplicate_option(usage);
	baseline = true;
	break;

    case arglex_token_development_directory:
	if (devdir)
	    duplicate_option(usage);
	devdir = true;
	break;

    case arglex_token_change:
    case arglex_token_delta_from_change:
	arglex();
	// fall through...

    case arglex_token_number:
	pid.parse_change_with_branch(change_number, usage);
	trace(("}\n"));
	return;

    case arglex_token_delta:
	if (delta_number >= 0 || delta_name)
	    duplicate_option(usage);
	switch (arglex())
	{
	default:
	    option_needs_number(arglex_token_delta, usage);
	    // NOTREACHED

	case arglex_token_number:
	    delta_number = arglex_value.alv_number;
	    if (delta_number < 0)
	    {
		sub_context_ty *scp = sub_context_new();
		sub_var_set_long(scp, "Number", delta_number);
		fatal_intl(scp, i18n("delta $number out of range"));
		// NOTREACHED
		sub_context_delete(scp);
	    }
	    break;

	case arglex_token_string:
	    delta_name = arglex_value.alv_string;
	    break;
	}
	break;

    case arglex_token_delta_date:
	if (delta_date != NO_TIME_SET)
	    duplicate_option(usage);
	if (arglex() != arglex_token_string)
	{
	    option_needs_string(arglex_token_delta_date, usage);
	    // NOTREACHED
	}
	delta_date = date_scan(arglex_value.alv_string);
	if (delta_date == NO_TIME_SET)
	    fatal_date_unknown(arglex_value.alv_string);
	break;

    case arglex_token_project:
    case arglex_token_string:
    case arglex_token_branch:
    case arglex_token_trunk:
    case arglex_token_grandparent:
	pid.command_line_parse(usage);
	trace(("}\n"));
	return;
    }
    arglex();
    trace(("}\n"));
}
