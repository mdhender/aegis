//
//	aegis - project change supervisor
//	Copyright (C) 2004, 2006, 2008 Peter Miller
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

#ifndef COMMON_YYYYMMDD_WK_H
#define COMMON_YYYYMMDD_WK_H

/**
  * The yyyymmdd_to_wday function is used to turn a date in years months
  * and days into a weekday number (as described by the localtime(3) man
  * page) Sunday=0.
  *
  * \param yyyy
  *     The year as all 4 digits, e.g. 1997 or 2004, etc
  *     (this is <b>not</b> the same as tm_year.)
  * \param mm
  *     The month number, in the range 1..12
  *     (this is <b>not</b> the same as tm_mon.)
  * \param dd
  *     The day number, in the range 1..31
  *     (this is <b>not</b> the same as tm_mon.)
  * \returns
  *     The day of the week, in the range 0..6; Sunday = 0.
  */
int yyyymmdd_to_wday(int yyyy, int mm, int dd);

/**
  * The days_in_month function may be used to calculate the number of
  * days in a month, given the year and the month.
  *
  * \param year
  *     The year as all 4 digits, e.g. 1997 or 2004, etc
  *     (this is <b>not</b> the same as tm_year.)
  * \param month
  *     The month number, in the range 1..12
  *     (this is <b>not</b> the same as tm_mon.)
  * \returns
  *     int; number of days in the month.
  */
int days_in_month(int year, int month);

#endif // COMMON_YYYYMMDD_WK_H
