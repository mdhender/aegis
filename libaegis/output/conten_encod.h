//
//	aegis - project change supervisor
//	Copyright (C) 2001, 2002, 2004-2006, 2008 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//

#ifndef LIBAEGIS_OUTPUT_CONTEN_ENCOD_H
#define LIBAEGIS_OUTPUT_CONTEN_ENCOD_H

#include <libaegis/output.h>

enum content_encoding_t
{
	content_encoding_unset,
	content_encoding_none,
	content_encoding_base64,
	content_encoding_quoted_printable,
	content_encoding_uuencode
};

content_encoding_t content_encoding_grok(const char *);
void content_encoding_header(output::pointer , content_encoding_t);
output::pointer output_content_encoding(output::pointer , content_encoding_t);

#endif // LIBAEGIS_OUTPUT_CONTEN_ENCOD_H
