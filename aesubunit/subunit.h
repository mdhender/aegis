//
//	aegis - project change supervisor
//	Copyright (C) 2005, 2006, 2008 Peter Miller
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

#ifndef AESUBUNIT_SUBUNIT_H
#define AESUBUNIT_SUBUNIT_H

/**
  * The subunit_usage function is sued to report command line usage
  * errors, and then exit.
  */
void subunit_usage(void);

/**
  * The subunit function is used to parse the commandline and run the
  * unit tests.
  */
void subunit(void);

#endif // AESUBUNIT_SUBUNIT_H
