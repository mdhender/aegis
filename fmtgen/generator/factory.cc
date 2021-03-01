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

#include <common/error.h>

#include <fmtgen/generator/introsp_code.h>
#include <fmtgen/generator/introsp_incl.h>
#include <fmtgen/generator/orig_code.h>
#include <fmtgen/generator/orig_include.h>
#include <fmtgen/arglex.h>


generator::pointer
generator::factory(int tok, const nstring &filename)
{
    switch (tok)
    {
    case arglex_token_introspector_code:
        return generator_introspector_code::create(filename);

    case arglex_token_introspector_include:
        return generator_introspector_include::create(filename);

    case arglex_token_original_code:
        return generator_original_code::create(filename);

    case arglex_token_original_include:
        return generator_original_include::create(filename);

    default:
        fatal_raw("generator \"%s\" unknown", arglex_token_name(tok));
        return pointer();
    }
}
