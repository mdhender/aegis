//
//	aegis - project change supervisor
//	Copyright (C) 2004-2006, 2008 Peter Miller
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

#ifndef AECVSSERVER_FAKE_VERSION_H
#define AECVSSERVER_FAKE_VERSION_H

#include <common/ac/time.h>
#include <common/str.h>

/**
  * The fake_version function is used to build a string which
  * superficially (to a program, not a human) looks like an RCS version
  * string.  Usually passed a file modification time.
  */
string_ty *fake_version(time_t);

/**
  * The fake_version_now function is used to build a string which
  * superficially (to a program, not a human) looks like an RCS version
  * string.  We want to to always be out-of-date, so we base it on the
  * current time.
  */
string_ty *fake_version_now(void);

#endif // AECVSSERVER_FAKE_VERSION_H
