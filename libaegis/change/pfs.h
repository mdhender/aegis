//
//	aegis - project change supervisor
//	Copyright (C) 2005-2008 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//

#ifndef LIBAEGIS_CHANGE_PFS_H
#define LIBAEGIS_CHANGE_PFS_H

#include <libaegis/change.h>

/**
  * The change_psf_write function is used to cache a copy of the project
  * file state at integrate pass time.
  *
  * @param cp
  *     The change being operated on.
  */
void change_pfs_write(change::pointer cp);

#endif // LIBAEGIS_CHANGE_PFS_H
