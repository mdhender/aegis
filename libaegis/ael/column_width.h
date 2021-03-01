//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2004, 2006, 2008 Peter Miller
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

#ifndef LIBAEGIS_AEL_COLUMN_WIDTH_H
#define LIBAEGIS_AEL_COLUMN_WIDTH_H


#define ELAPSED_TIME_THRESHOLD (10L * 60L) // ten minutes

//
// widths of the various columns
//
// (Many are 8n-1; this does nice things with tabs.)
//
#define USAGE_WIDTH	7	// strlen("manual_") = 7
#define ACTION_WIDTH	8	// strlen("insulate") = 8
#define EDIT_WIDTH	12	// strlen("1.23 (4.56)") = 11,
				   // strlen("1.23 -> 4.56") = 12
#define LOGIN_WIDTH	8	// login names will be <= 8
#define WHAT_WIDTH	15	// widest is 20, worst is 10
#define WHEN_WIDTH	15	// ctime fits in 2 lines
#define WHO_WIDTH	LOGIN_WIDTH
#define	CHANGE_WIDTH	7
#define	STATE_WIDTH	15	// widest is 20, worst is 11
#define ARCH_WIDTH	8
#define HOST_WIDTH	8
#define TIME_WIDTH	11	// strlen("99-Oct-2000") = 11
#define INDENT_WIDTH	8	// used for change_details
#define PROJECT_WIDTH	15
#define DIRECTORY_WIDTH 23
#define ATTR_WIDTH      15	// used for file and change attributes
#define UUID_WIDTH      36
#define FILENAME_WIDTH  42	// FILENAME_WIDTH + 1 + UUID_WIDTH < 80
#define VERSION_WIDTH   10       // strlen("1.234.D567") = 10

#endif // LIBAEGIS_AEL_COLUMN_WIDTH_H
