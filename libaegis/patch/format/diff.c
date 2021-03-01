/*
 *	aegis - project change supervisor
 *	Copyright (C) 2002 Peter Miller;
 *	All rights reserved.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * MANIFEST: functions to manipulate diffs
 */

#include <ac/stdlib.h>
#include <ac/string.h>

#include <error.h>
#include <patch.h>
#include <patch/context.h>
#include <patch/file.h>
#include <patch/format/diff.h>
#include <trace.h>


static int starts_with _((string_ty *, const char *));

static int
starts_with(line, prefix)
    string_ty	    *line;
    const char	    *prefix;
{
    size_t	    pfxlen;

    pfxlen = strlen(prefix);
    return
	(
	    line->str_length > pfxlen + 1
	&&
	    0 == memcmp(line->str_text, prefix, pfxlen)
	&&
	    (line->str_text[pfxlen] == ' ' || line->str_text[pfxlen] == '\t')
	);
}


static int parse_command _((string_ty *, long *, long *, char *, long *,
    long *));

static int
parse_command(line, b1, b2, c, a1, a2)
    string_ty	    *line;
    long	    *b1;
    long	    *b2;
    char	    *c;
    long	    *a1;
    long	    *a2;
{
    char	    *s;
    char	    *end;

    /*
     * Get the first range.
     */
    s = line->str_text;
    *b1 = strtol(s, &end, 10);
    if (s == end)
	return 0;
    s = end;

    if (*s == ',')
    {
	++s;
	*b2 = strtol(s, &end, 10);
	if (s == end)
	    return 0;
	s = end;
    }
    else
	*b2 = *b1;

    /*
     * Get the command.
     */
    if (!strchr("acd", *s))
	return 0;
    *c = *s++;

    /*
     * Get the second range.
     */
    *a1 = strtol(s, &end, 10);
    if (s == end)
	return 0;
    s = end;

    if (*s == ',')
    {
	++s;
	*a2 = strtol(s, &end, 10);
	if (s == end)
	    return 0;
	s = end;
    }
    else
	*a2 = *a1;

    if (*c == 'a' && *b1 != *b2)
	return 0;
    if (*c == 'd' && *a1 != *a2)
	return 0;

    /*
     * Make sure there is no junk on the end of the line.
     */
    return !*s;
}


static string_ty *second_word _((string_ty *));

static string_ty *
second_word(line)
	string_ty	*line;
{
	const char	*cp;
	const char	*ep;

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


static patch_ty *diff_header _((patch_context_ty *));

static patch_ty *
diff_header(context)
    patch_context_ty *context;
{
    string_ty	    *line;
    int		    idx;
    patch_ty	    *result;
    string_ty	    *s;
    long	    b1, b2, a1, a2;
    char	    cmd;

    trace(("diff_header()\n{\n"));
    result = patch_new();

    /*
     * Look for the optional index line.
     */
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
	string_list_append(&result->name, s);
	str_free(s);
	idx++;
    }

    /*
     * Look for a diff -r line, with two file names on it.
     * This may be the only clue we get as to the file names.
     */
    line = patch_context_getline(context, idx);
    if (starts_with(line, "diff"))
    {
	string_list_ty	wl;
	size_t		j;

	str2wl(&wl, line, 0, 1);
	for (j = 1; j < wl.nstrings; ++j)
	    if (wl.string[j]->str_text[0] != '-')
		break;
	if (j + 2 == wl.nstrings)
	{
	    string_list_append(&result->name, wl.string[j]);
	    string_list_append(&result->name, wl.string[j + 1]);

	    /*
	     * Get next line, we've used this one.
	     */
	    line = patch_context_getline(context, idx);
	}
	string_list_destructor(&wl);
    }

    /*
     * Look for a line which contains one of our commands
     */
    if (!parse_command(line, &b1, &b2, &cmd, &a1, &a2))
	goto oops;

    /*
     * Discard all of the header lines, except the command line
     * (it's actually part of the first hunk).
     */
    patch_context_discard(context, idx);
    trace(("return %08lX\n", (long)result));
    trace(("}\n"));
    return result;
}


static patch_hunk_ty *diff_hunk _((patch_context_ty *));

static patch_hunk_ty *
diff_hunk(context)
    patch_context_ty *context;
{
    long	    before1;
    long	    before2;
    long	    after1;
    long	    after2;
    char	    cmd;
    string_ty	    *line;
    patch_hunk_ty   *php;
    int		    idx;
    static string_ty *minus;
    long	    j;
    string_ty	    *value;

    trace(("diff_hunk()\n{\n"));
    line = patch_context_getline(context, 0);
    if (!line)
    {
	oops:
	trace(("return 0;\n}\n"));
	return 0;
    }
    if (!parse_command(line, &before1, &before2, &cmd, &after1, &after2))
	goto oops;
    idx = 1;

    php = patch_hunk_new();
    php->before.start_line_number = before1;
    php->after.start_line_number = after1;

    switch (cmd)
    {
    case 'a':
	php->before.start_line_number++;
	for (j = after1; j <= after2; ++j)
	{
	    line = patch_context_getline(context, idx++);
	    if (!line)
		goto oops;
	    if
	    (
		line->str_length < 2
	    ||
		line->str_text[0] != '>'
	    ||
		line->str_text[1] != ' '
	    )
		goto oops;
	    value = str_n_from_c(line->str_text + 2, line->str_length - 2);
	    patch_line_list_append
	    (
		&php->after,
		patch_line_type_inserted,
		value
	    );
	    str_free(value);
	}
	break;

    case 'c':
	for (j = before1; j <= before2; ++j)
	{
	    line = patch_context_getline(context, idx++);
	    if (!line)
		goto oops;
	    if
	    (
		line->str_length < 2
	    ||
		line->str_text[0] != '<'
	    ||
		line->str_text[1] != ' '
	    )
		goto oops;
	    value = str_n_from_c(line->str_text + 2, line->str_length - 2);
	    patch_line_list_append
	    (
		&php->before,
		patch_line_type_deleted,
		value
	    );
	    str_free(value);
	}
	line = patch_context_getline(context, idx++);
	if (!line)
	    goto oops;
	if (!minus)
	    minus = str_from_c("---");
	if (!str_equal(line, minus))
	    goto oops;
	for (j = after1; j <= after2; ++j)
	{
	    line = patch_context_getline(context, idx++);
	    if (!line)
		goto oops;
	    if
	    (
		line->str_length < 2
	    ||
		line->str_text[0] != '>'
	    ||
		line->str_text[1] != ' '
	    )
		goto oops;
	    value = str_n_from_c(line->str_text + 2, line->str_length - 2);
	    patch_line_list_append
	    (
		&php->after,
		patch_line_type_inserted,
		value
	    );
	    str_free(value);
	}
	break;

    case 'd':
	php->after.start_line_number++;
	for (j = before1; j <= before2; ++j)
	{
	    line = patch_context_getline(context, idx++);
	    if (!line)
		goto oops;
	    if
	    (
		line->str_length < 2
	    ||
		line->str_text[0] != '<'
	    ||
		line->str_text[1] != ' '
	    )
		goto oops;
	    value = str_n_from_c(line->str_text + 2, line->str_length - 2);
	    patch_line_list_append
	    (
		&php->before,
		patch_line_type_deleted,
		value
	    );
	    str_free(value);
	}
	break;

    default:
	assert(0);
	break;
    }

    /*
     * We have a viable hunk, take them out of the context because
     * we won't need to backtrack them any more.
     */
    trace(("mark\n"));
    patch_context_discard(context, idx);

    trace(("return %08lX\n", (long)php));
    trace(("}\n"));
    return php;
}


patch_format_ty patch_format_diff =
{
    "diff",
    diff_header,
    diff_hunk,
};
