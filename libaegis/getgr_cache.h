//
//	aegis - project change supervisor
//	Copyright (C) 2001, 2002, 2005-2008 Peter Miller
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

#ifndef LIBAEGIS_GETGR_CACHE_H
#define LIBAEGIS_GETGR_CACHE_H

#include <common/ac/grp.h>
#include <common/nstring.h>

struct group; // forward

group *getgrnam_cached(string_ty *name);
group *getgrnam_cached(const nstring &name);
group *getgrgid_cached(int gid);

#endif // LIBAEGIS_GETGR_CACHE_H
