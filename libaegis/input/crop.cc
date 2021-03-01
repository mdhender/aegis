//
//	aegis - project change supervisor
//	Copyright (C) 2004 Peter Miller;
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
// MANIFEST: functions to manipulate crops
//

#include <input/crop.h>
#include <input/private.h>
#include <str.h>
#include <trace.h>


struct input_crop_ty
{
    input_ty        inherited;
    input_ty        *deeper;
    int             delete_on_close;
    size_t          pos;
    size_t          maximum;
};


static void
input_crop_destructor(input_ty *p)
{
    input_crop_ty   *icp;

    trace(("input_crop_destructor()\n{\n"));
    icp = (input_crop_ty *)p;
    if (icp->pos < icp->maximum)
	input_skip(icp->deeper, icp->maximum - icp->pos);
    if (icp->delete_on_close)
	input_delete(icp->deeper);
    icp->deeper = 0; // paranoia
    trace(("}\n"));
}


static long
input_crop_read(input_ty *ip, void *data, size_t len)
{
    input_crop_ty   *icp;
    long            nbytes;

    trace(("input_crop_read()\n{\n"));
    icp = (input_crop_ty *)ip;
    if (icp->pos + len > icp->maximum)
	len = icp->maximum - icp->pos;
    nbytes = input_read(icp->deeper, data, len);
    if (nbytes > 0)
	icp->pos += nbytes;
    trace(("return %ld;\n", (long)nbytes));
    trace(("}\n"));
    return nbytes;
}


static long
input_crop_ftell(input_ty *ip)
{
    input_crop_ty   *icp;

    icp = (input_crop_ty *)ip;
    trace(("input_crop_ftell => %ld\n", (long)icp->pos));
    return icp->pos;
}


static string_ty *
input_crop_name(input_ty *ip)
{
    input_crop_ty   *icp;

    trace(("input_crop_name\n"));
    icp = (input_crop_ty *)ip;
    return input_name(icp->deeper);
}


static long
input_crop_length(input_ty *ip)
{
    input_crop_ty   *icp;

    icp = (input_crop_ty *)ip;
    return icp->maximum;
}


static void
input_crop_keepalive(input_ty *fp)
{
    input_crop_ty   *ip;

    ip = (input_crop_ty *)fp;
    input_keepalive(ip->deeper);
}


static input_vtbl_ty vtbl =
{
    sizeof(input_crop_ty),
    input_crop_destructor,
    input_crop_read,
    input_crop_ftell,
    input_crop_name,
    input_crop_length,
    input_crop_keepalive,
};


input_ty *
input_crop_new(input_ty *deeper, int delete_on_close, long maximum)
{
    input_ty        *ip;
    input_crop_ty   *icp;

    trace(("input_crop_new(deeper = %08lX)\n{\n", (long)deeper));
    ip = input_new(&vtbl);
    icp = (input_crop_ty *)ip;
    icp->deeper = deeper;
    icp->delete_on_close = !!delete_on_close;
    icp->pos = 0;
    icp->maximum = maximum;
    trace(("return %08lX\n", (long)ip));
    trace(("}\n"));
    return ip;
}
