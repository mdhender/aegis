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
 * MANIFEST: interface definition for common/option.c
 */

#ifndef OPTION_H
#define OPTION_H

#include <main.h>

/*
 * This is set to 14 so that your project will be portable
 * to less privileged unix boxes.  Please don't mess with it.
 */
#define PATH_ELEMENT_MAX 14

#define MAX_PAGE_WIDTH 2000

void option_progname_set _((char *));
char *option_progname_get _((void));

void option_verbose_set _((void));
int option_verbose_get _((void));
void option_unformatted_set _((void));
int option_unformatted_get _((void));
void option_terse_set _((void));
int option_terse_get _((void));

void option_page_width_set _((int columns));
int option_page_width_get _((void));
void option_page_length_set _((int rows));
int option_page_length_get _((void));

#endif /* OPTION_H */
