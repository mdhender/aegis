//
// aegis - project change supervisor
// Copyright (C) 2010, 2012 Peter Miller
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

#include <common/ac/assert.h>
#include <common/ac/string.h>

#include <common/nstring/list.h>
#include <libaegis/attribute.h>
#include <libaegis/change/file.h>
#include <libaegis/input/crlf.h>
#include <libaegis/input/file.h>
#include <libaegis/os.h>
#include <libaegis/sub.h>

#include <aede-policy/validation/files/escapehyphen.h>


validation_files_escape_hyphen::~validation_files_escape_hyphen()
{
}


validation_files_escape_hyphen::validation_files_escape_hyphen()
{
}


validation_files_escape_hyphen::pointer
validation_files_escape_hyphen::create(void)
{
    return pointer(new validation_files_escape_hyphen());
}


bool
validation_files_escape_hyphen::check(change::pointer cp, fstate_src_ty *src)
{
    //
    // Don't perform this check for files marked as being allowed to
    // have unsecaped hyphens.
    //
    if
    (
        !attributes_list_find_boolean
        (
            src->attribute,
            "aede-policy-escape-hyphen",
            !attributes_list_find_boolean
            (
                src->attribute,
                "aede-policy-no-escape-hyphen",
                false
            )
        )
    )
        return true;

    nstring path(cp->file_path(src));
    if
    (
        !path.ends_with(".man")
    &&
        !path.ends_with(".so")
    &&
        !path.gmatch("*.[1-8]")
    )
        return true;
    assert(!path.empty());
    if (path.empty())
        return true;

    bool notice_problems = true;
    os_become_orig();
    bool result = true;
    input ip = input_file_open(path);
    ip = input_crlf::create(ip);
    int state = '\n';
    int line_number = 1;
    for (;;)
    {
        int c = ip->getch();
        if (c < 0)
            break;
        if (c == '\n')
            ++line_number;
        if (state == '\n' && (c == '.' || c == '\''))
        {
            char buf[20];
            char *bp = buf;
            for (;;)
            {
                c = ip->getch();
                if (c < 0)
                {
                    c = '\n';
                    break;
                }
                if (c == '\n')
                {
                    ++line_number;
                    break;
                }
                if (bp < buf + sizeof(buf) - 1)
                    *bp++ = c;
            }
            *bp++ = '\0';
            nstring_list args;
            args.split(buf);
            if (args[0] == "PS" || args[0] == "TS")
                notice_problems = false;
            if (args[0] == "PE" || args[0] == "TE")
                notice_problems = true;
        }
        if (notice_problems)
        {
            if (state == '\\')
            {
                switch (c)
                {
                case '*':
                case 'f':
                case 's':
                    c = ip->getch();
                    switch (c)
                    {
                    case '[':
                        for (;;)
                        {
                            c = ip->getch();
                            if (c == ']' || c == '\n' || c < 0)
                                break;
                        }
                        break;

                    case '(':
                        ip->getch();
                        ip->getch();
                        break;

                    case '\n':
                        ++line_number;
                        break;

                    default:
                        break;
                    }
                    break;

                case '[':
                    for (;;)
                    {
                        c = ip->getch();
                        if (c == '\n')
                        {
                            ++line_number;
                            break;
                        }
                        if (c == ']' || c < 0)
                            break;
                    }
                    break;

                case '(':
                    if ('\n' == ip->getch())
                        ++line_number;
                    if ('\n' == ip->getch())
                        ++line_number;
                    break;

                default:
                    break;
                }
                c = ' ';
            }
            else
            {
                if (c == '-')
                {
                    sub_context_ty sc;
                    sc.var_set_format
                    (
                        "File_Name",
                        "%s: %d",
                        src->file_name->str_text,
                        line_number
                    );
                    change_error
                    (
                        cp,
                        &sc,
                        i18n("$filename: unescaped minus character")
                    );
                    result = false;
                }
            }
        }
        state = c;
    }
    ip.close();
    os_become_undo();
    return result;
}


bool
validation_files_escape_hyphen::check_foreign_copyright(void)
    const
{
    return false;
}


bool
validation_files_escape_hyphen::check_binaries(void)
    const
{
    return false;
}


bool
validation_files_escape_hyphen::check_branches(void)
    const
{
    return false;
}


bool
validation_files_escape_hyphen::check_downloaded(void)
    const
{
    return false;
}


// vim: set ts=8 sw=4 et :
