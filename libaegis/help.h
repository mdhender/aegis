/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993, 1997 Peter Miller;
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

void help _((char *progname, void (*usage)(void)));
void generic_argument _((void(*usage)(void)));
void bad_argument _((void(*usage)(void)));
void mutually_exclusive_options _((int name1, int name2, void(*usage)(void)));
void mutually_exclusive_options3 _((int name1, int name2, int name3,
	void (*usage)(void)));
void duplicate_option _((void(*usage)(void)));
void duplicate_option_by_name _((int name, void(*usage)(void)));
void option_needs_number _((int name, void(*usage)(void)));
void option_needs_string _((int name, void(*usage)(void)));
void option_needs_name _((int name, void(*usage)(void)));
void option_needs_file _((int name, void(*usage)(void)));
void option_needs_dir _((int name, void(*usage)(void)));
void option_needs_files _((int name, void(*usage)(void)));

struct string_ty; /* forward */
void fatal_too_many_files _((void));
void fatal_user_too_privileged _((struct string_ty *));
void fatal_group_too_privileged _((struct string_ty *));
void fatal_bad_project_name _((struct string_ty *));
void fatal_project_name_too_long _((struct string_ty *, int));
void fatal_date_unknown _((const char *));
void fatal_project_alias_exists _((struct string_ty *));

#endif /* HELP_H */
