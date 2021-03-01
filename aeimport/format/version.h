/*
 *	aegis - project change supervisor
 *	Copyright (C) 2001, 2002 Peter Miller;
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
 * MANIFEST: interface definition for aeimport/format/version.c
 */

#ifndef AEIMPORT_FORMAT_VERSION_H
#define AEIMPORT_FORMAT_VERSION_H

#include <ac/time.h>

#include <str_list.h>

typedef struct format_version_ty format_version_ty;
struct format_version_ty
{
    string_ty       *filename_physical;
    string_ty       *filename_logical;
    string_ty       *edit;
    time_t	    when;
    string_ty	    *who;
    string_ty	    *description;
    string_list_ty  tag;
    format_version_ty *before;
    format_version_ty *after;
    struct format_version_list_ty *after_branch;
    int		    dead;
};

format_version_ty *format_version_new(void);
void format_version_delete(format_version_ty *);
void format_version_validate(format_version_ty *);

#endif /* AEIMPORT_FORMAT_VERSION_H */
