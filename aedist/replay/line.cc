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

#include <common/ac/ctype.h>
#include <common/ac/string.h>

#include <common/error.h> // for assert
#include <aedist/replay/line.h>
#include <common/trace.h>
#include <common/uuidentifier.h>


replay_line::~replay_line()
{
}


replay_line::replay_line()
{
}


replay_line::replay_line(const replay_line &arg) :
    url1(arg.url1),
    version(arg.version),
    url2(arg.url2),
    uuid(arg.uuid),
    description(arg.description)
{
}


replay_line &
replay_line::operator=(const replay_line &arg)
{
    if (this != &arg)
    {
	url1 = arg.url1;
	version = arg.version;
	url2 = arg.url2;
	uuid = arg.uuid;
	description = arg.description;
    }
    return *this;
}


static const char *
get_next_form(const char *cp, nstring &result)
{
    // C locale
    while (*cp && isspace((unsigned char)*cp))
	++cp;
    if (*cp == '\0')
    {
	result = "";
	return cp;
    }
    if (*cp != '<')
    {
	const char *ep = strchr(cp, '<');
	if (!ep)
	    ep = cp + strlen(cp);
	result = nstring(cp, ep - cp);
	return ep;
    }
    const char *ep = strchr(cp, '>');
    if (!ep)
	return 0;
    result = nstring(cp, ep - cp);
    return (ep + 1);
}


static bool
extract_url(const nstring &form, nstring &url)
{
    //
    // The string should be of the form
    //     "<a href=\"...\""
    //
    assert(form.downcase().starts_with("<a href=\""));
    const char *cp = form.c_str() + 9;
    const char *ep = strchr(cp, '"');
    if (!ep || cp == ep)
	return false;
    url = nstring(cp, ep - cp);
    return true;
}


bool
replay_line::extract(const nstring &line)
{
    trace(("replay_line::extract(line = \"%s\")\n{\n", line.c_str()));

    //
    // The line will match the following regex:
    //     "<tr[^>]*>"
    //     "<td><a href=\"[^\"]+\">([^<]+)</a></td>"
    //     "<td><tt><a href=\"([^\"]+)\">([^<]+)</a></tt></td>"
    //     "<td>([^<]+)</td>"
    //     "</tr>"
    //
    // But we match it manually, because then we can work on systems
    // without GNU Regex extensions.
    //
    const char *cp = line.c_str();
    nstring form;
    cp = get_next_form(cp, form);
    if (!cp || !form.downcase().starts_with("<tr"))
    {
	trace(("}\n"));
	return false;
    }

    cp = get_next_form(cp, form);
    if (!cp || !form.downcase().starts_with("<td"))
    {
	trace(("}\n"));
	return false;
    }

    cp = get_next_form(cp, form);
    if (!cp || !form.downcase().starts_with("<a href=\""))
    {
	trace(("}\n"));
	return false;
    }

    //
    // extract the URL from this one
    //
    if (!extract_url(form, url1))
    {
	trace(("}\n"));
	return false;
    }
    trace_nstring(url1);

    //
    // The next one is the version string.
    //
    cp = get_next_form(cp, version);
    if (!cp || version.empty() || version[0] == '<')
    {
	trace(("}\n"));
	return false;
    }
    trace_nstring(version);

    cp = get_next_form(cp, form);
    if (!cp || !form.downcase().starts_with("</a"))
    {
	trace(("}\n"));
	return false;
    }

    cp = get_next_form(cp, form);
    if (!cp || !form.downcase().starts_with("</td"))
    {
	trace(("}\n"));
	return false;
    }

    cp = get_next_form(cp, form);
    if (!cp || !form.downcase().starts_with("<td"))
    {
	trace(("}\n"));
	return false;
    }

    cp = get_next_form(cp, form);
    if (!cp || !form.downcase().starts_with("<tt"))
    {
	trace(("}\n"));
	return false;
    }

    cp = get_next_form(cp, form);
    if (!cp || !form.downcase().starts_with("<a href=\""))
    {
	trace(("}\n"));
	return false;
    }

    //
    // extract the URL from this one
    //
    if (!extract_url(form, url2))
    {
	trace(("}\n"));
	return false;
    }
    trace_nstring(url2);

    //
    // The next one is the UUID.
    //
    cp = get_next_form(cp, uuid);
    if (!cp || uuid.empty() || uuid[0] == '<')
    {
	trace(("}\n"));
	return false;
    }
    if (!universal_unique_identifier_valid(uuid.get_ref()))
    {
	trace(("}\n"));
	return false;
    }
    trace_nstring(uuid);

    cp = get_next_form(cp, form);
    if (!cp || !form.downcase().starts_with("</a"))
    {
	trace(("}\n"));
	return false;
    }

    cp = get_next_form(cp, form);
    if (!cp || !form.downcase().starts_with("</tt"))
    {
	trace(("}\n"));
	return false;
    }

    cp = get_next_form(cp, form);
    if (!cp || !form.downcase().starts_with("</td"))
    {
	trace(("}\n"));
	return false;
    }

    cp = get_next_form(cp, form);
    if (!cp || !form.downcase().starts_with("<td"))
    {
	trace(("}\n"));
	return false;
    }

    //
    // The next one is the brief description string.
    //
    cp = get_next_form(cp, description);
    if (!cp || description[0] == '<')
    {
	trace(("}\n"));
	return false;
    }
    trace_nstring(description);

    //
    // We'll be generous and ignore the rest of the line (it should be
    // </td></tr>) just in case more columns are added in the future.
    //
    trace(("return true;\n"));
    trace(("}\n"));
    return true;
}
