/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991-1993, 1997, 2002-2004 Peter Miller;
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

void help(const char *progname, void (*usagefunc)(void));
void generic_argument(void(*usagefunc)(void));
void bad_argument(void(*usagefunc)(void)) NORETURN;
void mutually_exclusive_options(int name1, int name2, void(*usagefunc)(void));
void mutually_exclusive_options3(int name1, int name2, int name3,
	void (*usagefunc)(void));
void duplicate_option(void(*usagefunc)(void));
void duplicate_option_by_name(int name, void(*usagefunc)(void));
void option_needs_number(int name, void(*usagefunc)(void));
void option_needs_string(int name, void(*usagefunc)(void));
void option_needs_name(int name, void(*usagefunc)(void));
void option_needs_file(int name, void(*usagefunc)(void));
void option_needs_dir(int name, void(*usagefunc)(void));
void option_needs_files(int name, void(*usagefunc)(void));
void option_needs_uuid(int name, void(*usagefunc)(void));

struct string_ty; /* forward */
void fatal_too_many_files(void) NORETURN;
void fatal_user_too_privileged(struct string_ty *) NORETURN;
void fatal_group_too_privileged(struct string_ty *) NORETURN;
void fatal_bad_project_name(struct string_ty *) NORETURN;
void fatal_project_name_too_long(struct string_ty *, int) NORETURN;
void fatal_date_unknown(const char *) NORETURN;
void fatal_project_alias_exists(struct string_ty *) NORETURN;

#endif /* HELP_H */
