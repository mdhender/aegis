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
 * MANIFEST: interface definition for aecvsserver/scramble.c
 */

#ifndef AECVSSERVER_SCRAMBLE_H
#define AECVSSERVER_SCRAMBLE_H

#include <str.h>

string_ty *scramble(string_ty *);
string_ty *descramble(string_ty *);

#endif /* AECVSSERVER_SCRAMBLE_H */