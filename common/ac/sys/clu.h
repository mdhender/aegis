//
//	aegis - project change supervisor
//	Copyright (C) 2001, 2005, 2006, 2008 Peter Miller
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

#ifndef COMMON_AC_SYS_CLU_H
#define COMMON_AC_SYS_CLU_H

#include <common/config.h>

#ifdef HAVE_SYS_CLU_H
#include <sys/clu.h>
#else
#include <common/main.h>
#define MAXHOSTNAMELEN 100
#define CLU_INFO_MY_ID 0
#define CLU_INFO_NODENAME_BY_ID 1
typedef int memberid_t;
int clu_info(int, ...);
#endif

#endif // COMMON_AC_SYS_CLU_H
