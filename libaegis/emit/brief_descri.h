//
//	aegis - project change supervisor
//	Copyright (C) 2004, 2006-2008 Peter Miller
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

#ifndef LIBAEGIS_EMIT_BRIEF_DESCRI_H
#define LIBAEGIS_EMIT_BRIEF_DESCRI_H

/**
  * The emit_change_brief_description function is used to print (on the
  * standard output) the brief description of the given change in good
  * html.  The first twenty (or so) characters are linked to the change
  * menu.
  */
void emit_change_brief_description(change::pointer cp);

#endif // LIBAEGIS_EMIT_BRIEF_DESCRI_H
