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

void option_set_progname _((char *));
char *option_get_progname _((void));

void option_set_verbose _((void));
int option_get_verbose _((void));
void option_set_terse _((void));
int option_get_terse _((void));

void option_set_page_width _((int));
int option_get_page_width _((void));
void option_set_page_length _((int));
int option_get_page_length _((void));

#endif /* OPTION_H */
