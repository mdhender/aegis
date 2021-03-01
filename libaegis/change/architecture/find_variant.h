//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2002, 2003, 2005, 2006 Peter Miller;
//	All rights reserved.
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
// MANIFEST: interface defn for libaegis/change/architecture/find_variant.c
//

#ifndef LIBAEGIS_CHANGE_ARCHITECTURE_FIND_VARIANT_H
#define LIBAEGIS_CHANGE_ARCHITECTURE_FIND_VARIANT_H

#include <libaegis/change.h>
#include <libaegis/cstate.h>

/**
  * The change_find_architecture_variant function is used to obtain a
  * pointer to the architecture variant data for the currently executing
  * architecture.
  *
  * @param cp
  *     The change in question.
  * @returns
  *     a pointer to the desired data structure.  It is never NULL.
  */
cstate_architecture_times_ty *change_find_architecture_variant(change_ty *cp);

/**
  * The change_find_architecture_variant function is used to obtain a
  * pointer to the architecture variant data for the currently executing
  * architecture.
  *
  * @param cp
  *     The change in question.
  * @param variant
  *     The name of the architecture of interest.
  * @returns
  *     a pointer to the desired data structure.  It is never NULL.
  */
cstate_architecture_times_ty *change_find_architecture_variant(change_ty *cp,
    string_ty *variant);

#endif // LIBAEGIS_CHANGE_ARCHITECTURE_FIND_VARIANT_H
