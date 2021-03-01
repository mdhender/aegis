//
//	aegis - project change supervisor
//	Copyright (C) 1994, 1997, 2002, 2005-2008 Peter Miller
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

#ifndef AEGIS_AER_PARSE_H
#define AEGIS_AER_PARSE_H

#include <common/str_list.h>

void report_parse_filename_set(string_ty *);
void report_parse_output_set(string_ty *);
void report_parse_argument_set(string_list_ty *);
void report_run(void);
void report_parse__init_arg(void);

#endif // AEGIS_AER_PARSE_H
