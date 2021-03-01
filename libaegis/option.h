/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991-1993, 1995, 1997, 1999, 2002, 2003 Peter Miller;
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
 * MANIFEST: interface definition for common/option.c
 */

#ifndef OPTION_H
#define OPTION_H

#include <main.h>

void option_progname_set(char *);
char *option_progname_get(void);

void option_verbose_set(void(*)(void));
int option_verbose_get(void);
void option_unformatted_set(void(*)(void));
int option_unformatted_get(void);
void option_terse_set(void(*)(void));
int option_terse_get(void);

void option_page_width_set(int columns, void (*usage)(void));
void option_page_length_set(int rows, void (*usage)(void));

void option_tab_width_set(int, void(*)(void));
int option_tab_width_get(void);

void option_page_headers_set(int, void(*)(void));
int option_page_headers_get(void);

#endif /* OPTION_H */
