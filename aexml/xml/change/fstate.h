//
//	aegis - project change supervisor
//	Copyright (C) 2003, 2005, 2006, 2008 Peter Miller
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

#ifndef AEXML_XML_CHANGE_FSTATE_H
#define AEXML_XML_CHANGE_FSTATE_H

#include <common/main.h>

struct string_ty; // forward

void xml_change_fstate(struct string_ty *project_name, long change_number,
    output::pointer op);

#endif // AEXML_XML_CHANGE_FSTATE_H
