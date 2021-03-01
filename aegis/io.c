/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993, 1994 Peter Miller.
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
 *	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * MANIFEST: functions to parse and write fundamental data types
 */

#include <ctype.h>
#include <ac/string.h>

#include <indent.h>
#include <str.h>
#include <io.h>


void
integer_write(name, this)
	char		*name;
	long		this;
{
	if (!this && name)
		return;
	if (name)
		indent_printf("%s = ", name);
	if (name && !strcmp(name, "umask"))
		indent_printf("0%lo", this & 07777);
	else
		indent_printf("%ld", this);
	if (name)
		indent_printf(";\n");
}


void
time_write(name, this)
	char		*name;
	time_t		this;
{
	if (!this && name)
		return;
	if (name)
		indent_printf("%s = ", name);
	/*
	 * Time is always an arithmetic type, never a structure.
	 * This works on every system the author has seen,
	 * without loss of precision.
	 * (Loss of fractions of a second is acceptable.)
	 */
	indent_printf("%ld", (long)this);
	if (name)
		indent_printf("; /* %.24s */\n", ctime(&this));
}


void
string_write(name, this)
	char		*name;
	string_ty	*this;
{
	char		*s;
	int		count;

	if (!this && name)
		return;
	if (name)
		indent_printf("%s = ", name);
	indent_putchar('"');
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
			if (c < ' ' || c > '~')
			{
				char	*cp;
	
				cp = strchr("\bb\ff\nn\rr\tt", c);
				if (cp)
				{
					indent_putchar('\\');
					indent_putchar(cp[1]);
					if (c == '\n')
						indent_printf("\\\n");
				}
				else
				{
					escape:
					if (isdigit(s[1]))
						indent_printf("\\%03o", c);
					else
						indent_printf("\\%o", c);
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
					indent_putchar('\\');
					break;
				}
				indent_putchar(c);
			}
		}
	}
	indent_putchar('"');
	if (name)
		indent_printf(";\n");
}
