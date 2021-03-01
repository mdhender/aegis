//
//	aegis - project change supervisor
//	Copyright (C) 2003, 2005, 2006, 2008 Peter Miller
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
