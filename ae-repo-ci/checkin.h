//
//	aegis - project change supervisor
//	Copyright (C) 2006 Peter Miller;
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
// MANIFEST: interface of the checkin class
//

#ifndef AE_REPO_CHECKIN_CHECKIN_H
#define AE_REPO_CHECKIN_CHECKIN_H

/**
  * The checkin_usage function is used to print a usage message and exit.
  */
void checkin_usage(void);

/**
  * The checkin function is used to parse the command line for the
  * project and change and repository to use, and then redundantly
  * commit the change set into the foreign repository.
  */
void checkin(void);

#endif // AE_REPO_CHECKIN_CHECKIN_H
