/*
 *	aegis - project change supervisor
 *	Copyright (C) 2004 Walter Franzini;
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
 * MANIFEST: interface definition for common/uuidentifier.c
 */

#ifndef COMMON_UUIDENTIFIER_H
#define COMMON_UUIDENTIFIER_H

#include <main.h>

/** \addtogroup UUID
  * \brief UUID wrapper
  * \ingroup Common
  * @{
  */

#define ORIGINAL_UUID "original-UUID"
#define EDIT_ORIGIN_UUID "edit-origin-UUID"

/**
  * \brief Returns a string containing a new UUID.
  */
string_ty *universal_unique_identifier(void);

/**
  * \brief Check if the UUID passed as input is valid.
  */
int universal_unique_identifier_valid(string_ty *);

/** @} */
#endif /* COMMON_UUIDENTIFIER_H */
