/*
 *	aegis - project change supervisor
 *	Copyright (C) 1994, 1996, 2004 Peter Miller;
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
 * MANIFEST: insulate against <sys/ioctl.h> vs <termios.h> differences
 */

#ifndef COMMON_AC_TERMIOS_H
#define COMMON_AC_TERMIOS_H

#include <config.h>

#if HAVE_winsize_SYS_IOCTL_H
#include <sys/ioctl.h>
#else
#if HAVE_winsize_TERMIOS_H
#include <termios.h>
#endif /* HAVE_winsize_TERMIOS_H */
#endif /* !HAVE_winsize_SYS_IOCTL_H */

#endif /* COMMON_AC_TERMIOS_H */
