//
// aegis - project change supervisor
// Copyright (C) 2007 Walter Franzini
// Copyright (C) 2008 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or (at
// your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program. If not, see <http://www.gnu.org/licenses/>.
//

#ifndef LIBAEGIS_VIEW_PATH_NEXT_CHANGE_H
#define LIBAEGIS_VIEW_PATH_NEXT_CHANGE_H

#include <libaegis/view_path.h>


/**
  * The view_path_next_change function is used to find the next change
  * to consult while looking in the view path.
  *
  * @param cp
  *     The change currently visited.
  *
  * @param time_limit
  *     the time limit to use when looking at the view path.  Changes
  *     integrated after the time_limit are not considered.  Use the
  *     value TIME_NOT_SET to ignore the time limit.
  *
  * @note
  *     This function may returns a newly allocated change or a copy
  *     of a previously allocated one, so you should remember to
  *     change_free its result.
  */
change::pointer
view_path_next_change(change::pointer cp, time_t time_limit);


#endif // LIBAEGIS_VIEW_PATH_NEXT_CHANGE_H
