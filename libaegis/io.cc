//
//	aegis - project change supervisor
//	Copyright (C) 1991-1999, 2001-2004 Peter Miller;
//	All rights reserved.
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: functions to parse and write fundamental data types
//

#include <ac/ctype.h>
#include <ac/string.h>

#include <io.h>
#include <output.h>
#include <sub.h>
#include <str_list.h>
#include <zero.h>


void
integer_write(output_ty *fp, const char *name, long this_thing, int show)
{
    if (this_thing == INTEGER_NOT_SET && !show)
	return;
    if (name)
	output_fprintf(fp, "%s = ", name);
    if (name && !strcmp(name, "umask"))
	output_fprintf(fp, "0%lo", this_thing & 07777);
    else
    {
	if (this_thing == MAGIC_ZERO)
    	    output_fputs(fp, "ZERO");
	else
    	    output_fprintf(fp, "%ld", this_thing);
    }
    if (name)
	output_fputs(fp, ";\n");
}


void
integer_write_xml(output_ty *fp, const char *name, long this_thing, int show)
{
    if (this_thing == INTEGER_NOT_SET && !show)
	return;
    if (!name)
	name = "integer";
    output_fprintf(fp, "<%s>", name);
    if (!strcmp(name, "umask"))
	output_fprintf(fp, "0%lo", this_thing & 07777);
    else
    {
	if (this_thing == MAGIC_ZERO)
    	    output_fputs(fp, "0");
	else
    	    output_fprintf(fp, "%ld", this_thing);
    }
    output_fprintf(fp, "</%s>", name);
    output_fputc(fp, '\n');
}


void
real_write(output_ty *fp, const char *name, double this_thing, int show)
{
    if (this_thing == REAL_NOT_SET && !show)
	return;
    if (name)
	output_fprintf(fp, "%s = ", name);
    output_fprintf(fp, "%g", this_thing);
    if (name)
	output_fputs(fp, ";\n");
}


void
real_write_xml(output_ty *fp, const char *name, double this_thing, int show)
{
    if (this_thing == REAL_NOT_SET && !show)
	return;
    if (!name)
	name = "real";
    output_fprintf(fp, "<%s>", name);
    output_fprintf(fp, "%g", this_thing);
    output_fprintf(fp, "</%s>", name);
    output_fputc(fp, '\n');
}


void
time_write(output_ty *fp, const char *name, time_t this_thing, int show)
{
    if (this_thing == TIME_NOT_SET && !show)
	return;
    if (name)
	output_fprintf(fp, "%s = ", name);

    //
    // Time is always an arithmetic type, never a structure.
    // This works on every system the author has seen,
    // without loss of precision.
    // (Loss of fractions of a second is acceptable.)
    //
    output_fprintf(fp, "%ld", (long)this_thing);
    if (name)
	output_fprintf(fp, "; /* %.24s */\n", ctime(&this_thing));
}


void
time_write_xml(output_ty *fp, const char *name, time_t this_thing, int show)
{
    if (this_thing == TIME_NOT_SET && !show)
	return;
    if (!name)
	name = "time";
    output_fprintf(fp, "<%s>", name);
    output_fprintf(fp, "%ld", (long)this_thing);
    output_fprintf(fp, "</%s>", name);
    output_fprintf(fp, " <!-- %.24s -->\n", ctime(&this_thing));
}


void
string_write(output_ty *fp, const char *name, string_ty *this_thing)
{
    char            *s;
    int             count;

    if (!this_thing && name)
	return;
    if (name)
	output_fprintf(fp, "%s = ", name);
    output_fputc(fp, '"');
    if (this_thing)
    {
	count = 0;
	for (s = this_thing->str_text; *s; ++s)
	{
	    switch (*s)
	    {
	    case '(':
	    case '[':
	    case '{':
		++count;
		break;

	    case ')':
	    case ']':
	    case '}':
		--count;
		break;
	    }
	}
	if (count > 0)
	    count = -count;
	else
	    count = 0;
	for (s = this_thing->str_text; *s; ++s)
	{
	    unsigned char c = *s;
	    // always in the C locale
	    if (!isprint(c))
	    {
		char	*cp;

		cp = strchr("\bb\ff\nn\rr\tt", c);
		if (cp)
		{
		    output_fputc(fp, '\\');
		    output_fputc(fp, cp[1]);
		    if (c == '\n')
		       	output_fputs(fp, "\\\n");
		}
		else
		{
		    escape:
		    if (isdigit((unsigned char)s[1]))
		    {
			//
			// I'd prefer to use "\\%03o"
			// but that isn't entirely
			// portable (the glibc people
			// interpreted the standard
			// completely differently to
			// everyone else on the planet).
			// And "\\3.3o" isn't any better
			// (for the exact opposite reason).
			//
			output_fputc(fp, '\\');
			output_fputc(fp, '0' + ((c>>6)&3));
			output_fputc(fp, '0' + ((c>>3)&7));
			output_fputc(fp, '0' + ( c    &7));
		    }
		    else
			output_fprintf(fp, "\\%o", c);
		}
	    }
	    else
	    {
		switch (c)
		{
		case '(':
		case '[':
		case '{':
		    ++count;
		    if (count <= 0)
			goto escape;
		    break;

		case ')':
		case ']':
		case '}':
		    --count;
		    if (count < 0)
			goto escape;
		    break;

		case '\\':
		case '"':
		    output_fputc(fp, '\\');
		    break;
		}
		output_fputc(fp, c);
	    }
	}
    }
    output_fputc(fp, '"');
    if (name)
	output_fputs(fp, ";\n");
}


void
string_write_xml(output_ty *fp, const char *name, string_ty *this_thing)
{
    if (!this_thing && name)
	return;
    if (!name)
	name = "string";
    output_fprintf(fp, "<%s>", name);
    if (this_thing)
    {
	char            *s;

	for (s = this_thing->str_text; *s; ++s)
	{
	    unsigned char c;

	    c = *s;
	    switch (c)
	    {
	    case '<':
		output_fputs(fp, "&lt;");
		break;

	    case '>':
		output_fputs(fp, "&gt;");
		break;

	    case '&':
		output_fputs(fp, "&amp;");
		break;

	    case '"':
		output_fputs(fp, "&quot;");
		break;

	    case '\n':
		output_fputc(fp, c);
		break;

	    default:
		// always in the C locale
		if (isprint(c))
		    output_fputc(fp, c);
		else
		    output_fprintf(fp, "&#%d;", c);
		break;
	    }
	}
    }
    output_fprintf(fp, "</%s>\n", name);
}


static string_list_ty comment;


void
io_comment_append(sub_context_ty *scp, const char *fmt)
{
    string_ty       *s;
    string_list_ty  wl;
    size_t          j, k;

    // always in the C locale
    if (!scp)
    {
	scp = sub_context_new();
	s = subst_intl(scp, fmt);
	sub_context_delete(scp);
    }
    else
	s = subst_intl(scp, fmt);

    str2wl(&wl, s, "\n", 1);
    str_free(s);
    for (k = wl.nstrings; k > 0; --k)
	if (wl.string[k - 1]->str_length)
    	    break;
    for (j = 0; j < k; ++j)
	string_list_append(&comment, wl.string[j]);
    string_list_destructor(&wl);
}


void
io_comment_emit(output_ty *fp)
{
    size_t          j;

    if (!comment.nstrings)
	return;
    output_fputs(fp, "/*\n");
    for (j = 0; j < comment.nstrings; ++j)
	output_fprintf(fp, "** %s\n", comment.string[j]->str_text);
    output_fputs(fp, "*/\n");
    string_list_destructor(&comment);
}
