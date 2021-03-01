//
// aegis - project change supervisor
// Copyright (C) 2008 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or (at
// your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <fmtgen/arglex.h>


static arglex_table_ty argtab[] =
{
    { "-\\I*", arglex_token_include_short, },
    { "-Include", arglex_token_include_long, },
    { "-Introspector_Code", arglex_token_introspector_code },
    { "-Introspector_Include", arglex_token_introspector_include },
    { "-Original_Code", arglex_token_original_code },
    { "-Original_Include", arglex_token_original_include },
    { "-Tab_Width", arglex_token_tab_width },
    { "-Bit_Fields", arglex_token_bit_fields },
    { "-Not_Bit_Fields", arglex_token_bit_fields_not },
    ARGLEX_END_MARKER
};


void
arglex2_init(int argc, char **argv)
{
    arglex_init(argc, argv, argtab);
}
