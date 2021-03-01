//
//      aegis - project change supervisor
//      Copyright (C) 2004-2006, 2008, 2011, 2012 Peter Miller
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 3 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program. If not, see
//      <http://www.gnu.org/licenses/>.
//

#ifndef LIBAEGIS_PROJECT_INVENTO_WALK_H
#define LIBAEGIS_PROJECT_INVENTO_WALK_H

#include <libaegis/change/functor.h>

struct project; // forward

/**
  * The project_inventory_walk function is used to walk the project's
  * change set inventory, one change at a time.  The time limit is
  * calculated from the project itself, if it is completed, otherwise it
  * is "now".
  *
  * \param pp
  *     Pointer to the project to walk the change set inventory of.
  * \param cf
  *     Change functor to call for each change.
  */
void project_inventory_walk(project *pp, change_functor &cf);

/**
  * The project_inventory_walk function is used to walk the project's
  * change set inventory, one change at a time.
  *
  * \param pp
  *     Pointer to the project to walk the change set inventory of.
  * \param cf
  *     Change functor to call for each change.
  * \param limit
  *     This is the time limit for changes to be included.
  */
void project_inventory_walk(project *pp, change_functor &cf, time_t limit);

#endif // LIBAEGIS_PROJECT_INVENTO_WALK_H
// vim: set ts=8 sw=4 et :
