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
// MANIFEST: functions to manipulate responses
//

#include <error.h> // for assert
#include <mem.h>
#include <output.h>
#include <response.h>
#include <response/private.h>


void
response_delete(response_ty *rp)
{
    assert(rp);
    assert(rp->vptr);
    if (rp->vptr->destructor)
	rp->vptr->destructor(rp);
    mem_free(rp);
}


void
response_write(response_ty *rp, output_ty *op)
{
    assert(rp);
    assert(rp->vptr);
    assert(rp->vptr->write);
    rp->vptr->write(rp, op);
}


response_code_ty
response_code_get(response_ty *rp)
{
    assert(rp);
    assert(rp->vptr);
    return rp->vptr->code;
}


int
response_flushable(response_ty *rp)
{
    assert(rp);
    assert(rp->vptr);
    return rp->vptr->flushable;
}


void
output_mode_string(output_ty *op, int mode)
{
    output_fputc(op, 'u');
    output_fputc(op, '=');
    if (mode & 0400)
	output_fputc(op, 'r');
    if (mode & 0200)
	output_fputc(op, 'w');
    if (mode & 0100)
	output_fputc(op, 'x');
    output_fputc(op, ',');
    output_fputc(op, 'g');
    output_fputc(op, '=');
    if (mode & 0040)
	output_fputc(op, 'r');
    if (mode & 0020)
	output_fputc(op, 'w');
    if (mode & 0010)
	output_fputc(op, 'x');
    output_fputc(op, ',');
    output_fputc(op, 'o');
    output_fputc(op, '=');
    if (mode & 0004)
	output_fputc(op, 'r');
    if (mode & 0002)
	output_fputc(op, 'w');
    if (mode & 0001)
	output_fputc(op, 'x');
    output_fputc(op, '\n');
}
