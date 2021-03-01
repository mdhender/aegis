//
//	aegis - project change supervisor
//	Copyright (C) 2004, 2005 Peter Miller;
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
// MANIFEST: interface definition for aecvsserver/file_info.c
//

#ifndef AECVSSERVER_FILE_INFO_H
#define AECVSSERVER_FILE_INFO_H

#include <common/main.h>

struct file_info_ty
{
    struct string_ty *version;
    int modified;
    int mode;
    int questionable;
};


file_info_ty *file_info_new(void);
void file_info_reset(file_info_ty *);
void file_info_delete(file_info_ty *);

#endif // AECVSSERVER_FILE_INFO_H
