//
// aegis - project change supervisor
// Copyright (C) 2007, 2008, 2010-2012 Peter Miller
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
#include <common/ac/stdio.h>
#include <common/ac/stdlib.h>
#include <common/ac/string.h>

#include <common/gmatch.h>
#include <common/nstring/list.h>
#include <libaegis/attribute.h>
#include <libaegis/change.h>
#include <libaegis/change/branch.h>
#include <libaegis/change/file.h>
#include <libaegis/cstate.fmtgen.h>
#include <libaegis/input/file_text.h>
#include <libaegis/os.h>
#include <libaegis/sub.h>

#include <aede-policy/validation/files/line_length.h>


validation_files_line_length::~validation_files_line_length()
{
}


validation_files_line_length::validation_files_line_length(int linlen) :
    line_length(linlen > 0 ? linlen : 80)
{
}


validation::pointer
validation_files_line_length::create(int linlen)
{
    return pointer(new validation_files_line_length(linlen));
}


validation::pointer
validation_files_line_length::create80(void)
{
    return pointer(new validation_files_line_length(80));
}


bool
validation_files_line_length::check_branches()
    const
{
    return false;
}


bool
validation_files_line_length::check_downloaded()
    const
{
    return false;
}


bool
validation_files_line_length::check_foreign_copyright()
    const
{
    return false;
}


bool
validation_files_line_length::check_binaries()
    const
{
    return false;
}


static int tabstop;
static int textwidth;


static void
grope(const nstring_list &settings)
{
    for (size_t j = 0; j < settings.size(); ++j)
    {
        nstring s = settings[j];
        const char *eq = strchr(s.c_str(), '=');
        if (!eq)
            continue;
        nstring name(s.c_str(), eq - s.c_str());
        nstring value(eq + 1);
        if (name == "textwidth" || name == "tw")
        {
            long n = value.to_long();
            if (n < 1)
                n = 80;
            textwidth = n;
            continue;
        }
        if (name == "tabstop" || name == "ts")
        {
            long n = value.to_long();
            if (n < 1)
                n = 1;
            tabstop = n;
            continue;
        }
    }
}


static inline bool
safe_isspace(char c)
{
    return isspace((unsigned char)c);
}


static void
read_mode_lines(const char *filename)
{
    FILE *fp = fopen(filename, "r");
    if (!fp)
        return;
    for (;;)
    {
        char line[1000];
        if (!fgets(line, sizeof(line), fp))
            break;
        // first form (don't match self!)
        char *p = strstr(line, " vi" "m: ");
        if (p)
            p += 6;
        else
        {
            p = strstr(line, " v" "i: ");
            if (p)
                p += 5;
            else
                continue;
        }
        while (*p && safe_isspace(*p))
            ++p;

        // first form
        if (memcmp(p, "set ", 4) || memcmp(p, "se ", 3))
        {
            p += 3;
            // terminates at first colon
            // (actually, first non-escaped colon)
            char *colon = strchr(p, ':');
            if (colon)
            {
                nstring set = nstring(p, colon - p);
                nstring_list settings;
                settings.split(set);
                grope(settings);
            }
            continue;
        }

        // second form
        // terminates at first white space
        char *colon = p;
        while (*colon && !safe_isspace(*colon))
            ++colon;
        nstring set(p, colon - p);
        nstring_list settings;
        settings.split(set, ":");
        grope(settings);
    }
    fclose(fp);
}


static int
calc_line_length(const nstring &text)
{
    const char *cp = text.c_str();
    const char *ep = cp + text.size();
    int column = 0;
    while (cp < ep)
    {
        unsigned char c = *cp++;
        if (c == '\t')
            column = ((column / tabstop) + 1) * tabstop;
        else
            ++column;
    }
    return column;
}


bool
validation_files_line_length::check(change::pointer cp, fstate_src_ty *src)
{
    //
    // Tests have notoriously long lines.
    //
    if (gmatch("test/*/*.sh", src->file_name->str_text))
        return true;

    //
    // See if the file contains a mode line that indicates a different length.
    //
    textwidth = line_length;
    tabstop = 8;
    read_mode_lines(src->file_name->str_text);

    //
    // Don't perform this check for files marked as being allowed to
    // have really long lines.
    //
    textwidth =
        attributes_list_find_integer
        (
            src->attribute,
            "aede-policy-line-length",
            textwidth
        );
    if (textwidth <= 0)
        return true;

    nstring path(cp->file_path(src));
    assert(!path.empty());
    if (path.empty())
        return true;
    os_become_orig();
    bool ok = true;
    input ip = input_file_text_open(path);
    int line_number = 0;
    for (;;)
    {
        nstring text;
        if (!ip->one_line(text))
            break;
        ++line_number;
        int ll = calc_line_length(text);
        if (ll > textwidth)
        {
            sub_context_ty sc;
            sc.var_set_format
            (
                "File_Name",
                "%s: %d",
                src->file_name->str_text,
                line_number
            );
            sc.var_set_long("Number", ll - textwidth);
            sc.var_optional("Number");
            change_error(cp, &sc, "$filename: line too long, by $number");
            ok = false;
        }
    }
    ip.close();
    os_become_undo();
    return ok;
}


// vim: set ts=8 sw=4 et :
