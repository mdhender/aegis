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
#include <common/ac/ctype.h>
#include <common/ac/string.h>

#include <common/nstring.h>
#include <common/sizeof.h>
#include <libaegis/change/file.h>
#include <libaegis/input/file.h>
#include <libaegis/os.h>
#include <libaegis/sub.h>

#include <aede-policy/validation/files/gpl-version.h>


validation_files_gpl_version::~validation_files_gpl_version()
{
}


validation_files_gpl_version::validation_files_gpl_version(int a_version) :
    version(a_version)
{
}


validation::pointer
validation_files_gpl_version::create(int a_version)
{
    return pointer(new validation_files_gpl_version(a_version));
}


validation::pointer
validation_files_gpl_version::create3(void)
{
    return pointer(new validation_files_gpl_version(3));
}


//
// Be mindful of the commas between the pattern strings.
//
static const char *notice_patterns[] =
{
    "This program is free software; you can redistribute it and/or "
    "modify it under the terms of the GNU General Public License as "
    "published by the Free Software Foundation; either version * of the "
    "License, or (at your option) any later version.",

    "This program is free software; you can redistribute it and/or "
    "modify it under the terms of the GNU Lesser General Public License "
    "as published by the Free Software Foundation; either version * of "
    "the License, or (at your option) any later version.",

    "This program is free software: you can redistribute it and/or "
    "modify it under the terms of the GNU General Public License as "
    "published by the Free Software Foundation, version *",

    "This program is free software: you can redistribute it and/or "
    "modify it under the terms of the GNU Lesser General Public License "
    "as published by the Free Software Foundation, version *",

    "This program is free software: you can redistribute it and/or "
    "modify it under the terms of the GNU General Public License, "
    "version *, as published by the Free Software Foundation.",

    "This program is free software: you can redistribute it and/or "
    "modify it under the terms of the GNU Lesser General Public "
    "License, version *, as published by the Free Software Foundation.",
};


int
validation_files_gpl_version::match(const char *text, long nbytes,
        const char *pattern)
    const
{
    assert(isalnum(pattern[0]));
    size_t pattern_nbytes = strlen(pattern);
    const char *pattern_end = pattern + pattern_nbytes;

    //
    // Find length of first pattern word.
    // We will use the first word to eliminate most false positives.
    //
    size_t pattern_word_len = 1;
    while
    (
        pattern_word_len < pattern_nbytes
    &&
        isalnum((unsigned char)pattern[pattern_word_len])
    )
        ++pattern_word_len;

    const char *buffer = text;
    const char *buffer_end = text + nbytes;
    const char *start_of_number = 0;
    while (buffer < buffer_end)
    {
        const char *w =
            (const char *)
            memmem(buffer, buffer_end - buffer, pattern, pattern_word_len);
        if (!w)
            break;

        const char *buf_p = w + pattern_word_len;
        const char *pat_p = pattern + pattern_word_len;
        bool is_a_match = true;
        while (pat_p < pattern_end)
        {
            if (buf_p >= buffer_end)
            {
                is_a_match = false;
                break;
            }

            unsigned char pc = *pat_p++;
            unsigned char bc = *buf_p++;
            if (pc == '*')
            {
                start_of_number = buf_p - 1;
                if (!isdigit(bc))
                {
                    return 0;
                }
                int n = bc - '0';
                for (;;)
                {
                    bc = *buf_p;
                    if (!isdigit(bc))
                        break;
                    n = n * 10 + bc - '0';
                    ++buf_p;
                }
                if (n == version)
                {
                    // no need to change anything
                    return 0;
                }
            }
            else if (!isalnum(pc))
            {
                if (isalnum(bc))
                {
                    is_a_match = false;
                    break;
                }
                while
                (
                    pat_p < pattern_end
                &&
                    *pat_p != '*'
                &&
                    !isalnum((unsigned char)*pat_p)
                )
                    ++pat_p;
                for (;;)
                {
                    if (buf_p >= buffer_end)
                        break;
                    bc = *buf_p;
                    if
                    (
                        bc == '.'
                    &&
                        buf_p + 3 <= buffer_end
                    &&
                        buf_p[1] == 'b'
                    &&
                        buf_p[2] == 'r'
                    )
                    {
                        // assume this is a groff ".br" command
                        // to separate address lines
                        buf_p += 3;
                        continue;
                    }
                    if
                    (
                        bc == '<'
                    &&
                        buf_p + 4 <= buffer_end
                    &&
                        (buf_p[1] == 'b' || buf_p[1] == 'B')
                    &&
                        (buf_p[2] == 'r' || buf_p[2] == 'R')
                    &&
                        buf_p[3] == '>'
                    )
                    {
                        // assume this is a HTML "<br>" element
                        // to separate address lines
                        buf_p += 4;
                        continue;
                    }
                    if
                    (
                        bc == 'd'
                    &&
                        buf_p + 3 <= buffer_end
                    &&
                        buf_p[1] == 'n'
                    &&
                        buf_p[2] == 'l'
                    )
                    {
                        // assume this is an M4 comment (like
                        // configure.ac uses).
                        buf_p += 3;
                        continue;
                    }
                    if (isalnum(bc))
                        break;
                    ++buf_p;
                }
            }
            else
            {
                if (isupper(pc))
                    pc = tolower(pc);
                if (isupper(bc))
                    bc = tolower(bc);
                if (pc != bc)
                {
                    is_a_match = false;
                    break;
                }
            }
        }
        if (is_a_match)
        {
            int line_number = 1;
            for (buffer = text; buffer < start_of_number; ++buffer)
            {
                if (*buffer == '\n')
                    ++line_number;
            }
            return line_number;
        }

        buffer = w + 1;
    }
    return 0;
}


bool
validation_files_gpl_version::check_binaries(void)
    const
{
    return false;
}


bool
validation_files_gpl_version::check(change::pointer cp, fstate_src_ty *src)
{
    nstring path(cp->file_path(src));
    assert(!path.empty());
    if (path.empty())
        return true;

    //
    // Read the first few kB of the file.
    //
    os_become_orig();
    input ip = input_file_open(path);
    char buffer[1 << 14];
    size_t nbytes = ip->read(buffer, sizeof(buffer));
    ip.close();
    os_become_undo();

    //
    // Check against each of the patterns
    //
    for
    (
        const char **pattern_ptr = notice_patterns;
        pattern_ptr < ENDOF(notice_patterns);
        ++pattern_ptr
    )
    {
        int line_number = match(buffer, nbytes, *pattern_ptr);
        if (line_number > 0)
        {
            sub_context_ty sc;
            sc.var_set_format
            (
                "File_Name",
                "%s: %d",
                src->file_name->str_text,
                line_number
            );
            sc.var_set_long("Suggest", version);
            sc.var_optional("Suggest");
            change_error(cp, &sc, i18n("$filename: old GPL version"));
            return false;
        }
    }

    return true;
}


// vim: set ts=8 sw=4 et :
