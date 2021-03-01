//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2002, 2004, 2005 Peter Miller;
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
// MANIFEST: interface definition for aedist/rfc822header.c
//

#ifndef LIBAEGIS_RFC822HEADER_H
#define LIBAEGIS_RFC822HEADER_H

#include <main.h>

struct input_ty; // existence
struct string_ty; // existence
struct symtab_ty; // existence

struct rfc822_header_ty
{
	struct symtab_ty *stp;
};

rfc822_header_ty *rfc822_header_read(struct input_ty *);
struct string_ty *rfc822_header_query(rfc822_header_ty *, const char *);
void rfc822_header_delete(rfc822_header_ty *);

#endif // LIBAEGIS_RFC822HEADER_H
