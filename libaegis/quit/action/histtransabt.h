//
//	aegis - project change supervisor
//	Copyright (C) 2004-2008 Peter Miller
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

#ifndef LIBAEGIS_QUIT_ACTION_HISTTRANSABT_H
#define LIBAEGIS_QUIT_ACTION_HISTTRANSABT_H

#include <common/quit/action.h>

/**
  * The quit_action_history_transaction_abort class is used to represent
  * a change_run_history_transaction_abort_command to be run on command
  * failure.
  */
class quit_action_history_transaction_abort:
    public quit_action
{
public:
    /**
      * The destructor.
      */
    virtual ~quit_action_history_transaction_abort();

    /**
      * The constructor.
      */
    quit_action_history_transaction_abort(change::pointer );

    // See base class for documentation.
    void operator()(int exit_status);

private:
    /**
      * The cp instance variable is used to remember the change to operate on.
      */
    change::pointer cp;

    /**
      * The default constructor.  Do not use.
      */
    quit_action_history_transaction_abort();

    /**
      * The copy constructor.  Do not use.
      */
    quit_action_history_transaction_abort(
	const quit_action_history_transaction_abort &);

    /**
      * The assignment operator.  Do not use.
      */
    quit_action_history_transaction_abort &operator=(
	const quit_action_history_transaction_abort &);
};

#endif // LIBAEGIS_QUIT_ACTION_HISTTRANSABT_H
