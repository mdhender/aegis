//
// aegis - project change supervisor
// Copyright (C) 2002-2006, 2008, 2012 Peter Miller
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
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <common/ac/assert.h>

#include <common/sizeof.h>
#include <libaegis/sub.h>

#include <aecomplete/shell.h>
#include <aecomplete/shell/bash.h>
#include <aecomplete/shell/zsh.h>


typedef shell_ty *(*funcptr)(void);

static funcptr table[] =
{
    shell_bash,
    shell_zsh,
};


shell_ty *
shell_find(void)
{
    funcptr         *tp;
    shell_ty        *sp;
    funcptr         func;

    for (tp = table; tp < ENDOF(table); ++tp)
    {
        func = *tp;
        sp = func();
        if (shell_test(sp))
            return sp;
        shell_delete(sp);
    }
    fatal_intl(0, i18n("unknown shell"));
    // NOTREACHED
    return 0;
}


int
shell_test(shell_ty *this_thing)
{
    assert(this_thing);
    assert(this_thing->vptr);
    assert(this_thing->vptr->test);
    return this_thing->vptr->test(this_thing);
}


string_ty *
shell_command_get(shell_ty *this_thing)
{
    assert(this_thing);
    assert(this_thing->vptr);
    assert(this_thing->vptr->command_get);
    return this_thing->vptr->command_get(this_thing);
}


string_ty *
shell_prefix_get(shell_ty *this_thing)
{
    assert(this_thing);
    assert(this_thing->vptr);
    assert(this_thing->vptr->prefix_get);
    return this_thing->vptr->prefix_get(this_thing);
}


void
shell_emit(shell_ty *this_thing, string_ty *s)
{
    assert(this_thing);
    assert(this_thing->vptr);
    assert(this_thing->vptr->emit);
    this_thing->vptr->emit(this_thing, s);
}


// vim: set ts=8 sw=4 et :
