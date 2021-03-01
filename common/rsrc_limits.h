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

#ifndef COMMON_RSRC_LIMITS_H
#define COMMON_RSRC_LIMITS_H

/**
  * The resource_limits_init function is used to maximize all of the
  * memory limits.  This may be necessary for building file histories of
  * large projects.
  */
void resource_limits_init(void);

/**
  * The resource_usage_print method is used to print resource usage
  * statistics on the standard error.  This is usually done when an
  * ENOMEM error is seen.
  */
void resource_usage_print(void);

#endif // COMMON_RSRC_LIMITS_H
