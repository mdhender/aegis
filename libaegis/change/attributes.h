/*
 *	aegis - project change supervisor
 *	Copyright (C) 2000-2003 Peter Miller;
 *	All rights reserved.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * MANIFEST: interface definition for libaegis/change/attributes.c
 */

#ifndef LIBAEGIS_CHANGE_ATTRIBUTES_H
#define LIBAEGIS_CHANGE_ATTRIBUTES_H

#include <main.h>

struct cattr_ty;
struct change_ty;
struct cstate_ty;
struct pconf_ty;
struct project_ty;
struct string_ty;

void change_attributes_default(cattr_ty *, project_ty *, struct pconf_ty *);
void change_attributes_copy(struct cattr_ty *, cstate_ty *);
void change_attributes_verify(string_ty *, cattr_ty *);
void change_attributes_edit(cattr_ty **, int);

/**
  * The change_attributes_append function is used to append an
  * attribute to the change's list of attributes.  Note this this allow
  * duplicates.
  *
  * \param cs
  *     The chage state to work on.
  * \param name
  *     The name of the attribute, case insensitive.
  *     (names atrting with a lower-case letter do not appear in listings)
  * \param value
  *     The value to assign to the attribute.
  */
void change_attributes_append(cstate_ty *cs, const char *name,
    const char *value);

/**
  * The change_attributes_find function is used to find a specific named
  * attribute amongst the given change's attributes.  Only the first
  * such attribute is returned.
  *
  * \param cp
  *     The change to be searched.
  * \param name
  *     The name of the attribute to search for.
  * \returns
  *     The value of the given attribute, or NULL if not found.
  */
string_ty *change_attributes_find(change_ty *cp, string_ty *name);

/**
  * The change_attributes_fixup function is used to trim description and
  * brief description fields.  Only cosmetic changes are allowed.
  *
  * @param cap
  *     The change attributes in question.
  */
void change_attributes_fixup(cattr_ty *cap);

#endif /* LIBAEGIS_CHANGE_ATTRIBUTES_H */
