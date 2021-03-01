/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991-1993, 1999, 2002, 2003 Peter Miller.
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
 * MANIFEST: interface definition for libaegis/version.c
 */

#ifndef LIBAEGIS_VERSION_H
#define LIBAEGIS_VERSION_H

#include <main.h>

/**
  * The version_copyright function is used to print a generic version
  * number and copyright notice.
  */
void version_copyright(void);

/**
  * The version function is used to handle --version command line options.
  */
void version(void);

#endif /* LIBAEGIS_VERSION_H */
