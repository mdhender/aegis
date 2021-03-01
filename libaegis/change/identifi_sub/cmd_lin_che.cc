//
//	aegis - project change supervisor
//	Copyright (C) 2004 Peter Miller;
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
// MANIFEST: implementation of the change_id_cmd_lin_che class
//

#include <arglex2.h>
#include <change/identifi_sub.h>
#include <help.h>
#include <project/identifi_sub.h>


void
change_identifier_subset::command_line_check(void (*usage)(void))
{
    pid.command_line_check(usage);

    //
    // reject illegal combinations of options
    //
    if (baseline && devdir)
    {
	mutually_exclusive_options
	(
	    arglex_token_baseline,
	    arglex_token_development_directory,
	    usage
	);
    }

    if
    (
	(
	    !!baseline
	+
	    (!delta_name.empty() || delta_number >= 0)
	+
	    !!change_number
	+
	    (delta_date != NO_TIME_SET)
	)
    >
	1
    )
    {
	mutually_exclusive_options3
	(
	    arglex_token_baseline,
	    arglex_token_change,
	    arglex_token_delta,
	    usage
	);
    }
}
