/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993, 1994, 1999 Peter Miller;
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
 * MANIFEST: interface definition for aegis/file.c
 */

#ifndef FILE_H
#define FILE_H

#include <str.h>

void copy_whole_file _((string_ty *from, string_ty *to, int cmt));
string_ty *read_whole_file _((char *));
int files_are_different _((string_ty *, string_ty *));
void file_from_string _((string_ty *, string_ty *, int));
void cat_string_to_stdout _((string_ty *));

#endif /* FILE_H */
