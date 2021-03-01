//
//	aegis - project change supervisor
//	Copyright (C) 2003, 2005, 2006 Peter Miller;
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
// MANIFEST: interface definition for aeget/forkandwatch.c
//

#ifndef AEGET_FORKANDWATCH_H
#define AEGET_FORKANDWATCH_H

#include <common/main.h>

/**
  * The fork_and_watch function is used to fork the current process.
  * This current process then watches the child exit status.  If it
  * succeeds, the standard output is sent to the user.  If it fails,
  * the standard error is sent to the user (after suitable html-ization).
  * This function only returns in the child, it doesn't return in the parent.
  */
void fork_and_watch(void);

#endif // AEGET_FORKANDWATCH_H
