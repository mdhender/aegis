//
//      aegis - project change supervisor
//      Copyright (C) 1991-1993, 1997, 2002-2008, 2012 Peter Miller
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 3 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program. If not, see
//      <http://www.gnu.org/licenses/>.
//

#ifndef LIBAEGIS_HELP_H
#define LIBAEGIS_HELP_H

#include <common/gcc-attribute.h>

struct string_ty; // forward
class nstring; // forward

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

/**
  * The option_need_url function is used to report a command line error
  * to the user, complaining that the given option must be followed by a
  * URL.
  *
  * @param name
  *     The option to complain about
  * @param usagefunc
  *     The usage function to exit via
  */
void option_needs_url(int name, void (*usagefunc)(void));

void option_needs_uuid(int name, void(*usagefunc)(void));

void fatal_too_many_files(void) NORETURN;

void fatal_user_too_privileged(string_ty *name) NORETURN DEPRECATED;
void fatal_user_too_privileged(const nstring &name) NORETURN;

void fatal_group_too_privileged(string_ty *name) NORETURN DEPRECATED;
void fatal_group_too_privileged(const nstring &name) NORETURN;

void fatal_bad_project_name(string_ty *) NORETURN;
void fatal_project_name_too_long(string_ty *, int) NORETURN;
void fatal_date_unknown(const char *) NORETURN;
void fatal_project_alias_exists(string_ty *) NORETURN;

#endif // LIBAEGIS_HELP_H
// vim: set ts=8 sw=4 et :
