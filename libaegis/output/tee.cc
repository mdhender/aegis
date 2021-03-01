//
//	aegis - project change supervisor
//	Copyright (C) 2001, 2003, 2004 Peter Miller;
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
// MANIFEST: functions to manipulate tees
//

#include <output/tee.h>
#include <output/private.h>
#include <str.h>


typedef struct output_tee_ty output_tee_ty;
struct output_tee_ty
{
	output_ty	inherited;
	output_ty	*d1;
	int		d1_close;
	output_ty	*d2;
	int		d2_close;
};


static void
output_tee_destructor(output_ty *fp)
{
	output_tee_ty *this_thing;

	this_thing = (output_tee_ty *)fp;
	if (this_thing->d1_close)
		output_delete(this_thing->d1);
	if (this_thing->d2_close)
		output_delete(this_thing->d2);
}


static void
output_tee_write(output_ty *fp, const void *data, size_t nbytes)
{
	output_tee_ty *this_thing;

	this_thing = (output_tee_ty *)fp;
	output_write(this_thing->d1, data, nbytes);
	output_write(this_thing->d2, data, nbytes);
}


static void
output_tee_flush(output_ty *fp)
{
	output_tee_ty *this_thing;

	this_thing = (output_tee_ty *)fp;
	output_flush(this_thing->d1);
	output_flush(this_thing->d2);
}


static string_ty *
output_tee_filename(output_ty *fp)
{
	output_tee_ty *this_thing;

	this_thing = (output_tee_ty *)fp;
	return output_filename(this_thing->d1);
}


static long
output_tee_ftell(output_ty *fp)
{
	output_tee_ty	*this_thing;
	long		result;

	this_thing = (output_tee_ty *)fp;
	result = output_ftell(this_thing->d1);
	if (result < 0)
		result = output_ftell(this_thing->d2);
	return result;
}


static int
output_tee_page_width(output_ty *fp)
{
	output_tee_ty *this_thing;

	this_thing = (output_tee_ty *)fp;
	return output_page_width(this_thing->d1);
}


static int
output_tee_page_length(output_ty *fp)
{
	output_tee_ty *this_thing;

	this_thing = (output_tee_ty *)fp;
	return output_page_length(this_thing->d1);
}


static void
output_tee_eoln(output_ty *fp)
{
	output_tee_ty *this_thing;

	this_thing = (output_tee_ty *)fp;
	output_end_of_line(this_thing->d1);
	output_end_of_line(this_thing->d2);
}


static output_vtbl_ty vtbl =
{
	sizeof(output_tee_ty),
	output_tee_destructor,
	output_tee_filename,
	output_tee_ftell,
	output_tee_write,
	output_tee_flush,
	output_tee_page_width,
	output_tee_page_length,
	output_tee_eoln,
	"tee",
};


output_ty *
output_tee(output_ty *d1, int d1_close, output_ty *d2, int d2_close)
{
	output_ty	*result;
	output_tee_ty *this_thing;

	result = output_new(&vtbl);
	this_thing = (output_tee_ty *)result;
	this_thing->d1 = d1;
	this_thing->d1_close = !!d1_close;
	this_thing->d2 = d2;
	this_thing->d2_close = !!d2_close;
	return result;
}
