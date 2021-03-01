//
//      aegis - project change supervisor
//      Copyright (C) 2001, 2003-2006, 2008, 2012 Peter Miller
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

#include <common/ac/assert.h>
#include <common/ac/string.h>

#include <libaegis/patch.h>
#include <libaegis/patch/context.h>
#include <libaegis/patch/format/context.h>
#include <common/trace.h>

static string_ty *star15;


static int
starts_with(string_ty *line, const char *prefix, size_t pfxlen)
{
    return
        (
            line->str_length > pfxlen + 1
        &&
            0 == memcmp(line->str_text, prefix, pfxlen)
        &&
            (line->str_text[pfxlen] == ' ' || line->str_text[pfxlen] == '\t')
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
context_diff_header(patch_context_ty *context)
{
    string_ty       *line;
    int             idx;
    patch_ty        *result;
    string_ty       *s;

    trace(("context_diff_header()\n{\n"));
    if (!star15)
        star15 = str_from_c("***************");
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
    if (starts_with(line, "Index:", 6))
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
    if (starts_with(line, "***", 3))
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
    if (starts_with(line, "---", 3))
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
    // Look for a line which contains exactly 15 stars.
    //
    line = patch_context_getline(context, idx);
    if (!str_equal(line, star15))
        goto oops;

    //
    // Discard all of the header lines, except the row of stars
    // (it's actually part of the first hunk).
    //
    patch_context_discard(context, idx);
    trace(("return %p\n", result));
    trace(("}\n"));
    return result;
}


static int
range(string_ty *line, const char *prefix, int *n1, int *n2)
{
    const char      *cp;
    int             n;

    trace(("range(line = \"%s\", pfx = \"%s\")\n{\n", line->str_text, prefix));
    if (line->str_length < 10)
    {
        oops:
        trace(("return 0;\n}\n"));
        return 0;
    }
    cp = line->str_text;
    if (0 != memcmp(cp, prefix, 3))
        goto oops;
    cp += 3;
    if (*cp++ != ' ')
        goto oops;
    switch (*cp)
    {
    default:
        goto oops;

    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
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

        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            continue;
        }
        break;
    }
    *n1 = n;
    *n2 = n;
    trace(("n1 = %d\n", n));
    if (*cp == ',')
    {
        ++cp;
        switch (*cp)
        {
        default:
            goto oops;

        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
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

            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                continue;
            }
            break;
        }
        *n2 = n;
        trace(("n2 = %d\n", n));

        if (*n1 > *n2)
            goto oops;
    }
    if (*cp++ != ' ')
        goto oops;
    if (0 != memcmp(cp, prefix, 4))
        goto oops;
    cp += 4;
    if (*cp)
        goto oops;
    trace(("return 1;\n}\n"));
    return 1;
}


static int
line_append(patch_line_list_ty *pllp, string_ty *line)
{
    patch_line_type type;
    string_ty       *value;

    if (line->str_length < 2)
        return 0;
    if (line->str_text[1] != ' ')
        return 0;
    type = patch_line_type_unchanged;
    switch (line->str_text[0])
    {
    default:
        return 0;

    case ' ':
        type = patch_line_type_unchanged;
        break;

    case '!':
        type = patch_line_type_changed;
        break;

    case '+':
        type = patch_line_type_inserted;
        break;

    case '-':
        type = patch_line_type_deleted;
        break;
    }
    value = str_n_from_c(line->str_text + 2, line->str_length - 2);
    patch_line_list_append(pllp, type, value);
    str_free(value);
    return 1;
}


static patch_hunk_ty *
context_diff_hunk(patch_context_ty *context)
{
    int             before1;
    int             before2;
    int             before_list;
    int             after1;
    int             after2;
    int             after_list;
    string_ty       *line;
    int             lino;
    patch_hunk_ty   *php;
    int             idx;
    size_t          j;
    patch_line_ty   *plp;

    trace(("context_diff_hunk()\n{\n"));
    assert(star15);
    line = patch_context_getline(context, 0);
    if (!line || !str_equal(line, star15))
    {
        trace(("no start 15 line\n"));
        trace(("return 0;\n}\n"));
        return 0;
    }
    idx = 1;
    php = patch_hunk_new();

    //
    // Look for a line of the form
    //      *** N ****
    // or
    //      *** N,N ****
    //
    trace(("mark\n"));
    line = patch_context_getline(context, idx++);
    if (!line)
    {
        oops:
        patch_hunk_delete(php);
        trace(("return 0;\n}\n"));
        return 0;
    }
    trace(("line = \"%s\"\n", line->str_text));
    if (!range(line, "****", &before1, &before2))
        goto oops;
    php->before.start_line_number = before1;
    line = patch_context_getline(context, idx);
    if (!line)
        goto oops;
    trace(("line = \"%s\"\n", line->str_text));
    before_list = (line->str_length >= 2 && line->str_text[1] == ' ');
    if (before_list)
    {
        trace(("mark\n"));
        // collect a bunch of lines
        for (lino = before1; lino <= before2; ++lino)
        {
            trace(("mark\n"));
            line = patch_context_getline(context, idx++);
            if (!line)
                goto oops;
            trace(("line = \"%s\"\n", line->str_text));
            if (!line_append(&php->before, line))
                goto oops;
        }
        trace(("mark\n"));
    }

    //
    // Look for a line of the form
    //      --- N ----
    // or
    //      --- N,N ----
    //
    trace(("mark\n"));
    line = patch_context_getline(context, idx++);
    if (!line)
        goto oops;
    trace(("line = \"%s\"\n", line->str_text));
    if (!range(line, "----", &after1, &after2))
        goto oops;
    trace(("mark\n"));
    php->after.start_line_number = after1;
    trace(("mark\n"));
    line = patch_context_getline(context, idx);
    if (!line)
    {
        after_list = 0;
    }
    else
    {
        trace(("line = \"%s\"\n", line->str_text));
        after_list = (line->str_length >= 2 && line->str_text[1] == ' ');
    }
    trace(("mark\n"));
    if (after_list)
    {
        trace(("mark\n"));
        // collect a bunch of lines
        for (lino = after1; lino <= after2; ++lino)
        {
            trace(("mark\n"));
            line = patch_context_getline(context, idx++);
            if (!line)
                goto oops;
            trace(("line = \"%s\"\n", line->str_text));
            if (!line_append(&php->after, line))
                goto oops;
        }
        trace(("mark\n"));
    }

    //
    // We have a viable hunk, take them out of the context because
    // we won't need to backtrack them any more.
    //
    trace(("mark\n"));
    patch_context_discard(context, idx);

    //
    // Now we have all the lines, fill in the blanks.  This happens
    // when there is only deletes or only inserts in a hunk.
    //
    if (!before_list)
    {
        trace(("mark\n"));
        for (j = 0; j < php->after.length; ++j)
        {
            plp = &php->after.item[j];
            if (plp->type == patch_line_type_unchanged)
            {
                trace(("mark\n"));
                patch_line_list_append(&php->before, plp->type, plp->value);
            }
        }
        trace(("mark\n"));
    }
    if (!after_list)
    {
        trace(("mark\n"));
        for (j = 0; j < php->before.length; ++j)
        {
            plp = &php->before.item[j];
            if (plp->type == patch_line_type_unchanged)
            {
                patch_line_list_append(&php->after, plp->type, plp->value);
            }
        }
    }

    //
    // In the limiting case, using the diff -C0 flag, inserts
    // and deletes are off by one.  They mean "insert after" and
    // "delete after", but we need them to mean "insert before" and
    // "delete before".
    //
    if (php->before.start_line_number && php->before.length == 0)
        php->before.start_line_number++;
    if (php->after.start_line_number && php->after.length == 0)
        php->after.start_line_number++;

    trace(("mark\n"));
    trace(("return %p\n", php));
    trace(("}\n"));
    return php;
}


patch_format_ty patch_format_context =
{
    "context diff",
    context_diff_header,
    context_diff_hunk,
};


// vim: set ts=8 sw=4 et :
