//
//	aegis - project change supervisor
//	Copyright (C) 2002-2006, 2008 Peter Miller
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

#ifndef AECOMPLETE_SHELL_PRIVATE_H
#define AECOMPLETE_SHELL_PRIVATE_H

#include <aecomplete/shell.h>

struct shell_vtbl_ty
{
    /**
      * The destructor method is called when the shell object is deleted.
      */
    void (*destructor)(shell_ty *);

    /**
      * The test method is used to probe a shell type to see if this is
      * the calling shell.  It returns 0 if it does not match, and
      * non-zero if it does match.
      *
      * As a side-effect of a match, the rest of the command line (via
      * the arglex functions) is to be read in.  The command line being
      * completed in to be synthesized and installed in its place.
      */
    int (*test)(shell_ty *);

    /**
      * The command name method is used to extract the name of the
      * command being completed.
      */
    string_ty *(*command_get)(shell_ty *);

    /**
      * The incomplete_word method is used to extract the incomplete word
      * (the prefix) to be completed.
      */
    string_ty *(*prefix_get)(shell_ty *);

    /**
      * The emit method is used to emit a completion candidate, in the
      * format required by the particular shell.
      */
    void (*emit)(shell_ty *, string_ty *);

    /**
      * The size of the class instance, as returned by sizeof().
      */
    int size;

    /**
      * This should always be last.  That way, if the structure
      * initializers are missing a new function, this will cause an compiler
      * type warning.
      */
    const char *name;
};

shell_ty *shell_new(shell_vtbl_ty *);

#endif // AECOMPLETE_SHELL_PRIVATE_H
