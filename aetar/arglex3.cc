//
//	aegis - project change supervisor
//	Copyright (C) 2002-2006, 2008 Peter Miller
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

#include <aetar/arglex3.h>

static arglex_table_ty argtab[] =
{
    { "-Entire_Source", arglex_token_entire_source, },
    { "-Not_Entire_Source", arglex_token_entire_source_not, },
    { "-Partial_Source", arglex_token_entire_source_not, },
    { "-EXCLude", arglex_token_exclude, },
    { "-Exclude_Auto_Tools", arglex_token_exclude_auto_tools, },
    { "-Exclude_Concurrent_Version_System", arglex_token_exclude_cvs, },
    { "-Include_BUild", arglex_token_include_build, },
    { "-Not_Include_BUild", arglex_token_include_build_not, },
    { "-Add_Path_Prefix", arglex_token_path_prefix_add, },
    { "-Remove_Path_Prefix", arglex_token_path_prefix_remove, },
    { "-Receive", arglex_token_receive, },
    { "-Send", arglex_token_send, },
    { "-Trojan", arglex_token_trojan, },
    { "-Not_Trojan", arglex_token_trojan_not, },
    ARGLEX_END_MARKER
};


void
arglex3_init(int argc, char **argv)
{
    arglex2_init3(argc, argv, argtab);
}
