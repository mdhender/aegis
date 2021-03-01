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
// MANIFEST: functions to manipulate oks
//
//
// The command completed successfully.
//

#include <output.h>
#include <response/ok.h>
#include <response/private.h>


static void
write(response_ty *rp, output_ty *op)
{
    op->fputs("ok\n");
}


static const response_method_ty vtbl =
{
    sizeof(response_ty),
    0, // destructor
    write,
    response_code_ok,
    1, // flushable
};


response_ty *
response_ok_new(void)
{
    return response_new(&vtbl);
}
