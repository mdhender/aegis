//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2003, 2004 Peter Miller;
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
// MANIFEST: functions to manipulate wraps
//

#include <output/wrap.h>
#include <output/to_wide.h>
#include <wide_output/expand.h>
#include <wide_output/wrap.h>
#include <wide_output/to_narrow.h>


output_ty *
output_wrap_open(output_ty *deeper, int delete_on_close, int width)
{
	wide_output_ty	*fp1;
	wide_output_ty	*fp2;
	wide_output_ty	*fp3;
	output_ty	*fp4;

	fp1 = wide_output_to_narrow_open(deeper, delete_on_close);
	fp2 = wide_output_wrap_open(fp1, 1, width);
	fp3 = wide_output_expand_open(fp2, 1);
	fp4 = output_to_wide_open(fp3, 1);
	return fp4;
}
