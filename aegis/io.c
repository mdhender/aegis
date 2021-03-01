/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993 Peter Miller.
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

#include <string.h>
#include <time.h>

#include <indent.h>
#include <str.h>
#include <io.h>


type_ty	integer_type =
{
	type_class_integer,
	"integer",
	0, /* alloc */
	0, /* free */
	0, /* enumerator */
};


type_ty string_type =
{
	type_class_string,
	"string",
	0, /* alloc */
	0, /* free */
	0, /* enum_parse */
	0, /* list_parse */
	0, /* struct_parse */
};


static int suffix _((char *s1, char *s2));

static int
suffix(s1, s2)
	char	*s1;
	char	*s2;
{
	size_t	len1;
	size_t	len2;

	len1 = strlen(s1);
	len2 = strlen(s2);
	return (len2 < len1 && !strcmp(s1 + len1 - len2, s2));
}


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
	{
		indent_printf(";");
		if (suffix(name, "_time") || !strcmp(name, "when"))
			indent_printf(" /* %.24s */", ctime(&this));
		indent_printf("\n");
	}
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
	
			c = *s;
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
					if (s[1] >= '0' && s[1] <= '9')
						indent_printf("\\%03o", (unsigned char)c);
					else
						indent_printf("\\%o", (unsigned char)c);
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
