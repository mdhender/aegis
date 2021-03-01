//
//	aegis - project change supervisor
//	Copyright (C) 2001-2006, 2008 Peter Miller
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

#include <aepatch/arglex3.h>


static arglex_table_ty argtab[] =
{
    { "-Ascii_Armor", arglex_token_ascii_armor, },
    { "-Not_Ascii_Armor", arglex_token_ascii_armor_not, },
    { "-Content_Transfer_Encoding", arglex_token_content_transfer_encoding},
    { "-Receive", arglex_token_receive, },
    { "-Send", arglex_token_send, },
    { "-Trojan", arglex_token_trojan, },
    { "-Not_Trojan", arglex_token_trojan_not, },
    { "-Add_Path_Prefix", arglex_token_path_prefix_add, },
    { "-Remove_Path_Prefix", arglex_token_path_prefix_remove, },
    ARGLEX_END_MARKER
};


void
arglex3_init(int argc, char **argv)
{
    arglex2_init3(argc, argv, argtab);
}
