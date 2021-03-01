//
// aegis - project change supervisor
// Copyright (C) 2004-2006, 2008, 2012 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published
// by the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <common/ac/assert.h>
#include <common/ac/stdlib.h>
#include <common/ac/string.h>

#include <common/trace.h>
#include <libaegis/os.h>
#include <libaegis/url.h>


url::~url()
{
}


url::url() :
    protocol("file"),
    hostname("localhost"),
    port(80)
{
}


url::url(const nstring &s) :
    port(0)
{
    split(s.c_str());
}


url::url(const char *s) :
    port(0)
{
    split(s);
}


url::url(const url &arg) :
    protocol(arg.protocol),
    userpass(arg.userpass),
    hostname(arg.hostname),
    port(arg.port),
    path(arg.path),
    query(arg.query),
    anchor_name(arg.anchor_name)
{
}


url &
url::operator=(const url &arg)
{
    if (this != &arg)
    {
        protocol = arg.protocol;
        userpass = arg.userpass;
        hostname = arg.hostname;
        port = arg.port;
        path = arg.path;
        query = arg.query;
        anchor_name = arg.anchor_name;
    }
    return *this;
}


static bool
check_exists(const nstring &path)
{
    os_become_orig();
    bool result = os_exists(path);
    os_become_undo();
    trace(("check_exists(\"%s\")->%d\n", path.c_str(), (int)result));
    return result;
}


void
url::split(const char *cp)
{
    assert(cp);
    trace(("url::split(cp = \"%s\")\n{\n", cp));

    //
    // Clear everything to a known state.
    // (But make sure it *exactly* matches the default constructor.)
    //
    protocol = "file";
    userpass = "";
    hostname = "localhost";
    port = 80;
    path = "";
    query = "";
    anchor_name = "";

    //
    // We have a couple of escapes, to ensure that ordinary files get
    // through as a file, and not as some weird and incorrect URL.
    //
    if (cp[0] == '/')
    {
        path = nstring(cp);
        trace(("return \"%s\";\n}\n", reassemble().c_str()));
        return;
    }
    if (check_exists(nstring(cp)))
    {
        os_become_orig();
        nstring here(os_curdir());
        os_become_undo();
        path = os_path_rel2abs(here, nstring(cp));
        trace(("return \"%s\";\n}\n", reassemble().c_str()));
        return;
    }

    //
    // Extract the protocol portion.
    //
    const char *colon = strpbrk(cp, ":/?#");
    if (colon && colon[0] == ':')
    {
        protocol = nstring(cp, colon - cp).downcase();
        cp = colon + 1;
    }
    else
    {
        protocol = "http";
    }

    //
    // Extract the hostname portion.
    //
    if (cp[0] == '/' && cp[1] == '/')
    {
        // we are definitely looking at a host name
        cp += 2;
        const char *end_of_hostname = strpbrk(cp, "/?#");
        if (!end_of_hostname)
        {
            hostname = nstring(cp);
            cp += hostname.size();
            path = "/";
        }
        else if (end_of_hostname == cp)
        {
            hostname = "localhost";
        }
        else
        {
            hostname = nstring(cp, end_of_hostname - cp).downcase();
            cp = end_of_hostname;
        }
    }
    else if (check_exists(nstring(cp)))
    {
        //
        // We are looking at a file name.
        //
        protocol = "file";
        userpass = "";
        hostname = "localhost";
        os_become_orig();
        nstring here(os_curdir());
        os_become_undo();
        path = os_path_rel2abs(here, nstring(cp));
        cp += strlen(cp);
        trace(("return \"%s\";\n}\n", reassemble().c_str()));
        return;
    }
    else
    {
        //
        // We are looking at a host name.
        //
        const char *end_of_hostname = strpbrk(cp, "/?#");
        if (!end_of_hostname)
        {
            hostname = nstring(cp).downcase();
            cp += hostname.size();
        }
        else if (end_of_hostname == cp)
            hostname = "localhost";
        else
        {
            hostname = nstring(cp, end_of_hostname - cp).downcase();
            cp = end_of_hostname;
        }
    }

    //
    // Extract the user name and password portion from the hostname.
    //
    const char *at = strchr(hostname.c_str(), '@');
    if (at)
    {
        userpass = nstring(hostname.c_str(), at - hostname.c_str());
        ++at; // must exclude @ sign
        hostname = nstring(at);
    }

    //
    // Extract the port portion from the hostname.
    //
    colon = strchr(hostname.c_str(), ':');
    if (colon)
    {
        port = atoi(colon + 1);
        if (port <= 0 || port >= 0x10000)
            port = 80;
        hostname = nstring(hostname.c_str(), colon - hostname.c_str());
    }

    //
    // Extract the path portion.
    //
    if (*cp != '?' && *cp != '#')
    {
        const char *end_of_path = strpbrk(cp, "?#");
        if (!end_of_path)
        {
            path = nstring(cp);
            trace(("return \"%s\";\n}\n", reassemble().c_str()));
            return;
        }
        path = nstring(cp, end_of_path - cp);
        cp = end_of_path;
    }
    else
    {
        // No path given.  Leave it empty for now.
    }

    //
    // Extract the query portion.
    //
    if (*cp == '?')
    {
        const char *end_of_query = strchr(cp, '#');
        if (!end_of_query)
        {
            query = nstring(cp);
            cp += query.size();
        }
        else
        {
            query = nstring(cp, end_of_query - cp);
            cp = end_of_query;
        }
    }

    //
    // Extract the anchor_name portion.
    //
    if (*cp == '#')
    {
        anchor_name = nstring(cp);
        cp += anchor_name.size();
    }
    assert(*cp == '\0');
    trace(("return \"%s\";\n}\n", reassemble().c_str()));
}


bool
url::is_a_file()
    const
{
    return (protocol == "file");
}


nstring
url::get_path()
    const
{
    assert(is_a_file());
    if (path.empty())
        return "/";
    assert(path[0] == '/');
    return path;
}


void
url::set_path_if_empty(const nstring &s)
{
    assert(!strpbrk(s.c_str(), "?#"));
    if (path.empty())
    {
        if (path[0] == '/')
            path = s;
        else
            path = "/" + s;
    }
}


void
url::set_query_if_empty(const nstring &s)
{
    assert(s.c_str()[0] != '?');
    if (query.empty())
        query = "?" + s;
}


nstring
url::reassemble(bool exclude_userpass)
    const
{
    if (is_a_file())
        return get_path();
    assert(path.empty() || path[0] == '/');
    assert(query.empty() || query[0] == '?');
    assert(anchor_name.empty() || anchor_name[0] == '#');
    return
        (
            protocol
        +
            "://"
        +
            // This includes the trailing '@' if needed
            ((exclude_userpass || userpass.empty()) ? "" : userpass + "@")
        +
            hostname
        +
            (port == 80 ? "" : nstring::format(":%d", port))
        +
            // The path includes the '/' or is empty
            (path.empty() ? "/" : path)
        +
            // This includes the '?' or is empty
            query
        +
            // This includes the '#' or is empty
            anchor_name
        );
}


void
url::set_host_part_from(const url &arg)
{
    protocol = arg.get_protocol();
    hostname = arg.get_hostname();
    port = arg.get_port();
}


// vim: set ts=8 sw=4 et :
