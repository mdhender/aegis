//
// aegis - project change supervisor
// Copyright (C) 1999, 2002, 2003, 2005-2008, 2012 Peter Miller
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
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#ifndef LIBAEGIS_CHANGE_ARCHITECTURE_FIND_VARIANT_H
#define LIBAEGIS_CHANGE_ARCHITECTURE_FIND_VARIANT_H

#include <libaegis/change.h>
#include <libaegis/cstate.fmtgen.h>

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
cstate_architecture_times_ty *change_find_architecture_variant(
    change::pointer cp);

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
cstate_architecture_times_ty *change_find_architecture_variant(
    change::pointer cp, string_ty *variant);

#endif // LIBAEGIS_CHANGE_ARCHITECTURE_FIND_VARIANT_H
// vim: set ts=8 sw=4 et :
