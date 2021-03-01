//
//	aegis - project change supervisor
//	Copyright (C) 2003, 2004 Peter Miller;
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
// MANIFEST: functions to manipulate view_paths
//

#include <view_path.h>


const char *
view_path_ename(view_path_ty n)
{
    switch (n)
    {
    case view_path_first:
	return "view_path_first";

    case view_path_none:
	return "view_path_none";

    case view_path_simple:
	return "view_path_simple";

    case view_path_extreme:
	return "view_path_extreme";
    }
    return "unknown";
}
