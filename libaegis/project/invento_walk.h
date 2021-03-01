//
//	aegis - project change supervisor
//	Copyright (C) 2004-2006 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: interface of the project_invento_walk class
//

#ifndef LIBAEGIS_PROJECT_INVENTO_WALK_H
#define LIBAEGIS_PROJECT_INVENTO_WALK_H

#include <libaegis/change/functor.h>

struct project_ty; // forward

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
void project_inventory_walk(project_ty *pp, change_functor &cf);

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
void project_inventory_walk(project_ty *pp, change_functor &cf, time_t limit);

#endif // LIBAEGIS_PROJECT_INVENTO_WALK_H
