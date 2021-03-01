//
//	aegis - project change supervisor
//	Copyright (C) 1997, 1999, 2002-2004, 2006-2008 Peter Miller
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

#ifndef COMMON_LIBDIR_H
#define COMMON_LIBDIR_H

/** \addtogroup Configured
  * \brief Retrieve configure time values
  * \ingroup Common
  * @{
  */
const char *configured_prefix(void);
const char *configured_bindir(void);
const char *configured_comdir(void);
const char *configured_datadir(void);
const char *configured_datarootdir(void);
const char *configured_libdir(void);
const char *configured_nlsdir(void);
const char *configured_mandir(void);
int configured_aegis_uid(void);
int configured_aegis_gid(void);
const char *configured_exeext(void);
const char *configured_sysconfdir(void);

/** @} */
#endif // COMMON_LIBDIR_H
