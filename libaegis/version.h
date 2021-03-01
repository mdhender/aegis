//
//	aegis - project change supervisor
//	Copyright (C) 1991-2008 Peter Miller
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

#ifndef LIBAEGIS_VERSION_H
#define LIBAEGIS_VERSION_H

#include <common/main.h>

/**
  * The version_copyright function is used to print a generic version
  * number and copyright notice.
  */
void version_copyright(void);

/**
  * The version function is used to handle --version command line options.
  */
void version(void);

#endif // LIBAEGIS_VERSION_H
