/*
 *	aegis - project change supervisor
 *	Copyright (C) 2004 Peter Miller;
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
 * MANIFEST: interface definition for aecvsserver/module/change.c
 */

#ifndef AECVSSERVER_MODULE_CHANGE_H
#define AECVSSERVER_MODULE_CHANGE_H

#include <module.h>

struct string_ty; /* forward */

/**
  * The module_change_new create a new instance of a module which operates
  * on Aegis change sets.
  */
module_ty *module_change_new(struct string_ty *projname, long change_number);

#endif /* AECVSSERVER_MODULE_CHANGE_H */
