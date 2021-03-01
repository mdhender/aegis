//
//	aegis - project change supervisor
//	Copyright (C) 2005 Peter Miller;
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
// MANIFEST: interface of the change_pfs class
//

#ifndef LIBAEGIS_CHANGE_PFS_H
#define LIBAEGIS_CHANGE_PFS_H

#include <change.h>

/**
  * The change_psf_write function is used to cache a copy of the project
  * file state at integrate pass time.
  *
  * @param cp
  *     The change being operated on.
  */
void change_pfs_write(change_ty *cp);

#endif // LIBAEGIS_CHANGE_PFS_H
