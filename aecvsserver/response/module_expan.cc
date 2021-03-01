//
//	aegis - project change supervisor
//	Copyright (C) 2004, 2005 Peter Miller;
//	All rights reserved.
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: functions to manipulate module_expans
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

#include <output.h>
#include <response/module_expan.h>
#include <response/private.h>


struct response_module_expansion_ty
{
    response_ty     inherited;
    string_ty       *name;
};


static void
destructor(response_ty *rp)
{
    response_module_expansion_ty *rmep;

    rmep = (response_module_expansion_ty *)rp;
    str_free(rmep->name);
    rmep->name = 0;
}


static void
write(response_ty *rp, output_ty *op)
{
    response_module_expansion_ty *rmep;

    rmep = (response_module_expansion_ty *)rp;
    op->fprintf("Module-expansion %s\n", rmep->name->str_text);
}


static const response_method_ty vtbl =
{
    sizeof(response_module_expansion_ty),
    destructor,
    write,
    response_code_Module_expansion,
    0, // not flushable
};


response_ty *
response_module_expansion_new(string_ty *name)
{
    response_ty     *rp;
    response_module_expansion_ty *rmep;

    rp = response_new(&vtbl);
    rmep = (response_module_expansion_ty *)rp;
    rmep->name = str_copy(name);
    return rp;
}
