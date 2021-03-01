/*
 *	aegis - project change supervisor
 *	Copyright (C) 2002 Peter Miller;
 *	All rights reserved.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * MANIFEST: functions to manipulate shells
 */

#include <error.h> /* for assert */
#include <shell.h>
#include <shell/bash.h>
#include <sub.h>


typedef shell_ty *(*funcptr)_((void));

static funcptr table[] =
{
    shell_bash,
};


shell_ty *
shell_find()
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
}


int
shell_test(this)
    shell_ty        *this;
{
    assert(this);
    assert(this->vptr);
    assert(this->vptr->test);
    return this->vptr->test(this);
}


string_ty *
shell_command_get(this)
    shell_ty        *this;
{
    assert(this);
    assert(this->vptr);
    assert(this->vptr->command_get);
    return this->vptr->command_get(this);
}


string_ty *
shell_prefix_get(this)
    shell_ty        *this;
{
    assert(this);
    assert(this->vptr);
    assert(this->vptr->prefix_get);
    return this->vptr->prefix_get(this);
}


void
shell_emit(this, s)
    shell_ty        *this;
    string_ty       *s;
{
    assert(this);
    assert(this->vptr);
    assert(this->vptr->emit);
    this->vptr->emit(this, s);
}
