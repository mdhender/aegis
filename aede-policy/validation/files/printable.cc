//
// aegis - project change supervisor
// Copyright (C) 2006-2008, 2010, 2012 Peter Miller
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
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <common/ac/assert.h>
#include <common/ac/ctype.h>
#include <common/ac/string.h>

#include <libaegis/attribute.h>
#include <libaegis/change/file.h>
#include <libaegis/fstate.fmtgen.h>
#include <libaegis/input/file.h>
#include <libaegis/os.h>
#include <libaegis/sub.h>

#include <aede-policy/validation/files/printable.h>


validation_files_printable::~validation_files_printable()
{
}


validation_files_printable::validation_files_printable()
{
}


validation::pointer
validation_files_printable::create(void)
{
    return pointer(new validation_files_printable());
}


bool
validation_files_printable::check_binaries()
    const
{
    return false;
}


static bool
international_character_set(fstate_src_ty *src)
{
    //
    // If the content-type attribute doesn't exist, or no character set
    // is specified by the content-type, plain ascii is assumed.
    //
    attributes_ty *ap = attributes_list_find(src->attribute, "content-type");
    if (!ap || !ap->value)
        return false;
    const char *cp = strstr(ap->value->str_text, "charset=");
    if (!cp)
        return false;
    cp += 8;
    return (0 != strcmp(cp, "ascii") && 0 != strcmp(cp, "us-ascii"));
}


bool
validation_files_printable::check(change::pointer cp, fstate_src_ty *src)
{
    //
    // Note: we return true if the file is acceptable to the policy
    // (i.e. has no unprintable characters).  We return false if the
    // file is unacceptable.
    //
    nstring path(cp->file_path(src));
    assert(!path.empty());
    if (path.empty())
        return true;

    os_become_orig();
    bool result = true;
    input ip = input_file_open(path);
    if
    (
        nstring(src->file_name).ends_with(".po")
    ||
        international_character_set(src)
    )
    {
        //
        // An obvious exception is the message translation files, which
        // are supposed to contain character sets other than ASCII.
        //
        for (;;)
        {
            int c = ip->getch();
            if (c < 0)
                break;
            if (c == 0)
            {
                sub_context_ty sc;
                sc.var_set_string("File_Name", src->file_name);
                change_error(cp, &sc, i18n("$filename: is binary"));
                result = false;
                break;
            }
        }
    }
    else
    {
        int line_number = 1;
        for (;;)
        {
            int c = ip->getch();
            if (c < 0)
                break;
            if (c == 0)
            {
                sub_context_ty sc;
                sc.var_set_string("File_Name", src->file_name);
                change_error(cp, &sc, i18n("$filename: is binary"));
                result = false;
                break;
            }
            unsigned char uc = c;
            // Using the "C" locale.
            if (!isprint(uc) && !isspace(uc))
            {
                sub_context_ty sc;
                sc.var_set_string("File_Name", src->file_name);
                sc.var_set_long("Number", line_number);
                sc.var_optional("Number");
                change_error(cp, &sc, i18n("$filename: is unprintable"));
                result = false;
                break;
            }
            if (c == '\n')
                ++line_number;
        }
    }
    ip.close();
    os_become_undo();
    return result;
}


// vim: set ts=8 sw=4 et :
