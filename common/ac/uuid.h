/*
 *	aegis - project change supervisor
 *	Copyright (C) 2004 Walter Franzini;
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
 * MANIFEST: interface definition for common/ac/uuid.c
 */

#ifndef COMMON_AC_UUID_H
#define COMMON_AC_UUID_H

#include <config.h>

#ifdef HAVE_DCE_UUID_H
#include <dce/uuid.h>
#else
#ifdef HAVE_UUID_H
#include <uuid.h>
#else
#ifdef HAVE_UUID_UUID_H
#include <uuid/uuid.h>
#endif
#endif
#endif

#ifdef HAVE_UUID_HASH
#ifdef UUID_FLAVOR_SET
#error Too many uuid library found
#endif

#define UUID_FLAVOR_SET 1
#define UUID_IS_DCE 1
#endif

#ifdef HAVE_UUID_LOAD
#ifdef UUID_FLAVOR_SET
#error Too many uuid library found
#endif

#define UUID_FLAVOR_SET 1
#define UUID_IS_OSSP 1
#endif


#ifdef HAVE_UUID_GENERATE
#ifdef UUID_FLAVOR_SET
#error Too many uuid library found
#endif

#define UUID_FLAVOR_SET 1
#define UUID_IS_E2FS 1
#endif

#ifdef HAVE_LINUX_UUID
#ifndef UUID_FLAVOR_SET
#define UUID_IS_LINUX 1
#endif
#endif


#endif /* COMMON_AC_UUID_H */
