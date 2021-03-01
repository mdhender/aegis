//
//	aegis - project change supervisor
//	Copyright (C) 2003, 2005-2008 Peter Miller
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

#ifndef COMMON_NOW_H
#define COMMON_NOW_H

/** \addtogroup Date
  * @{
  */
#include <common/ac/time.h>

/**
  * The now function is used to get the current time.  It resturns a
  * consistent result no matter how long the program has been running.
  */
time_t now(void);

/**
  * The now_set function is used to set the programs idea of the
  * current time.  All future calls to now() will return this value.
  */
void now_set(time_t);

/**
  * The time_clear function is used to reset the program's idea of the
  * current time.
  */
void now_clear(void);

/**
  * The now_unclearable function is used by aeimport to tell now_clear
  * that it isn't to do anything.  Otherwise aeimport can't backdate
  * the project statrt (etc).
  */
void now_unclearable(void);

/**
  * The time_max function is used to determine the value of the largest
  * positive value a time_t variable may assume.
  */
time_t time_max(void);

/**
  * The dtime function may be used to obtain the current time, to
  * sub-second precision.
  */
double dtime();

/** @} */
#endif // COMMON_NOW_H
