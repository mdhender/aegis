//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2003, 2005, 2006 Peter Miller;
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
// MANIFEST: inclulate against <math.h> vagueries
//

#ifndef COMMON_AC_MATH_H
#define COMMON_AC_MATH_H

#include <common/config.h>

#ifdef __linux__
#ifndef CONST
#define CONST const
#endif
#endif

#include <math.h>

#ifndef M_LN2
#define M_LN2 0.69314718055994530942
#endif

#endif // COMMON_AC_MATH_H
