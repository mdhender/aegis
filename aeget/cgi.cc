//
//	aegis - project change supervisor
//	Copyright (C) 2003, 2004 Peter Miller;
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
// MANIFEST: functions to manipulate cgis
//

#include <ac/string.h>

#include <cgi.h>
#include <forkandwatch.h>
#include <get.h>
#include <http.h>


struct table_t
{
    const char      *name;
    void            (*action)(void);
};


static table_t table[] =
{
    { "GET", get },
};


void
cgi(void)
{
    const char      *request_method;
    table_t         *tp;

    fork_and_watch();
    request_method = http_getenv("REQUEST_METHOD");
    for (tp = table; tp < ENDOF(table); ++tp)
    {
	if (0 == strcasecmp(request_method, tp->name))
	{
	    tp->action();
	    return;
	}
    }
    http_fatal
    (
	http_error_method_not_allowed,
	"The \"%s\" method is not supported.",
	request_method
    );
}
