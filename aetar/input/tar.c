/*
 *	aegis - project change supervisor
 *	Copyright (C) 2002 Peter Miller;
 *	All rights reserved.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * MANIFEST: functions to manipulate tars
 */

#include <error.h> /* for assert */
#include <input/tar.h>
#include <input/tar_child.h>
#include <input/private.h>
#include <str.h>


typedef struct input_tar_ty input_tar_ty;
struct input_tar_ty
{
    input_ty	    inherited;
    input_ty	    *deeper;
};


static void input_tar_destructor _((input_ty *));

static void
input_tar_destructor(fp)
    input_ty	    *fp;
{
    input_tar_ty    *this;

    this = (input_tar_ty *)fp;
    input_delete(this->deeper);
}


static long input_tar_read _((input_ty *, void *, size_t));

static long
input_tar_read(fp, data, len)
    input_ty	    *fp;
    void	    *data;
    size_t	    len;
{
    assert(0);
    return -1;
}


static long input_tar_ftell _((input_ty *));

static long
input_tar_ftell(fp)
    input_ty	*fp;
{
    assert(0);
    return 0;
}


static string_ty *input_tar_name _((input_ty *));

static string_ty *
input_tar_name(fp)
    input_ty	    *fp;
{
    input_tar_ty    *this;

    this = (input_tar_ty *)fp;
    return input_name(this->deeper);
}


static long input_tar_length _((input_ty *));

static long
input_tar_length(fp)
    input_ty	    *fp;
{
    input_tar_ty    *this;

    this = (input_tar_ty *)fp;
    return input_length(this->deeper);
}


static input_vtbl_ty vtbl =
{
    sizeof(input_tar_ty),
    input_tar_destructor,
    input_tar_read,
    input_tar_ftell,
    input_tar_name,
    input_tar_length,
};


input_ty *
input_tar(deeper)
    input_ty	    *deeper;
{
    input_ty	    *result;
    input_tar_ty    *this;

    result = input_new(&vtbl);
    this = (input_tar_ty *)result;
    this->deeper = deeper;
    return result;
}


input_ty *
input_tar_child(fp, archive_name_p)
    input_ty	    *fp;
    string_ty	    **archive_name_p;
{
    input_tar_ty    *this;

    assert(archive_name_p);
    if (fp->vptr != &vtbl)
	return 0;
    this = (input_tar_ty *)fp;
    return input_tar_child_open(this->deeper, archive_name_p);
}
