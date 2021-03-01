//
// aegis - project change supervisor
// Copyright (C) 2007, 2008, 2010, 2012 Peter Miller
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
#include <common/ac/string.h>

#include <common/sizeof.h>
#include <libaegis/attribute.h>
#include <libaegis/change/file.h>
#include <libaegis/os.h>

#include <aede-policy/validation/files.h>


bool
validation_files::is_a_text_file(fstate_src_ty *src, change::pointer cp)
{
    //
    // Grab the content type.
    // If it is not present, assume "text/plain"
    //
    attributes_ty *ap = attributes_list_find(src->attribute, "content-type");
    if (!ap || !ap->value)
    {
        nstring path(cp->file_path(src));
        if (path.empty())
            return true;

        os_become_orig();
        nstring mime = os_magic_file(path);
        os_become_undo();

        if (!src->attribute)
            src->attribute = (attributes_list_ty *)attributes_list_type.alloc();
        attributes_list_insert(src->attribute, "content-type", mime.c_str());

        //
        // and get the value we just set,
        // for the benefit of the rest of the code.
        //
        ap = attributes_list_find(src->attribute, "content-type");
        assert(ap);
        if (!ap)
            return true;
    }

    //
    // extract the first semicolon-separated field (often the content
    // type is followed by the charset)
    //
    nstring s(ap->value);
    const char *semi = strchr(s.c_str(), ';');
    if (semi)
        s = nstring(s.c_str(), semi - s.c_str()).trim();

    //
    // If it is a text file, we are done.
    //
    if (s.starts_with("text/") || s == "text")
        return true;

    //
    // We also need to look for scripts (shell scripts, perl scripts, etc)
    // which libmagic will call "application/x-*"
    //
    static const char *scripts[] =
    {
        "awk"
        "bash",
        "csh",
        "gawk"
        "nawk",
        "perl",
        "sh",
        "shellscript",
    };
    for (const char **script = scripts; script < ENDOF(scripts); ++script)
    {
        if (s == nstring::format("application/x-%s", *script))
            return true;
    }

    //
    // assume anything else is binary
    //
    return false;
}


// vim: set ts=8 sw=4 et :
