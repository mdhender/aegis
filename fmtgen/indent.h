/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991-1993, 2002, 2003 Peter Miller.
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
 * MANIFEST: interface definition for fmtgen/indent.c
 */

#ifndef INDENT_H
#define INDENT_H

#include <main.h>

void indent_close(void);
void indent_less(void);
void indent_more(void);
void indent_open(const char *filename);
void indent_printf(const char *, ...)			ATTR_PRINTF(1, 2);
void indent_putchar(int);

#endif /* INDENT_H */
