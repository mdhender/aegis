//
//	aegis - project change supervisor
//	Copyright (C) 2004-2007 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
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
// MANIFEST: implementation of the quit_action_history_transaction_abort class
//

#include <libaegis/change.h>
#include <libaegis/quit/action/histtransabt.h>


quit_action_history_transaction_abort::~quit_action_history_transaction_abort()
{
}


quit_action_history_transaction_abort::quit_action_history_transaction_abort(
	change::pointer arg) :
    cp(arg)
{
}


void
quit_action_history_transaction_abort::operator()(int exit_status)
{
    if (exit_status != 0)
	change_run_history_transaction_abort_command(cp);
}
