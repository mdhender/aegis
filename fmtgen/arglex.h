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

#ifndef FMTGEN_ARGLEX_H
#define FMTGEN_ARGLEX_H

#include <common/arglex.h>

enum
{
    arglex_token_bit_fields,
    arglex_token_bit_fields_not,
    arglex_token_include_long,
    arglex_token_include_short,
    arglex_token_introspector_code,
    arglex_token_introspector_include,
    arglex_token_original_code,
    arglex_token_original_include,
    arglex_token_tab_width,
};

void arglex2_init(int argc, char **argv);

#endif // FMTGEN_ARGLEX_H
