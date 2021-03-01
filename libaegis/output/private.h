/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999, 2002 Peter Miller;
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
 * MANIFEST: interface definition for libaegis/output/private.c
 */

#ifndef LIBAEGIS_OUTPUT_PRIVATE_H
#define LIBAEGIS_OUTPUT_PRIVATE_H

#include <output.h>

output_ty *output_new(output_vtbl_ty *);
long output_generic_ftell(output_ty *);
void output_generic_fputs(output_ty *, const char *);
void output_generic_write(output_ty *, const void *, size_t);
void output_generic_flush(output_ty *);
int output_generic_page_width(output_ty *);
int output_generic_page_length(output_ty *);

#endif /* LIBAEGIS_OUTPUT_PRIVATE_H */
