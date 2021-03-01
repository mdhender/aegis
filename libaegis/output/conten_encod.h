/*
 *	aegis - project change supervisor
 *	Copyright (C) 2001, 2002 Peter Miller;
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
 * MANIFEST: interface definition for libaegis/output/conten_encod.c
 */

#ifndef LIBAEGIS_OUTPUT_CONTEN_ENCOD_H
#define LIBAEGIS_OUTPUT_CONTEN_ENCOD_H

#include <output.h>

enum content_encoding_t
{
	content_encoding_unset,
	content_encoding_none,
	content_encoding_base64,
	content_encoding_quoted_printable,
	content_encoding_uuencode
};
typedef enum content_encoding_t content_encoding_t;

content_encoding_t content_encoding_grok(const char *);
void content_encoding_header(output_ty *, content_encoding_t);
output_ty *output_content_encoding(output_ty *, content_encoding_t);

#endif /* LIBAEGIS_OUTPUT_CONTEN_ENCOD_H */
