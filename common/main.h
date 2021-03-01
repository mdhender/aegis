/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993 Peter Miller.
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
 * MANIFEST: common definitions, including DEBUG define
 */

#ifndef MAIN_H
#define MAIN_H

/*
 *  function prototype insulation
 */
#ifndef _
# ifdef __STDC__
#  define _(x) x
# else
#  define _(x) ()
#  define const
# endif
#endif /* _ */

/*
 * array manipulation
 */
#define SIZEOF(a) (sizeof(a) / sizeof(a[0]))
#define ENDOF(a) ((a) + SIZEOF(a))

/*
 * Take the define out of comments to
 * enable the debugging the functionality.
 *
#define DEBUG
 */

#endif /* MAIN_H */
