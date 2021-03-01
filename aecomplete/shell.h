//
//	aegis - project change supervisor
//	Copyright (C) 2002, 2004-2006, 2008 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//

#ifndef AECOMPLETE_SHELL_H
#define AECOMPLETE_SHELL_H

#include <common/str.h>

struct shell_ty
{
    struct shell_vtbl_ty *vptr;
};

struct string_list_ty; // forward


/**
  * The shell_find function is used to determine which shell invoked
  * the aecomplete command, so that the appropriate responses may be made.
  *
  * If the shell cannot be determined, this function will exit with
  * a fatal error.  This function will only return if the shell can
  * be determined.
  */
shell_ty *shell_find(void);

/**
  * The shell_command_get function is used to get the name of the command
  * being completed.
  */
string_ty *shell_command_get(shell_ty *);

/**
  * The shell_prefix_get function is used to fetch the incomplete word
  * (prefix) to be completed.
  */
string_ty *shell_prefix_get(shell_ty *);

/**
  * The shell_test function is used to determine whether the environment
  * matches that required for the use of thus shell.  Returns zero
  * on failure.
  *
  * Success is messier.  It parses the command line, and then re-creates
  * the command line being completed, before returning non-zero.
  */
int shell_test(shell_ty *);

/**
  * The shell_delete function is used when you have finished wioth
  * a shell,  It is important to call it, so that any shell-specific
  * wrap-up can be performed.
  */
void shell_delete(shell_ty *);

/**
  * The shell_emit function is used to emit a completion candidate in
  * the format required by the given shell.
  */
void shell_emit(shell_ty *, string_ty *);

/**
  * The shell_emit_file_list function is used to emit a list of files
  * names.  It will be carefully culled to only print names up to the
  * slash following the longest common prefix.
  */
void shell_emit_file_list(shell_ty *, struct string_list_ty *);

#endif // AECOMPLETE_SHELL_H
