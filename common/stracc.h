/*
 *	aegis - project change supervisor
 *	Copyright (C) 1998 Peter Miller;
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
 * MANIFEST: interface definition for common/stracc.c
 */

#ifndef COMMON_STRACC_H
#define COMMON_STRACC_H

#include <str.h>

typedef struct stracc_t stracc_t;
struct stracc_t
{
	size_t	length;
	size_t	maximum;
	char	*buffer;
};

void stracc_constructor _((stracc_t *));
void stracc_destructor _((stracc_t *));
void stracc_open _((stracc_t *));
string_ty *stracc_close _((const stracc_t *));
void stracc_char _((stracc_t *, int));
void stracc_chars _((stracc_t *, const char *, size_t));

#endif /* COMMON_STRACC_H */