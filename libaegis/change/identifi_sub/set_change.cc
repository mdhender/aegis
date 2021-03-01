//
//	aegis - project change supervisor
//	Copyright (C) 2004-2008 Peter Miller
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

#include <libaegis/change.h>
#include <libaegis/change/identifi_sub.h>
#include <common/error.h> // for assert
#include <common/now.h>
#include <libaegis/project.h>
#include <libaegis/project/history.h>
#include <libaegis/project/identifi_sub.h>
#include <libaegis/sub.h>
#include <common/trace.h>
#include <libaegis/user.h>


void
change_identifier_subset::set_change()
{
    trace(("change_identifier_subset::set_change()\n{\n"));
    assert(!cp);
    if (baseline)
    {
	cp = change_copy(pid.get_pp()->change_get());
	trace(("baseline\n"));
	trace(("}\n"));
	return;
    }

    //
    // it is an error if the delta does not exist
    //
    if (!delta_name.empty())
    {
	change_number =
	    project_history_change_by_name
	    (
		pid.get_pp(),
		delta_name.get_ref(),
		0
	    );
	trace(("delta \"%s\" == change %ld\n", delta_name.c_str(),
	    change_number));
	delta_name = "";
    }
    if (delta_date != NO_TIME_SET)
    {
	//
	// If the time is in the future, you could get a different
	// answer for the same input at some point in the future.
	//
	// This is the "time safe" quality first described by
	// Damon Poole <damon@ede.com>
	//
	if (delta_date > now())
	    project_error(pid.get_pp(), 0, i18n("date in the future"));

	//
    	// Now find the change number corresponding.
	//
	change_number =
	    project_history_change_by_timestamp(pid.get_pp(), delta_date);
	trace(("delta date %ld == change %ld\n", (long)delta_date,
	    change_number));
    }
    if (delta_number >= 0)
    {
	// does not return if no such delta number
	change_number =
	    project_history_change_by_delta(pid.get_pp(), delta_number);
	trace(("delta %ld == change %ld\n", delta_number, change_number));
	delta_number = 0;
    }

    //
    // locate change data
    //
    if (!change_number)
    {
	change_number = get_up()->default_change(get_pp());
	trace(("change %ld\n", change_number));
    }
    cp = change_alloc(pid.get_pp(), change_number);
    change_bind_existing(cp);
    trace(("}\n"));
}
