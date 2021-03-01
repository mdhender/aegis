//
//	aegis - project change supervisor
//	Copyright (C) 2006 Peter Miller
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
// MANIFEST: implementation of the nstring_gmatch3 class
//

#include <common/nstring.h>
#include <common/nstring/list.h>


bool
nstring::gmatch(const nstring_list &patterns)
    const
{
    for (size_t j = 0; j < patterns.size(); ++j)
        if (gmatch(patterns[j]))
            return true;
    return false;
}
