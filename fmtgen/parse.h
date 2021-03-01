//
//	aegis - project change supervisor
//	Copyright (C) 1992, 1993, 2002, 2003, 2005, 2006, 2008 Peter Miller.
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

#ifndef PARSE_H
#define PARSE_H

#include <common/nstring.h>

void generate_code__init(const nstring &);
void parse(const char *in, const char *c_out, const char *h_out);

#endif // PARSE_H
