//
//	aegis - project change supervisor
//	Copyright (C) 2005, 2006 Peter Miller
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
// MANIFEST: interface of the project_change_walk class
//

#ifndef LIBAEGIS_PROJECT_CHANGE_WALK_H
#define LIBAEGIS_PROJECT_CHANGE_WALK_H

#include <libaegis/project/change/walk.h>

class project_ty; // forward
class change_functor; // forward

/**
  * The project_change_walk method is used to walk each change of a
  * project branch tree.
  *
  * @param pp
  *     The project in question.
  * @param functor
  *     The function-like object to call for each change.
  */
void project_change_walk(project_ty *pp, change_functor &functor);

#endif // LIBAEGIS_PROJECT_CHANGE_WALK_H
