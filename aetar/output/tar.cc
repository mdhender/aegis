//
//	aegis - project change supervisor
//	Copyright (C) 2002-2004 Peter Miller;
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
// MANIFEST: functions to output to tar archivess
//

#include <error.h>
#include <output/tar.h>
#include <output/tar_child.h>
#include <output/private.h>
#include <str.h>


typedef struct output_tar_ty output_tar_ty;
struct output_tar_ty
{
    output_ty       inherited;
    output_ty       *deeper;
};


static void
output_tar_destructor(output_ty *fp)
{
    output_tar_ty   *this_thing;

    //
    // Finish writing the archive file.
    //
    this_thing = (output_tar_ty *)fp;
    output_delete(this_thing->deeper);
}


static string_ty *
output_tar_filename(output_ty *fp)
{
    output_tar_ty   *this_thing;

    this_thing = (output_tar_ty *)fp;
    return output_filename(this_thing->deeper);
}


static long
output_tar_ftell(output_ty *fp)
{
    return 0;
}


static void
output_tar_write(output_ty *fp, const void *data, size_t len)
{
    this_is_a_bug();
}


static void
output_tar_eoln(output_ty *fp)
{
    this_is_a_bug();
}


static output_vtbl_ty vtbl =
{
    sizeof(output_tar_ty),
    output_tar_destructor,
    output_tar_filename,
    output_tar_ftell,
    output_tar_write,
    output_generic_flush,
    output_generic_page_width,
    output_generic_page_length,
    output_tar_eoln,
    "tar archive",
};


output_ty *
output_tar(output_ty *deeper)
{
    output_ty       *result;
    output_tar_ty   *this_thing;

    result = output_new(&vtbl);
    this_thing = (output_tar_ty *)result;
    this_thing->deeper = deeper;
    return result;
}


output_ty *
output_tar_child(output_ty *fp, string_ty *name, long len, int executable)
{
    output_tar_ty   *this_thing;

    if (fp->vptr != &vtbl)
	    this_is_a_bug();
    this_thing = (output_tar_ty *)fp;
    assert(len >= 0);
    return output_tar_child_open(this_thing->deeper, name, len, executable);
}
