//
// aegis - project change supervisor
// Copyright (C) 2012 Peter Miller
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

#include <common/ac/string.h>

#include <common/nstring/list.h>
#include <libaegis/attribute.h>
#include <libaegis/change/file.h>
#include <libaegis/sub.h>

#include <aede-policy/validation/man_pages.h>


validation_man_pages::~validation_man_pages()
{
}


validation_man_pages::validation_man_pages()
{
}


validation_man_pages::pointer
validation_man_pages::create(void)
{
    return pointer(new validation_man_pages());
}


static nstring
extract_man_page_details(const nstring_list &part, size_t idx)
{
    if (idx + 2 != part.size())
        return nstring();
    if (!part[idx].gmatch("man[18nl]*"))
        return nstring();
    nstring section =  "." + part[idx].substr(3, 1);
    nstring progname = part[idx + 1];
    const char *s = progname.c_str();
    const char *extn = strstr(s, section.c_str());
    if (!extn || extn == s)
        return nstring();
    progname = nstring(s, extn - s);
    return progname;
}


static nstring
extract_man_page_details(const nstring &filename)
{
    nstring_list part;
    part.split(filename, "/");

    // e.g. man/man1/fred.1
    if (part.size() == 3 && part[0] == "man")
    {
        nstring progname = extract_man_page_details(part, 1);
        if (!progname.empty())
            return progname;
    }
    // e.g. "man1/fred.1"
    if (part.size() == 2)
    {
        nstring progname = extract_man_page_details(part, 0);
        if (!progname.empty())
            return progname;
    }
    // e.g. "lib/en/man1/fred.1"
    if (part.size() >= 3 && part[part.size() - 3] == "en")
    {
        nstring progname = extract_man_page_details(part, part.size() - 2);
        if (!progname.empty())
            return progname;
    }
    return nstring();
}


static nstring
extract_progname_details(const nstring &filename)
{
    nstring_list part;
    part.split(filename, "/");
    if (part.size() != 2)
        return nstring();

    // look for C and C++ program entry points
    if (part[1] == "main.c" || part[1] == "main.cc")
        return part[0];

    // look for sripts
    if (part[0] == "script" || part[0] == "scripts")
    {
        nstring progname = part[1];
        if (progname.ends_with(".in"))
            progname = progname.substr(0, progname.size() - 3);
        if (progname.ends_with(".sh") || progname.ends_with(".pl"))
            return progname.substr(0, progname.size() - 3);
    }

    // i gots nufink
    return nstring();
}


static nstring
extract_progname_details_generous(const nstring &filename)
{
    nstring_list part;
    part.split(filename, "/");
    if (part.size() < 2)
        return nstring();

    // looks for scripts
    if (part[0] == "script" || part[0] == "scripts")
    {
        nstring progname = part[1];
        if (progname.ends_with(".in"))
            progname = progname.substr(0, progname.size() - 3);
        if (progname.ends_with(".sh") || progname.ends_with(".pl"))
            return progname.substr(0, progname.size() - 3);
    }

    // looks for C and C++ programs
    if (part[1].ends_with(".c") || part[1].ends_with(".cc"))
        return part[0];

    // i gots nufink
    return nstring();
}


static bool
file_attr_noinst(fstate_src_ty *src)
{
    return
        (
            src
        &&
            src->attribute
        &&
            attributes_list_find_boolean(src->attribute, "aemakegen:noinst")
        );
}


bool
validation_man_pages::run(change::pointer cp)
{
    //
    // Don't check branches, only individual changes have control over
    // the presence of man pages.
    //
    if (cp->was_a_branch())
        return true;

    //
    // Don't perform this check for changes downloaded and applied by
    // aedist, because the original developer is no longer in control.
    //
    if (was_downloaded(cp))
        return true;

    //
    // Don't perform this check for change sets marked as owning a
    // foreign copyright.
    //
    if (cp->attributes_get_boolean("foreign-copyright"))
        return true;

    //
    // Look at every file in the (project + change set), building lists
    // of program names and man page names.
    //
    nstring_list man_pages;
    nstring_list programs;
    for (long j = 0; ; ++j)
    {
        fstate_src_ty *src = change_file_nth(cp, j, view_path_extreme);
        if (!src)
            break;
        nstring filename(src->file_name);

        // remember man page names, but only sections 1 and 8
        {
            nstring progname = extract_man_page_details(filename);
            if (!progname.empty())
            {
                man_pages.push_back_unique(progname);
            }
        }

        // remember program names, but only installable ones
        {
            nstring progname = extract_progname_details(filename);
            if
            (
                !progname.empty()
            &&
                !file_attr_noinst(src)
            &&
                !progname.starts_with("test_")
            &&
                !progname.starts_with("test-")
            &&
                !progname.starts_with("noinst_")
            &&
                !progname.starts_with("noinst-")
            )
            {
                programs.push_back_unique(progname);
            }
        }
    }

    //
    // Check each file in the change set.
    // If a file is a part of a specific program,
    // verify that the program has a man page.
    //
    unsigned number_of_errors = 0;
    for (long j = 0; ; ++j)
    {
        fstate_src_ty *src = change_file_nth(cp, j, view_path_first);
        if (!src)
            break;
        nstring filename(src->file_name);

        nstring candidate = extract_progname_details_generous(filename);
        if (!candidate.empty() && programs.member(candidate))
        {
            if (!man_pages.member(candidate))
            {
                sub_context_ty sc;
                sc.var_set_string("File_Name", candidate);
                change_error(cp, &sc, i18n("prog $filename has no man page"));
                ++number_of_errors;
            }
            // only check once per program
            programs.remove(candidate);
        }
    }

    return (number_of_errors == 0);
}


// vim: set ts=8 sw=4 et :
