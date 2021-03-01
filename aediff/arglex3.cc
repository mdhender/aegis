//
//	aegis - project change supervisor
//	Copyright (C) 2004-2006, 2008 Peter Miller
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

#include <aediff/arglex3.h>


static arglex_table_ty argtab[] =
{
    { "-COMmand", arglex_token_command, },
    { "-CONtext", arglex_token_context, },
    { "-Ignore_All_Space", arglex_token_ignore_all_space, },
    { "-Ignore_Blank_Lines", arglex_token_ignore_blank_lines, },
    { "-Ignore_Case", arglex_token_ignore_case, },
    { "-Ignore_Space_Change", arglex_token_ignore_space_change, },
    { "-Show_C_Function", arglex_token_show_c_function, },
    { "-TEXt", arglex_token_text, },
    { "-UNified", arglex_token_unified, },
    ARGLEX_END_MARKER
};


void
arglex3_init(int argc, char **argv)
{
    arglex2_init3(argc, argv, argtab);
}
