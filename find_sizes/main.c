/*
 *	aegis - project change supervisor
 *	Copyright (C) 1994, 1995 Peter Miller;
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
 * MANIFEST: functions to manipulate mains
 */

#include <stdio.h>
#include <ac/stddef.h>
#include <ac/limits.h>

#include <arglex.h>
#include <error.h>


typedef struct table_ty table_ty;
struct table_ty
{
	char	*name;
	int	num_bits;
	int	is_signed;
};


static table_ty table[] =
{
	{ "unsigned long",	sizeof(unsigned long) * CHAR_BIT,	0, },
	{ "long\t",		sizeof(long) * CHAR_BIT,		1, },
	{ "unsigned int",	sizeof(unsigned int) * CHAR_BIT,	0, },
	{ "int\t",		sizeof(int) * CHAR_BIT,			1, },
	{ "unsigned short",	sizeof(unsigned short) * CHAR_BIT,	0, },
	{ "short\t",		sizeof(short) * CHAR_BIT,		1, },
};


static void find _((int, int));

static void
find(num_bits, is_signed)
	int		num_bits;
	int		is_signed;
{
	table_ty	*tp;

	for (tp = table; tp < ENDOF(table); ++tp)
	{
		if (tp->num_bits == num_bits && tp->is_signed == is_signed)
		{
			printf
			(
				"typedef\t%s\t%sint%d;\n",
				tp->name,
				(is_signed ? "" : "u"),
				num_bits
			);
			return;
		}
	}
	error_raw
	(
		"unable to find a%ssigned %d bit integer type",
		(is_signed ? "" : "n un"),
		num_bits
	);
}


int main _((int, char **));

int
main(argc, argv)
	int		argc;
	char		**argv;
{
	arglex_init(argc, argv, (arglex_table_ty *)0);
	printf("#ifndef COMMON_FIND_SIZES_H\n");
	printf("#define COMMON_FIND_SIZES_H\n");
	printf("\n");
	find(32, 1);
	find(32, 0);
	find(16, 1);
	find(16, 0);
	printf("\n");
	printf("#endif /* COMMON_FIND_SIZES_H */\n");
	exit(0);
}