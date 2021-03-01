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
 * MANIFEST: functions to manipulate counts
 */

#include <ac/stdio.h>
#include <ac/string.h>

#include <count.h>
#include <error.h>


typedef struct data_t data_t;
struct data_t
{
    long            code_lines;
    long            comment_lines;
    long            blank_lines;

    int             line_has_code;
    int             line_has_comment;
    int             line_length;
};


static int
getc_special(FILE *fp, data_t *data)
{
    int             c;

    c = getc(fp);
    switch (c)
    {
    case EOF:
	if (data->line_length)
	{
	    if (data->line_has_code)
		data->code_lines++;
	    else if (data->line_has_comment)
		data->comment_lines++;
	    else
		data->blank_lines++;
	}
	break;

    case '\n':
	if (data->line_has_code)
	    data->code_lines++;
	else if (data->line_has_comment)
	    data->comment_lines++;
	else
	    data->blank_lines++;
	data->line_has_code = 0;
	data->line_has_comment = 0;
	data->line_length = 0;
	break;

    default:
	data->line_length++;
	break;
    }
    return c;
}


static void
generic(FILE *fp, data_t *data)
{
    for (;;)
    {
	int             c;

	c = getc_special(fp, data);
	switch (c)
	{
	case EOF:
	    break;

	case ' ':
	case '\b':
	case '\f':
	case '\n':
	case '\r':
	case '\t':
	case '\v':
	    continue;

	case '#':
	    data->line_has_comment = 1;
	    continue;

	default:
	    data->line_has_code = 1;
	    continue;
	}
	break;
    }
}


static void
C(FILE *fp, data_t *data)
{
    for (;;)
    {
	int             c;

	c = getc_special(fp, data);
	loop:
	switch (c)
	{
	case EOF:
	    break;

	case ' ':
	case '\b':
	case '\f':
	case '\n':
	case '\r':
	case '\t':
	case '\v':
	    continue;

	case '/':
	    c = getc_special(fp, data);
	    if (c == EOF)
		break;
	    if (c == '/')
	    {
		/* C++ comment */
		data->line_has_comment = 1;
		for (;;)
		{
		    c = getc_special(fp, data);
		    if (c == EOF || c == '\n')
			break;
		}
		continue;
	    }
	    if (c == '*')
	    {
		/* C comment */
		for (;;)
		{
		    for (;;)
		    {
			data->line_has_comment = 1;
			c = getc_special(fp, data);
			if (c == EOF || c == '*')
			    break;
		    }
		    for (;;)
		    {
			data->line_has_comment = 1;
			c = getc_special(fp, data);
			if (c != '*')
			    break;
		    }
		    if (c == EOF || c == '/')
			break;
		}
		continue;
	    }
	    data->line_has_code = 1;
	    goto loop;

	case '"':
	    for (;;)
	    {
		data->line_has_code = 1;
		c = getc_special(fp, data);
		if (c == EOF || c == '"' || c == '\n')
		    break;
		if (c == '\\')
		    getc_special(fp, data);
	    }
	    continue;

	case '\'':
	    for (;;)
	    {
		data->line_has_code = 1;
		c = getc_special(fp, data);
		if (c == EOF || c == '\'' || c == '\n')
		    break;
		if (c == '\\')
		    getc_special(fp, data);
	    }
	    continue;

	default:
	    data->line_has_code = 1;
	    continue;
	}
	break;
    }
}


static void
roff(FILE *fp, data_t *data)
{
    for (;;)
    {
	int             c;

	c = getc_special(fp, data);
	switch (c)
	{
	case EOF:
	    break;

	case ' ':
	case '\b':
	case '\f':
	case '\n':
	case '\r':
	case '\t':
	case '\v':
	    continue;

	case '.':
	case '\'':
	    data->line_has_code = 1;
	    if (data->line_length > 1)
		continue;
	    for (;;)
	    {
		c = getc_special(fp, data);
		switch (c)
		{
		case ' ':
		case '\t':
		    continue;

		default:
		    break;
		}
		break;
	    }
	    if (c != '\\')
		continue;
	    c = getc_special(fp, data);
	    if (c != '"')
		continue;
	    data->line_has_code = 0;
	    data->line_has_comment = 1;
	    for (;;)
	    {
		c = getc_special(fp, data);
		if (c == EOF || c == '\n')
		    break;
	    }
	    continue;

	default:
	    data->line_has_code = 1;
	    continue;
	}
	break;
    }
}


static int
ends_with(const char *haystack, const char *needle)
{
    size_t          haystack_len;
    size_t          needle_len;

    haystack_len = strlen(haystack);
    needle_len = strlen(needle);
    return
    (
	haystack_len >= needle_len
    &&
	0 == memcmp(haystack + haystack_len - needle_len, needle, needle_len)
    );
}


void
count(const char *ifn, const char *ofn)
{
    FILE            *ifp;
    FILE            *ofp;
    static data_t   data_zero;
    data_t	    data;
    void            (*language)(FILE *, data_t *);

    if (ifn)
    {
	ifp = fopen(ifn, "r");
	if (!ifp)
	    nfatal("open %s", ifn);
    }
    else
    {
	ifn = "standard input";
	ifp = stdin;
    }

    language = generic;
    if
    (
	ends_with(ifn, ".c")
    ||
	ends_with(ifn, ".h")
    ||
        ends_with(ifn, ".cook")
    ||
	ends_with(ifn, ".def")
    ||
        ends_with(ifn, ".y")
    ||
        ends_with(ifn, ".rpt")
    )
	language = C;
    if
    (
       	ends_with(ifn, ".so")
    ||
       	ends_with(ifn, ".man")
    ||
       	ends_with(ifn, ".mm")
    ||
       	ends_with(ifn, ".ms")
    ||
       	ends_with(ifn, ".1")
    ||
       	ends_with(ifn, ".5")
    )
	language = roff;
    data = data_zero;
    language(ifp, &data);
    if (ferror(ifp))
	nfatal("read %s", ifn);
    fclose(ifp);

    /*
     * Write the output.
     */
    if (ofn)
    {
	ofp = fopen(ofn, "w");
	if (!ofp)
	    nfatal("open %s", ofn);
    }
    else
    {
	ofn = "standard output";
	ofp = stdout;
    }
    fprintf(ofp, "metrics = [\n");
    fprintf
    (
	ofp,
	"{ name = \"lines\"; value = %ld; },\n",
	data.code_lines + data.comment_lines + data.blank_lines
    );
    fprintf
    (
	ofp,
	"{ name = \"code_lines\"; value = %ld; },\n",
	data.code_lines
    );
    fprintf
    (
	ofp,
	"{ name = \"comment_lines\"; value = %ld; },\n",
	data.comment_lines
    );
    fprintf
    (
	ofp,
	"{ name = \"blank_lines\"; value = %ld; },\n",
	data.blank_lines
    );
    fprintf(ofp, "];\n");
    if (fflush(ofp))
	nfatal("write %s", ofn);
    if (fclose(ofp))
	nfatal("close %s", ofn);
}
