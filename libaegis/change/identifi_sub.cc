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
// MANIFEST: implementation of the change_identifi_sub class
//

#pragma implementation "change_identifier_subset"

#include <change/identifi_sub.h>
#include <change.h>
#include <project/file/roll_forward.h>
#include <trace.h>


change_identifier_subset::~change_identifier_subset()
{
    trace(("change_identifier_subset::~change_identifier_subset()\n{\n"));
    if (historian_p)
    {
	delete historian_p;
	historian_p = 0;
    }
    if (cp)
    {
	change_free(cp);
	cp = 0;
    }
    trace(("}\n"));
}


change_identifier_subset::change_identifier_subset(
	project_identifier_subset &arg) :
    pid(arg),
    baseline(false),
    devdir(false),
    change_number(0),
    delta_date(NO_TIME_SET),
    delta_number(-1),
    cp(0),
    historian_p(0)
{
    trace(("change_identifier_subset::change_identifier_subset()\n"));
}
