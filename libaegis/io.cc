//
//	aegis - project change supervisor
//	Copyright (C) 1991-1999, 2001-2006, 2008 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//

#include <common/ac/ctype.h>
#include <common/ac/string.h>

#include <libaegis/io.h>
#include <common/nstring.h>
#include <libaegis/output.h>
#include <libaegis/sub.h>
#include <common/str_list.h>
#include <libaegis/zero.h>


void
boolean_write(output::pointer fp, const char *name, bool this_thing, int show)
{
    if (!this_thing)
    {
	if (!show || type_enum_option_query())
	    return;
    }
    if (name)
	fp->fprintf("%s = ", name);
    fp->fputs(boolean_ename(this_thing));
    if (name)
	fp->fputs(";\n");
}


void
boolean_write_xml(output::pointer fp, const char *name, bool this_thing,
    int show)
{
    if (!this_thing)
    {
	if (!show || type_enum_option_query())
	    return;
    }
    if (!name)
	name = "boolean";
    fp->fprintf("<%s>%s</%s>\n", name, boolean_ename(this_thing), name);
}


void
integer_write(output::pointer fp, const char *name, long this_thing, int show)
{
    if (this_thing == INTEGER_NOT_SET && !show)
	return;
    if (name)
	fp->fprintf("%s = ", name);
    if (name && !strcmp(name, "umask"))
	fp->fprintf("0%lo", this_thing & 07777);
    else
    {
	if (this_thing == MAGIC_ZERO)
    	    fp->fputs("ZERO");
	else
    	    fp->fprintf("%ld", this_thing);
    }
    if (name)
	fp->fputs(";\n");
}


void
integer_write_xml(output::pointer fp, const char *name, long this_thing,
    int show)
{
    if (this_thing == INTEGER_NOT_SET && !show)
	return;
    if (!name)
	name = "integer";
    fp->fprintf("<%s>", name);
    if (!strcmp(name, "umask"))
	fp->fprintf("0%lo", this_thing & 07777);
    else
    {
	if (this_thing == MAGIC_ZERO)
    	    fp->fputs("0");
	else
    	    fp->fprintf("%ld", this_thing);
    }
    fp->fprintf("</%s>\n", name);
}


void
real_write(output::pointer fp, const char *name, double this_thing, int show)
{
    if (this_thing == REAL_NOT_SET && !show)
	return;
    if (name)
	fp->fprintf("%s = ", name);
    fp->fprintf("%g", this_thing);
    if (name)
	fp->fputs(";\n");
}


void
real_write_xml(output::pointer fp, const char *name, double this_thing,
    int show)
{
    if (this_thing == REAL_NOT_SET && !show)
	return;
    if (!name)
	name = "real";
    fp->fprintf("<%s>%g</%s>\n", name, this_thing, name);
}


void
time_write(output::pointer fp, const char *name, time_t this_thing, int show)
{
    if (this_thing == TIME_NOT_SET && !show)
	return;
    if (name)
	fp->fprintf("%s = ", name);

    //
    // Time is always an arithmetic type, never a structure.
    // This works on every system the author has seen,
    // without loss of precision.
    // (Loss of fractions of a second is acceptable.)
    //
    fp->fprintf("%ld", (long)this_thing);
    if (name)
	fp->fprintf("; /* %.24s */\n", ctime(&this_thing));
}


void
time_write_xml(output::pointer fp, const char *name, time_t this_thing,
    int show)
{
    if (this_thing == TIME_NOT_SET && !show)
	return;
    if (!name)
	name = "time";
    fp->fprintf
    (
	"<%s>%ld</%s> <!-- %.24s -->\n",
	name,
	(long)this_thing,
	name,
	ctime(&this_thing)
    );
}


static void
string_write(output::pointer fp, const char *name, const char *value)
{
    if (name)
	fp->fprintf("%s = ", name);
    fp->fputc('"');

    int nesting_depth = 0;
    for (const char *s1 = value; *s1; ++s1)
    {
        switch (*s1)
        {
        case '(':
        case '[':
        case '{':
            ++nesting_depth;
            break;

        case ')':
        case ']':
        case '}':
            --nesting_depth;
            break;
        }
    }
    if (nesting_depth > 0)
        nesting_depth = -nesting_depth;
    else
        nesting_depth = 0;
    for (const char *s = value; *s; ++s)
    {
        unsigned char c = *s;
        switch (c)
        {
        case '(':
        case '[':
        case '{':
            ++nesting_depth;
            if (nesting_depth <= 0)
            {
                escape:
                if (isdigit((unsigned char)s[1]))
                {
                    //
                    // I'd prefer to use "\\%03o"
                    // but that isn't entirely
                    // portable (the glibc people
                    // interpreted the standard
                    // completely differently than
                    // everyone else on the planet).
                    // And "\\3.3o" isn't any better
                    // (for the exact opposite reason).
                    //
                    fp->fputc('\\');
                    fp->fputc('0' + ((c>>6)&3));
                    fp->fputc('0' + ((c>>3)&7));
                    fp->fputc('0' + ( c    &7));
                }
                else
                    fp->fprintf("\\%o", c);
            }
            else
                fp->fputc(c);
            break;

        case ')':
        case ']':
        case '}':
            --nesting_depth;
            if (nesting_depth < 0)
                goto escape;
            fp->fputc(c);
            break;

        case '\\':
        case '"':
            fp->fputc('\\');
            fp->fputc(c);
            break;

        default:
            // always in the C locale
            if (!isprint(c))
            {
                const char *cp = strchr("\bb\ff\nn\rr\tt", c);
                if (!cp)
                    goto escape;
                fp->fputc('\\');
                fp->fputc(cp[1]);
                if (c == '\n')
                    fp->fputs("\\\n");
            }
            else
                fp->fputc(c);
            break;
        }
    }
    fp->fputc('"');
    if (name)
	fp->fputs(";\n");
}


void
string_write(output::pointer fp, const char *name, string_ty *this_thing)
{
    if (!this_thing && name)
	return;
    string_write(fp, name, (this_thing ? this_thing->str_text : ""));
}



void
string_write(output::pointer fp, const char *name, const nstring &value)
{
    string_write(fp, name, value.c_str());
}


void
string_write_xml(output::pointer fp, const char *name, string_ty *this_thing)
{
    if (!this_thing && name)
	return;
    if (!name)
	name = "string";
    fp->fprintf("<%s>", name);
    if (this_thing)
	fp->fputs_xml(this_thing);
    fp->fprintf("</%s>\n", name);
}


void
string_write_xml(output::pointer fp, const char *name, const nstring &value)
{
    if (!name)
	name = "string";
    fp->fprintf("<%s>", name);
    fp->fputs_xml(value);
    fp->fprintf("</%s>\n", name);
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

    wl.split(s, "\n", true);
    str_free(s);
    for (k = wl.nstrings; k > 0; --k)
	if (wl.string[k - 1]->str_length)
    	    break;
    for (j = 0; j < k; ++j)
	comment.push_back(wl.string[j]);
}


void
io_comment_emit(output::pointer fp)
{
    size_t          j;

    if (!comment.nstrings)
	return;
    fp->fputs("/*\n");
    for (j = 0; j < comment.nstrings; ++j)
	fp->fprintf("** %s\n", comment.string[j]->str_text);
    fp->fputs("*/\n");
    comment.clear();
}
