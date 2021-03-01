//
//	aegis - project change supervisor
//	Copyright (C) 2007, 2008 Peter Miller
//
//      This program is free software; you can redistribute it and/or
//      modify it under the terms of the GNU General Public License as
//      published by the Free Software Foundation; either version 3 of
//      the License, or (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program. If not, see
//      <http://www.gnu.org/licenses/>.
//

#include <common/error.h> // for assert
#include <libaegis/aer/expr.h>
#include <libaegis/aer/lex.h>


void
rpt_expr::pos_from_lex()
{
    assert(!pos);
    pos = rpt_lex_pos_get();
    assert(pos);
}


void
rpt_expr::pos_from(const rpt_expr::pointer &other)
{
    assert(!pos);
    assert(other->get_pos());
    pos = other->get_pos();
    assert(pos);
}
