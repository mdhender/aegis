//
//      aegis - project change supervisor
//      Copyright (C) 2001-2006, 2008, 2012 Peter Miller
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

#include <common/ac/string.h>

#include <common/error.h>
#include <libaegis/patch.h>
#include <libaegis/patch/context.h>
#include <libaegis/patch/format/uni.h>
#include <common/trace.h>


static int
starts_with(string_ty *line, const char *prefix)
{
    size_t          pfxlen;

    pfxlen = strlen(prefix);
    return
    (
        line->str_length > pfxlen + 1
    &&
        0 == memcmp(line->str_text, prefix, pfxlen)
    &&
        (
            line->str_text[pfxlen] == ' '
        ||
            line->str_text[pfxlen] == '\t'
        )
    );
}


static string_ty *
second_word(string_ty *line)
{
    const char      *cp;
    const char      *ep;

    cp = line->str_text;
    while (*cp && *cp != ' ' && *cp != '\t')
        ++cp;
    while (*cp && (*cp == ' ' || *cp == '\t'))
        ++cp;
    ep = cp;
    while (*ep && *ep != ' ' && *ep != '\t')
        ++ep;
    return str_n_from_c(cp, ep - cp);
}


static patch_ty *
uni_diff_header(patch_context_ty *context)
{
    string_ty       *line;
    int             idx;
    patch_ty        *result;
    string_ty       *s;

    trace(("uni_diff_header()\n{\n"));
    result = patch_new();

    //
    // Look for the optional index line.
    //
    line = patch_context_getline(context, 0);
    if (!line)
    {
        oops:
        patch_delete(result);
        trace(("return 0\n"));
        trace(("}\n"));
        return 0;
    }
    idx = 0;
    if (starts_with(line, "Index:"))
    {
        s = second_word(line);
        result->name.push_back(s);
        str_free(s);
        idx++;
    }

    //
    // Look for the optional before line.
    //
    line = patch_context_getline(context, idx);
    if (!line)
        goto oops;
    static string_ty *dev_null;
    if (!dev_null)
        dev_null = str_from_c("/dev/null");
    if (starts_with(line, "---"))
    {
        s = second_word(line);
        if (!str_equal(s, dev_null))
            result->name.push_back(s);
        str_free(s);
        idx++;
    }

    //
    // Look for the optional after line.
    //
    line = patch_context_getline(context, idx);
    if (!line)
        goto oops;
    if (starts_with(line, "+++"))
    {
        s = second_word(line);
        if (!str_equal(s, dev_null))
            result->name.push_back(s);
        str_free(s);
        idx++;
    }

    //
    // If there are no names at all,
    // this isn't one of our files.
    //
    if (result->name.nstrings == 0)
        goto oops;

    //
    // Look for a line which starts with "@@"
    //
    line = patch_context_getline(context, idx);
    if (!starts_with(line, "@@"))
        goto oops;

    //
    // Discard all of the header lines, except @@ line
    // (it's actually part of the first hunk).
    //
    patch_context_discard(context, idx);
    trace(("return %p\n", result));
    trace(("}\n"));
    return result;
}


static const char *
getnum(const char *cp, int *np)
{
    int             n;

    switch (*cp)
    {
    default:
        return 0;

    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
        break;
    }
    n = 0;
    for (;;)
    {
        n = n * 10 + *cp++ - '0';
        switch (*cp)
        {
        default:
            break;

        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            continue;
        }
        break;
    }
    *np = n;
    return cp;
}


static int
range(string_ty *line, int *b1, int *b2, int *a1, int *a2)
{
    const char      *cp;

    //
    // The lines are of the form
    //  @@ -N[,N] +N[,N] @@
    //
    // The first pair of numbers is a start line and a line count
    // (the count defaults to 1 if not given) of the first file,
    // the second pair of numbers applies to the second file.
    //
    trace(("range(line = \"%s\")\n{\n", line->str_text));
    if (line->str_length < 11)
    {
        oops:
        trace(("return 0;\n}\n"));
        return 0;
    }
    cp = line->str_text;
    if (*cp++ != '@')
        goto oops;
    if (*cp++ != '@')
        goto oops;
    if (*cp++ != ' ')
        goto oops;
    if (*cp++ != '-')
        goto oops;

    cp = getnum(cp, b1);
    if (!cp)
        goto oops;
    if (*cp == ',')
    {
        ++cp;
        cp = getnum(cp, b2);
        if (!cp)
            goto oops;
    }
    else
        *b2 = 1;

    if (*cp++ != ' ')
        goto oops;
    if (*cp++ != '+')
        goto oops;

    cp = getnum(cp, a1);
    if (!cp)
        goto oops;
    if (*cp == ',')
    {
        ++cp;
        cp = getnum(cp, a2);
        if (!cp)
            goto oops;
    }
    else
        *a2 = 1;

    if (*cp++ != ' ')
        goto oops;
    if (*cp++ != '@')
        goto oops;
    if (*cp++ != '@')
        goto oops;
    if (*cp)
        goto oops;
    trace(("return 1;\n}\n"));
    return 1;
}


static patch_hunk_ty *
uni_diff_hunk(patch_context_ty *context)
{
    int             before1, num_before;
    int             after1, num_after;
    string_ty       *line;
    patch_hunk_ty   *php;
    int             idx;

    trace(("uni_diff_hunk()\n{\n"));
    line = patch_context_getline(context, 0);
    if (!line)
    {
        oops:
        trace(("return 0;\n}\n"));
        return 0;
    }
    if (!range(line, &before1, &num_before, &after1, &num_after))
        goto oops;
    idx = 1;

    php = patch_hunk_new();
    php->before.start_line_number = before1;
    php->after.start_line_number = after1;

    while (num_before > 0 || num_after > 0)
    {
        patch_line_type type;
        string_ty       *value;

        trace(("num_before=%d\n", num_before));
        trace(("num_after =%d\n", num_after));
        line = patch_context_getline(context, idx++);
        if (!line)
            goto oops;
        trace(("line = \"%s\"\n", line->str_text));
        type = patch_line_type_unchanged;
        switch (line->str_text[0])
        {
        default:
            goto oops;

        case ' ':
            break;

        case '-':
            type = patch_line_type_deleted;
            break;

        case '+':
            type = patch_line_type_inserted;
            break;
        }
        value = str_n_from_c(line->str_text + 1, line->str_length - 1);
        if (type != patch_line_type_inserted)
        {
            if (num_before <= 0)
                goto oops;
            patch_line_list_append(&php->before, type, value);
            --num_before;
        }
        if (type != patch_line_type_deleted)
        {
            if (num_after <= 0)
                goto oops;
            patch_line_list_append(&php->after, type, value);
            --num_after;
        }
        str_free(value);
        trace(("mark\n"));
    }

    //
    // In the limiting case, using the diff -U0 flag, inserts
    // and deletes are off by one.      They mean "append after" and
    // "delete after", but we need them to mean "insert before" and
    // "delete before".
    //
    if (php->before.start_line_number && php->before.length == 0)
        php->before.start_line_number++;
    if (php->after.start_line_number && php->after.length == 0)
        php->after.start_line_number++;

    //
    // We have a viable hunk, take them out of the context because
    // we won't need to backtrack them any more.
    //
    trace(("mark\n"));
    patch_context_discard(context, idx);

    trace(("mark\n"));
    trace(("return %p\n", php));
    trace(("}\n"));
    return php;
}


patch_format_ty patch_format_uni =
{
    "uni diff",
    uni_diff_header,
    uni_diff_hunk,
};


// vim: set ts=8 sw=4 et :
