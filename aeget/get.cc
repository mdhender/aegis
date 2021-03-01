//
//      aegis - project change supervisor
//      Copyright (C) 2003-2008, 2011, 2012 Peter Miller
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 3 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program. If not, see
//      <http://www.gnu.org/licenses/>.
//

#include <common/ac/stdlib.h>
#include <common/ac/string.h>

#include <common/nstring/list.h>
#include <common/str.h>
#include <libaegis/change.h>
#include <libaegis/http.h>
#include <libaegis/project.h>
#include <libaegis/project/history.h>

#include <aeget/get.h>
#include <aeget/get/change.h>
#include <aeget/get/icon.h>
#include <aeget/get/project.h>
#include <aeget/get/project/list.h>
#include <aeget/get/rect.h>


static int
extract_change_number(string_ty **project_name_p, long *change_number_p)
{
    string_ty       *project_name;
    const char      *cp;
    char            *end;
    long            change_number;
    string_ty       *new_project_name;

    project_name = *project_name_p;
    cp = strstr(project_name->str_text, ".C");
    if (!cp)
        cp = strstr(project_name->str_text, ".c");
    if (!cp)
        return 0;
    change_number = strtol(cp + 2, &end, 10);
    if (end == cp + 2 || *end)
        return 0;
    *change_number_p = change_number;
    new_project_name =
        str_n_from_c(project_name->str_text, cp - project_name->str_text);
    str_free(project_name);
    *project_name_p = new_project_name;
    return 1;
}


static int
extract_delta_number(string_ty **project_name_p, long *delta_number_p)
{
    string_ty       *project_name;
    const char      *cp;
    char            *end;
    long            delta_number;
    string_ty       *new_project_name;

    project_name = *project_name_p;
    cp = strstr(project_name->str_text, ".D");
    if (!cp)
        cp = strstr(project_name->str_text, ".d");
    if (!cp)
        return 0;
    delta_number = strtol(cp + 2, &end, 10);
    if (end == cp + 2 || *end)
        return 0;
    *delta_number_p = delta_number;
    new_project_name =
        str_n_from_c(project_name->str_text, cp - project_name->str_text);
    str_free(project_name);
    *project_name_p = new_project_name;
    return 1;
}


static string_ty *
path_info(void)
{
    const char      *path;

    path = getenv("PATH_INFO");
    if (!path)
        path = "/";
    if (path[0] == '/')
        ++path;
    return str_from_c(path);
}


void
get(void)
{
    //
    // Get the path being accessed.
    // The web server has already decoded the % escapes for us.
    //
    string_ty *path = path_info();

    //
    // Pull out the @@ modifier.
    // It may have several parts, e.g. @@changes@being_developed
    //
    string_list_ty modifier;
    const char *end = strstr(path->str_text, "@@");
    if (end)
    {
        string_ty *s = str_from_c(end + 2);
        modifier.split(s, "@");
        str_free(s);

        s = str_n_from_c(path->str_text, end - path->str_text);
        str_free(path);
        path = s;
    }

    //
    // Pull out the ? modifier.
    // It may have several parts, e.g. ?changes+being_developed
    //
    const char *query_string = getenv("QUERY_STRING");
    if (query_string && *query_string)
    {
        nstring_list qm;
        qm.split(query_string, "+");
        for (size_t j = 0; j < qm.size(); ++j)
        {
            nstring ms = qm[j].url_unquote();
            modifier.push_back(ms.get_ref());
        }
    }

    //
    // If the noerror modifier is present, a status os 200 (success)
    // will always be returned, no matter what.  This is especially
    // important for debugging, because the client can see the text of
    // the error message.
    //
    if
    (
        modifier_test_and_clear(&modifier, "noerror")
    ||
        modifier_test_and_clear(&modifier, "debug")
    )
    {
        http_fatal_noerror = true;
    }

    if (path->str_length == 0)
    {
        if
        (
            modifier.nstrings >= 1
        &&
            0 == strcasecmp(modifier.string[0]->str_text, "rect")
        )
        {
            get_rect(&modifier);
            return;
        }

        //
        // Give the project list for the root directory.
        //
        if (path->str_length == 0)
        {
            get_project_list();
            return;
        }
    }

    //
    // If the first component is "icon", the rest of the path looks into
    // $(datadir)/icon/ to find the given file.
    //
    // Make sure they can't abuse this access method by inserting ".."
    // components into the path.
    //
    if
    (
        0 == memcmp(path->str_text, "icon/", 5)
    &&
        0 == memmem(path->str_text, path->str_length, "/../", 4)
    )
    {
        //
        // Leave the "icon/" in the path, that way we can re-use it for
        // other directories in the future, if needed.
        //
        get_icon(path, &modifier);
        return;
    }

    //
    // The first component is "project" or "project.Cnnn"
    //
    end = strchr(path->str_text, '/');
    string_ty *project_name = 0;
    if (!end)
    {
        project_name = path;
        path = str_from_c(".");
    }
    else
    {
        string_ty       *s;

        project_name = str_n_from_c(path->str_text, end - path->str_text);
        while (*end == '/')
            ++end;
        s = str_from_c(*end ? end : ".");
        str_free(path);
        path = s;
    }

    //
    // Figure out if there is a .Cnnn suffix.
    //
    long change_number = 0;
    if (extract_change_number(&project_name, &change_number))
    {
        project *pp = project_alloc(project_name);
        pp->bind_existing();

        change::pointer cp = change_alloc(pp, change_number);
        change_bind_existing(cp);

        //
        // From this point, fetch a file from the change.
        //
        get_change(cp, path, &modifier);
        change_free(cp);
        project_free(pp);
    }
    else
    {
        long delta_number = 0;
        if (extract_delta_number(&project_name, &delta_number))
        {
            project *pp = project_alloc(project_name);
            pp->bind_existing();

            change_number =
                project_history_change_by_delta(pp, delta_number);

            change::pointer cp = change_alloc(pp, change_number);
            change_bind_existing(cp);

            //
            // From this point, fetch a file from the change.
            //
            get_change(cp, path, &modifier);
            change_free(cp);
            project_free(pp);
        }
        else
        {
            project *pp = project_alloc(project_name);
            pp->bind_existing();

            //
            // From this point, fetch a file from the project.
            //
            get_project(pp, path, &modifier);
            project_free(pp);
        }
    }
    str_free(project_name);
    str_free(path);
}


// vim: set ts=8 sw=4 et :
