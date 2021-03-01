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
 * MANIFEST: functions to manipulate executes
 */

#include <os.h>
#include <sub.h>
#include <trace.h>


#define MAX_CMD_RPT 36


void
os_execute(cmd, flags, dir)
    string_ty       *cmd;
    int             flags;
    string_ty       *dir;
{
    int             result;

    trace(("os_execute()\n{\n"));
    result = os_execute_retcode(cmd, flags, dir);
    if (result)
    {
	sub_context_ty  *scp;
	string_ty       *s;

	if (cmd->str_length > MAX_CMD_RPT)
	    s = str_format("%.*S...", MAX_CMD_RPT - 3, cmd);
	else
	    s = str_copy(cmd);
	scp = sub_context_new();
	sub_var_set_string(scp, "Command", s);
	sub_var_set_long(scp, "Number", result);
	fatal_intl(scp, i18n("command \"$command\" exit status $number"));
	/* NOTREACHED */
	sub_context_delete(scp);
	str_free(s);
    }
    trace(("}\n"));
}
