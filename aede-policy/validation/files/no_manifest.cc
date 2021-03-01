//
// aegis - project change supervisor
// Copyright (C) 2007, 2008 Peter Miller
// Copyright (C) 2007 Walter Franzini
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

#include <common/error.h> // for assert
#include <libaegis/attribute.h>
#include <libaegis/change/file.h>
#include <libaegis/input/file_text.h>
#include <libaegis/os.h>
#include <libaegis/sub.h>

#include <aede-policy/validation/files/no_manifest.h>


// Beware: Must not match itself.
static nstring bad("MANIFEST" ":");


validation_files_no_manifest::~validation_files_no_manifest()
{
}


validation_files_no_manifest::validation_files_no_manifest()
{
}


bool
validation_files_no_manifest::check_branches()
    const
{
    return false;
}


bool
validation_files_no_manifest::check_downloaded()
    const
{
    return false;
}


bool
validation_files_no_manifest::check_foreign_copyright()
    const
{
    return false;
}


bool
validation_files_no_manifest::check_binaries()
    const
{
    return false;
}


static bool
line_is_ok(const nstring &text)
{
    // Be very careful: this test must not match itself.
    //
    // (It's OK to use the memmem function, even though it is a GNU
    // glibc extension, beause there is an implementation provided in
    // the Aegis sources in the common/ac/string/memmem.cc file.)
    //
    return (0 == memmem(text.c_str(), text.size(), bad.c_str(), bad.size()));
}


bool
validation_files_no_manifest::check(change::pointer cp, fstate_src_ty *src)
{
    //
    // Don't perform this check for files marked as being allowed to
    // have the manifest line.
    //
    long ok =
        attributes_list_find_boolean
        (
            src->attribute,
            "aede-policy-manifest-allowed",
            !attributes_list_find_boolean
            (
                src->attribute,
                "aede-policy-no-manifest",
                true
            )
        );
    if (ok)
        return true;

    nstring path(change_file_path(cp, src));
    assert(!path.empty());
    if (path.empty())
        return true;
    os_become_orig();
    ok = true;
    input ip = input_file_text_open(path);
    int line_number = 0;
    for (;;)
    {
        nstring text;
        if (!ip->one_line(text))
            break;
        ++line_number;
        if (!line_is_ok(text))
        {
            sub_context_ty sc;
            sc.var_set_format
            (
                "File_Name",
                "%s: %d",
                src->file_name->str_text,
                line_number
            );
            nstring msg = "$filename: line contains " + bad.quote_c();
            change_error(cp, &sc, msg.c_str());
            ok = false;
        }
    }
    ip.close();
    os_become_undo();
    return ok;
}
