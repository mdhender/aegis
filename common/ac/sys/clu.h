/*
 *	aegis - project change supervisor
 *	Copyright (C) 2001 Peter Miller;
 *	All rights reserved.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * MANIFEST: interface definition for common/ac/sys/clu.c
 */

#ifndef COMMON_AC_SYS_CLU_H
#define COMMON_AC_SYS_CLU_H

#include <config.h>

#ifdef HAVE_SYS_CLU_H
#include <sys/clu.h>
#else
#include <main.h>
#define MAXHOSTNAMELEN 100
#define CLU_INFO_MY_ID 0
#define CLU_INFO_NODENAME_BY_ID 1
typedef int memberid_t;
int clu_info(int, ...);
#endif

#endif /* COMMON_AC_SYS_CLU_H */
