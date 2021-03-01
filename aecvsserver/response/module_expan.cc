//
//	aegis - project change supervisor
//	Copyright (C) 2004-2006, 2008 Peter Miller
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
//
// Module-expansion <pathname>
//
// Return a file or directory which is included in a particular module.
// The <pathname> is relative to cvsroot, unlike most pathnames in
// responses.  The <pathname> should be used to look and see whether some
// or all of the module exists on the client side; it is not necessarily
// suitable for passing as an argument to a co request (for example,
// if the modules file contains the -d option, it will be the directory
// specified with -d, not the name of the module).
//

#include <libaegis/output.h>
#include <aecvsserver/response/module_expan.h>


response_module_expansion::~response_module_expansion()
{
    str_free(answer);
    answer = 0;
}


response_module_expansion::response_module_expansion(string_ty *arg) :
    answer(str_copy(arg))
{
}


void
response_module_expansion::write(output::pointer op)
{
    op->fprintf("Module-expansion %s\n", answer->str_text);
}


response_code_ty
response_module_expansion::code_get()
    const
{
    return response_code_Module_expansion;
}
