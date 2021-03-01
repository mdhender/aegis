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
 * MANIFEST: interface defn for libaegis/change/develop_direct/read_write.c
 */

#ifndef LIBAEGIS_CHANGE_DEVELOP_DIRECT_READ_WRITE_H
#define LIBAEGIS_CHANGE_DEVELOP_DIRECT_READ_WRITE_H

#include <main.h>

struct change_ty; /* forward */

void change_development_directory_chmod_read_write(struct change_ty *);

#endif /* LIBAEGIS_CHANGE_DEVELOP_DIRECT_READ_WRITE_H */
