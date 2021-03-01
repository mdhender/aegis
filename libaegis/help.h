/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991-1993, 1997, 2002, 2003 Peter Miller;
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
 * MANIFEST: interface definition for aegis/help.c
 */

#ifndef HELP_H
#define HELP_H

#include <main.h>

void help(const char *progname, void (*usage)(void));
void generic_argument(void(*usage)(void));
void bad_argument(void(*usage)(void));
void mutually_exclusive_options(int name1, int name2, void(*usage)(void));
void mutually_exclusive_options3(int name1, int name2, int name3,
	void (*usage)(void));
void duplicate_option(void(*usage)(void));
void duplicate_option_by_name(int name, void(*usage)(void));
void option_needs_number(int name, void(*usage)(void));
void option_needs_string(int name, void(*usage)(void));
void option_needs_name(int name, void(*usage)(void));
void option_needs_file(int name, void(*usage)(void));
void option_needs_dir(int name, void(*usage)(void));
void option_needs_files(int name, void(*usage)(void));

struct string_ty; /* forward */
void fatal_too_many_files(void);
void fatal_user_too_privileged(struct string_ty *);
void fatal_group_too_privileged(struct string_ty *);
void fatal_bad_project_name(struct string_ty *);
void fatal_project_name_too_long(struct string_ty *, int);
void fatal_date_unknown(const char *);
void fatal_project_alias_exists(struct string_ty *);

#endif /* HELP_H */
