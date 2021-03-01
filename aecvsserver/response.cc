//
//	aegis - project change supervisor
//	Copyright (C) 2004-2006 Peter Miller;
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

#include <common/error.h> // for assert
#include <common/mem.h>
#include <libaegis/output.h>
#include <aecvsserver/response.h>


response::~response()
{
}


response::response()
{
}


bool
response::flushable()
    const
{
    return false;
}


void
output_mode_string(output_ty *op, int mode)
{
    op->fputc('u');
    op->fputc('=');
    if (mode & 0400)
	op->fputc('r');
    if (mode & 0200)
	op->fputc('w');
    if (mode & 0100)
	op->fputc('x');
    op->fputc(',');
    op->fputc('g');
    op->fputc('=');
    if (mode & 0040)
	op->fputc('r');
    if (mode & 0020)
	op->fputc('w');
    if (mode & 0010)
	op->fputc('x');
    op->fputc(',');
    op->fputc('o');
    op->fputc('=');
    if (mode & 0004)
	op->fputc('r');
    if (mode & 0002)
	op->fputc('w');
    if (mode & 0001)
	op->fputc('x');
    op->fputc('\n');
}
