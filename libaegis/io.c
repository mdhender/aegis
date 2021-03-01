/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993, 1994, 1995, 1996, 1997, 1998, 1999 Peter Miller;
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
 * MANIFEST: functions to parse and write fundamental data types
 */

#include <ac/ctype.h>
#include <ac/string.h>

#include <io.h>
#include <output.h>
#include <sub.h>
#include <str_list.h>
#include <zero.h>


void
integer_write(fp, name, this)
	output_ty	*fp;
	const char	*name;
	long		this;
{
	if (this == INTEGER_NOT_SET && name)
		return;
	if (name)
		output_fprintf(fp, "%s = ", name);
	if (name && !strcmp(name, "umask"))
		output_fprintf(fp, "0%lo", this & 07777);
	else
	{
		if (this == MAGIC_ZERO)
			output_fputs(fp, "ZERO");
		else
			output_fprintf(fp, "%ld", this);
	}
	if (name)
		output_fputs(fp, ";\n");
}


void
real_write(fp, name, this)
	output_ty	*fp;
	const char	*name;
	double		this;
{
	if (this == REAL_NOT_SET && name)
		return;
	if (name)
		output_fprintf(fp, "%s = ", name);
	output_fprintf(fp, "%g", this);
	if (name)
		output_fputs(fp, ";\n");
}


void
time_write(fp, name, this)
	output_ty	*fp;
	const char	*name;
	time_t		this;
{
	if (this == TIME_NOT_SET && name)
		return;
	if (name)
		output_fprintf(fp, "%s = ", name);
	/*
	 * Time is always an arithmetic type, never a structure.
	 * This works on every system the author has seen,
	 * without loss of precision.
	 * (Loss of fractions of a second is acceptable.)
	 */
	output_fprintf(fp, "%ld", (long)this);
	if (name)
		output_fprintf(fp, "; /* %.24s */\n", ctime(&this));
}


void
string_write(fp, name, this)
	output_ty	*fp;
	const char	*name;
	string_ty	*this;
{
	char		*s;
	int		count;

	if (!this && name)
		return;
	if (name)
		output_fprintf(fp, "%s = ", name);
	output_fputc(fp, '"');
	if (this)
	{
		count = 0;
		for (s = this->str_text; *s; ++s)
		{
			switch (*s)
			{
			case '('/*)*/:
			case '['/*]*/:
			case '{'/*}*/:
				++count;
				break;
	
			case /*(*/')':
			case /*[*/']':
			case /*{*/'}':
				--count;
				break;
			}
		}
		if (count > 0)
			count = -count;
		else
			count = 0;
		for (s = this->str_text; *s; ++s)
		{
			int	c;
	
			c = (unsigned char)*s;
			/* always in the C locale */
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
					if (isdigit(s[1]))
						/* not entirely portable */
						output_fprintf(fp, "\\%03o", c);
					else
						output_fprintf(fp, "\\%o", c);
				}
			}
			else
			{
				switch (c)
				{
				case '('/*)*/:
				case '['/*]*/:
				case '{'/*}*/:
					++count;
					if (count <= 0)
						goto escape;
					break;
		
				case /*(*/')':
				case /*[*/']':
				case /*{*/'}':
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


static string_list_ty comment;


void
io_comment_append(scp, fmt)
	sub_context_ty	*scp;
	const char	*fmt;
{
	string_ty	*s;
	string_list_ty		wl;
	size_t		j, k;

	/* always in the C locale */
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
io_comment_emit(fp)
	output_ty	*fp;
{
	size_t		j;

	if (!comment.nstrings)
		return;
	output_fputs(fp, "/*\n");
	for (j = 0; j < comment.nstrings; ++j)
		output_fprintf(fp, "** %s\n", comment.string[j]->str_text);
	output_fputs(fp, "*/\n");
	string_list_destructor(&comment);
}
