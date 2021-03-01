//
//	aegis - project change supervisor
//	Copyright (C) 2005, 2006, 2008 Peter Miller
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

#ifndef COMMON_AC_SYS_RESOURCE_H
#define COMMON_AC_SYS_RESOURCE_H

//
// MacOS X needs time.h to be included first
//
#include <common/ac/time.h>

#include <sys/resource.h>

//
// MacOS X does not define RLIMIT_AS
//
#ifndef RLIMIT_AS
#  ifdef RLIMIT_MEMLOCK
#    define RLIMIT_AS RLIMIT_MEMLOCK
#  else
#    error Nothing defined for RLIMIT_AS
#  endif
#endif

#endif // COMMON_AC_SYS_RESOURCE_H
