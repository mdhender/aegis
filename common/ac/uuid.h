//
//	aegis - project change supervisor
//	Copyright (C) 2006, 2008 Peter Miller
//	Copyright (C) 2004 Walter Franzini;
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

#ifndef COMMON_AC_UUID_H
#define COMMON_AC_UUID_H

#include <common/config.h>

#if HAVE_DCE_UUID_H && HAVE_UUID_CREATE && HAVE_UUID_TO_STRING
#include <dce/uuid.h>
#define UUID_FLAVOR_SET 1
#define UUID_IS_DCE 1

#elif HAVE_UUID_H
#include <uuid.h>

#elif HAVE_UUID_UUID_H
#include <uuid/uuid.h>

#elif HAVE_SYS_UUID_H
extern "C" {
#include <sys/uuid.h>
}
#endif

#if HAVE_UUID_CREATE && HAVE_UUID_MAKE && HAVE_UUID_EXPORT
    #ifdef UUID_FLAVOR_SET
    #error Too many uuid library found
    #endif
#define UUID_FLAVOR_SET 1
#define UUID_IS_OSSP 1
#endif


#if HAVE_UUID_GENERATE && HAVE_UUID_UNPARSE
    #ifdef UUID_FLAVOR_SET
    #error Too many uuid library found
    #endif
#define UUID_FLAVOR_SET 1
#define UUID_IS_E2FS 1
#endif

#if HAVE_LINUX_UUID
    #ifndef UUID_FLAVOR_SET
    #define UUID_IS_LINUX 1
    #endif
#endif


#endif // COMMON_AC_UUID_H
